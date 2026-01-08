var turtle_game = (function () {
    // begin nonsense thoughtlessly copied from internet
    window.requestAnimFrame = (function(callback) {
        return window.requestAnimationFrame || window.webkitRequestAnimationFrame || window.mozRequestAnimationFrame || window.oRequestAnimationFrame || window.msRequestAnimationFrame ||
            function(callback) {
                window.setTimeout(callback, 1000 / 60);
            };
    })();
    // end nonsense


    // === Geometry

    // distToSegment by Grumdrig, http://stackoverflow.com/a/1501725/94977
    function sqr(x) {
        return x * x;
    }

    function dist2(v, w) {
        return sqr(v.x - w.x) + sqr(v.y - w.y);
    }

    function distToSegmentSquared(p, v, w) {
        var l2 = dist2(v, w);
        if (l2 == 0)
            return dist2(p, v);
        var t = ((p.x - v.x) * (w.x - v.x) + (p.y - v.y) * (w.y - v.y)) / l2;
        if (t < 0)
            return dist2(p, v);
        if (t > 1)
            return dist2(p, w);
        return dist2(p, {x: v.x + t * (w.x - v.x),
                         y: v.y + t * (w.y - v.y)});
    }

    function distToSegment(p, v, w) {
        return Math.sqrt(distToSegmentSquared(p, v, w));
    }
    // this ends the section by Grumdig; mistakes after this are all mine

    function intersectSegmentToHalfPlane(seg, hp) {
        // Unpack arguments.
        var a = hp.a, b = hp.b, c = hp.c;
        var s0 = seg[0], x0 = s0.x, y0 = s0.y,
            s1 = seg[1], x1 = s1.x, y1 = s1.y;

        // Now the goal is to find the segment ((x0', y0'), (x1', y1'))
        // that is the intersection of
        // the segment          seg = ((x0, y0), (x1, y1))
        // and the half-plane   hp = {(x, y) | Ax + By + C >= 0}
        var c0 = a * x0 + b * y0 + c;
        var c1 = a * x1 + b * y1 + c;
        if (c0 < 0) {
            if (c1 < 0)
                return null;  // Neither endpoint is in hp.
            // s1 is in hp but s1 isn't.
            return [proportion_point(x0, y0, c0, x1, y1, c1), s1];
        } else if (c1 < 0) {
            // s0 is in hp but s1 isn't.
            return [s0, proportion_point(x0, y0, c0, x1, y1, c1)];
        } else {
            // Both endpoints (and thus all points in the segment) lie in hp.
            return seg;
        }

        function proportion_point(x0, y0, c0, x1, y1, c1) {
            var d = c0 / (c0 - c1);
            return {
                x: x0 + d * (x1 - x0),
                y: y0 + d * (y1 - y0)
            };
        }
    }

    function distToSegmentInWedge(p, dir, a, segment) {
        // 0 <= a < pi
        // But I think if a == 0, floating-point imprecision can probably cause
        // this to return null erroneously, so don't do that either.
        if (a < 0 || a >= Math.PI)
            throw new RangeError("invalid argument");

        // The first half-plane is the region to the left of
        // the ray from p in the direction a1.
        var a1 = dir - a/2, sin1 = Math.sin(a1), cos1 = Math.cos(a1);
        var hp1 = {a: -sin1, b:  cos1, c: p.x * sin1 - p.y * cos1};

        // The second half-plane is the region to the right of
        // the ray from p in the direction a2.
        var a2 = dir + a/2, sin2 = Math.sin(a2), cos2 = Math.cos(a2);
        var hp2 = {a:  sin2, b: -cos2, c: p.y * cos2 - p.x * sin2};

        // Simply truncate the segment to the part that intersects the wedge
        // (which is the intersection of the two half-planes).
        segment = intersectSegmentToHalfPlane(segment, hp1);
        if (segment === null)
            return null;
        segment = intersectSegmentToHalfPlane(segment, hp2);
        if (segment === null)
            return null;
        return distToSegment(p, segment[0], segment[1]);
    }

    function pointIsInWedge(p1, dir, a, p2) {
        // Half-planes are again the easiest way to settle this question.
        var a1 = dir - a/2;
        if ((p2.y - p1.y) * Math.cos(a1) < (p2.x - p1.x) * Math.sin(a1))
            return false;

        var a2 = dir + a/2;
        return (p2.y - p1.y) * Math.cos(a2) < (p2.x - p1.x) * Math.sin(a2);
    }

    function assert(x) { if (!x) throw new Error(); }
    assert(pointIsInWedge({x: 0, y: 0}, 0, Math.PI / 2, {x: 1, y: 0}));
    assert(pointIsInWedge({x: 0, y: 0}, 0, Math.PI / 2, {x: 2, y: 1}));
    assert(pointIsInWedge({x: 0, y: 0}, 0, Math.PI / 2, {x: 2, y: -1}));
    assert(!pointIsInWedge({x: 0, y: 0}, 0, Math.PI / 2, {x: -1, y: 0}));
    assert(!pointIsInWedge({x: 0, y: 0}, 0, Math.PI / 2, {x: 1, y: 2}));
    assert(!pointIsInWedge({x: 0, y: 0}, 0, Math.PI / 2, {x: 1, y: -2}));
    console.log("geometry tests passed");


    // === Bullets

    var BULLET_SPEED = 2;

    function Bullet(x, y, direction, turtle) {
        this.x0 = x;  // origin
        this.y0 = y;
        this.x = x;   // current position
        this.y = y;
        this.vx = BULLET_SPEED * Math.cos(direction);
        this.vy = BULLET_SPEED * Math.sin(direction);
        this.direction = direction;
        this.turtle = turtle;
        this.color =
            turtle.color === 'rgb(80,40,0)'
            ? 'rgba(255, 160, 20, 0.6)'
            : 'rgba(80, 255, 160, 0.4)';
    }

    Bullet.prototype = {
        paintTo: function paintTo(ctx) {
            var color = this.color;
            var x1 = this.x, y1 = this.y;
            var len_x = 25 * this.vx, len_y = 25 * this.vy;
            var x0, y0;
            if (Math.abs(len_x) > Math.abs(this.x - this.x0)) {
                len_x = this.x - this.x0;
                len_y = this.y - this.y0;
            }
            x0 = x1 - len_x;
            y0 = y1 - len_y;
            ctx.lineCap = "round";
            
            // Enhanced bullet with glow effect
            ctx.shadowBlur = 20;
            ctx.shadowColor = color;
            
            function draw(w, alpha) {
                ctx.lineWidth = w;
                ctx.beginPath();
                ctx.moveTo(x0, y0);
                ctx.lineTo(x1, y1);
                
                // Create gradient for bullet trail
                var gradient = ctx.createLinearGradient(x0, y0, x1, y1);
                if (color === 'rgba(255, 160, 20, 0.6)') {
                    gradient.addColorStop(0, `rgba(255, 200, 50, ${alpha * 0.3})`);
                    gradient.addColorStop(0.5, `rgba(255, 160, 20, ${alpha})`);
                    gradient.addColorStop(1, `rgba(255, 120, 0, ${alpha})`);
                } else {
                    gradient.addColorStop(0, `rgba(150, 255, 200, ${alpha * 0.3})`);
                    gradient.addColorStop(0.5, `rgba(80, 255, 160, ${alpha})`);
                    gradient.addColorStop(1, `rgba(0, 200, 100, ${alpha})`);
                }
                ctx.strokeStyle = gradient;
                ctx.stroke();
            }
            
            // Draw multiple layers for glow effect
            draw(8, 0.4);
            draw(6, 0.6);
            draw(4, 0.8);
            draw(2, 1.0);
            
            // Reset shadow
            ctx.shadowBlur = 0;
            ctx.lineWidth = 1;
        },

        step: function step(turtles) {
            var p = {x: this.x + this.vx, y: this.y + this.vy};
            for (var i = 0; i < turtles.length; i++) {
                var t = turtles[i];
                if (t !== this.turtle && distToSegmentSquared(t, this, p) < sqr(t.r)) {
                    // Return hit information instead of creating explosion directly
                    return {
                        hit: true,
                        turtle: t,
                        x: this.x,
                        y: this.y,
                        color: this.color
                    };
                }
            }
            this.x = p.x;
            this.y = p.y;
            return {hit: false};
        },

        isOutOfBounds: function isOutOfBounds(canvas) {
            if (!canvas) return;

            var result = (this.x < 0 || this.x > canvas.width || this.y < 0 || this.y > canvas.height);

            return result;
        }
    }

    // === Explosion System
    function Explosion(x, y, color) {
        this.x = x;
        this.y = y;
        this.color = color;
        this.particles = [];
        this.life = 30;
        
        // Create particles
        var particleCount = 20;
        for (var i = 0; i < particleCount; i++) {
            var angle = (Math.PI * 2 * i) / particleCount;
            var speed = 2 + Math.random() * 3;
            this.particles.push({
                x: x,
                y: y,
                vx: Math.cos(angle) * speed,
                vy: Math.sin(angle) * speed,
                size: 2 + Math.random() * 3,
                life: 20 + Math.random() * 10
            });
        }
    }

    Explosion.prototype = {
        paintTo: function paintTo(ctx) {
            this.particles.forEach(function(p) {
                if (p.life > 0) {
                    var alpha = p.life / 20;
                    ctx.fillStyle = this.color.replace(/[\d.]+\)$/, alpha + ')');
                    ctx.beginPath();
                    ctx.arc(p.x, p.y, p.size, 0, Math.PI * 2);
                    ctx.fill();
                }
            }.bind(this));
        },
        
        step: function step() {
            this.life--;
            this.particles.forEach(function(p) {
                if (p.life > 0) {
                    p.x += p.vx;
                    p.y += p.vy;
                    p.vx *= 0.95; // friction
                    p.vy *= 0.95;
                    p.life--;
                }
            });
            return this.life > 0;
        }
    };


    // === Turtles

    var TURTLE_SPEED = 1;
    var LookWall = 1, LookTurtle = 2;

    function Turtle(name, code, color) {
        var self = this;

        // Set up the global functions for turtle_lang.
        var g = Object.create(turtle_lang.globals);
        g.shoot = function () {
            self.game.bullets.push(self.shoot());  // i love oop
            return null;
        };
        g.rt1 = function () { self.h += Math.PI / 180; return null; };
        g.lt1 = function () { self.h -= Math.PI / 180; return null; };
        g.fd1 = function () {
            self.x += TURTLE_SPEED * Math.cos(self.h);
            self.y += TURTLE_SPEED * Math.sin(self.h);

            // If the turtle is out of bounds, move it back.
            var limit = self.r + WALL_THICKNESS;
            self.x = Math.max(limit, Math.min(self.x, self.game.canvas.width - limit));
            self.y = Math.max(limit, Math.min(self.y, self.game.canvas.height - limit));
            return null;
        };
        g.rt = turtle_lang.eval("{n => rep n {rt1!}}", g);
        g.lt = turtle_lang.eval("{n => rep n {lt1!}}", g);
        g.fd = turtle_lang.eval("{n => rep n {fd1!}}", g);

        g.look = function (dir) {
            if (typeof dir != 'number' || isNaN(dir))
                throw new TypeError("first argument to look must be a number");
            else if (dir === 1/0 || dir === -1/0)
                throw new RangeError("first argument to look can't be " + dir);
            return function (width) {
                if (typeof width != 'number' || isNaN(width))
                    throw new TypeError("second argument to look must be a number");
                if (width > 180)
                    width = 180;
                else if (width < 1)
                    width = 1;

                var absdir = self.h + dir * (Math.PI / 180);
                width *= Math.PI / 180;

                var closestType = null, minDistance = 1/0;
                self.game.turtles.forEach(function (t) {
                    if (t !== self && pointIsInWedge(self, absdir, width, t)) {
                        var dist = Math.sqrt(dist2(self, t));
                        if (dist < minDistance) {
                            minDistance = dist;
                            closestType = LookTurtle;
                        }
                    }
                });
                self.game.walls.forEach(function (w) {
                    var dist = distToSegmentInWedge(self, absdir, width, w);
                    if (dist !== null && dist < minDistance) {
                        minDistance = dist;
                        closestType = LookWall;
                    }
                });
                return {type: "pair", head: closestType, tail: minDistance};
            };
        };

        this.name = name;
        this.thread = new turtle_lang.Thread(code, g);

        this.x = 0;
        this.y = 0;
        this.h = 0; // heading
        this.r = 9; // Turtle radius
        this.color = color;
        this.health = 50;
    }

    Turtle.prototype = {
        paintTo: function paintTo(ctx) {
            ctx.save();
            ctx.translate(this.x, this.y);
            ctx.rotate(this.h);
            
            // Enhanced turtle design with shell pattern
            ctx.beginPath();
            
            // Shell body (more detailed)
            ctx.ellipse(0, 0, this.r * 1.2, this.r, 0, 0, Math.PI * 2);
            
            // Create gradient for shell
            var gradient = ctx.createRadialGradient(0, 0, 0, 0, 0, this.r * 1.2);
            if (this.color === 'rgb(80,40,0)') {
                gradient.addColorStop(0, 'rgb(120,60,20)');
                gradient.addColorStop(0.5, 'rgb(80,40,0)');
                gradient.addColorStop(1, 'rgb(40,20,0)');
            } else {
                gradient.addColorStop(0, 'rgb(80,200,80)');
                gradient.addColorStop(0.5, 'rgb(0,128,0)');
                gradient.addColorStop(1, 'rgb(0,80,0)');
            }
            ctx.fillStyle = gradient;
            ctx.fill();
            
            // Shell pattern
            ctx.strokeStyle = 'rgba(0,0,0,0.3)';
            ctx.lineWidth = 1;
            ctx.beginPath();
            ctx.arc(0, 0, this.r * 0.3, 0, Math.PI * 2);
            ctx.stroke();
            ctx.beginPath();
            ctx.arc(-this.r * 0.4, -this.r * 0.3, this.r * 0.2, 0, Math.PI * 2);
            ctx.stroke();
            ctx.beginPath();
            ctx.arc(this.r * 0.4, -this.r * 0.3, this.r * 0.2, 0, Math.PI * 2);
            ctx.stroke();
            ctx.beginPath();
            ctx.arc(-this.r * 0.4, this.r * 0.3, this.r * 0.2, 0, Math.PI * 2);
            ctx.stroke();
            ctx.beginPath();
            ctx.arc(this.r * 0.4, this.r * 0.3, this.r * 0.2, 0, Math.PI * 2);
            ctx.stroke();
            
            // Head (more detailed)
            ctx.beginPath();
            ctx.ellipse(this.r * 1.1, 0, this.r * 0.4, this.r * 0.3, 0, 0, Math.PI * 2);
            ctx.fillStyle = this.color;
            ctx.fill();
            ctx.strokeStyle = '#000';
            ctx.lineWidth = 2;
            ctx.stroke();
            
            // Eyes
            ctx.fillStyle = '#fff';
            ctx.beginPath();
            ctx.arc(this.r * 1.2, -this.r * 0.1, 2, 0, Math.PI * 2);
            ctx.fill();
            ctx.beginPath();
            ctx.arc(this.r * 1.2, this.r * 0.1, 2, 0, Math.PI * 2);
            ctx.fill();
            
            ctx.fillStyle = '#000';
            ctx.beginPath();
            ctx.arc(this.r * 1.2, -this.r * 0.1, 1, 0, Math.PI * 2);
            ctx.fill();
            ctx.beginPath();
            ctx.arc(this.r * 1.2, this.r * 0.1, 1, 0, Math.PI * 2);
            ctx.fill();
            
            // Legs
            ctx.fillStyle = this.color;
            ctx.strokeStyle = '#000';
            ctx.lineWidth = 1;
            
            // Front legs
            ctx.beginPath();
            ctx.ellipse(this.r * 0.3, -this.r * 0.8, this.r * 0.2, this.r * 0.3, Math.PI / 6, 0, Math.PI * 2);
            ctx.fill();
            ctx.stroke();
            ctx.beginPath();
            ctx.ellipse(this.r * 0.3, this.r * 0.8, this.r * 0.2, this.r * 0.3, -Math.PI / 6, 0, Math.PI * 2);
            ctx.fill();
            ctx.stroke();
            
            // Back legs
            ctx.beginPath();
            ctx.ellipse(-this.r * 0.3, -this.r * 0.8, this.r * 0.2, this.r * 0.3, -Math.PI / 6, 0, Math.PI * 2);
            ctx.fill();
            ctx.stroke();
            ctx.beginPath();
            ctx.ellipse(-this.r * 0.3, this.r * 0.8, this.r * 0.2, this.r * 0.3, Math.PI / 6, 0, Math.PI * 2);
            ctx.fill();
            ctx.stroke();
            
            // Tail
            ctx.beginPath();
            ctx.moveTo(-this.r * 1.1, 0);
            ctx.lineTo(-this.r * 1.4, -this.r * 0.1);
            ctx.lineTo(-this.r * 1.4, this.r * 0.1);
            ctx.closePath();
            ctx.fillStyle = this.color;
            ctx.fill();
            ctx.stroke();
            
            ctx.restore();
        },

        shoot: function shoot() {
            return new Bullet(this.x, this.y, this.h, this);
        },

        takeDamage: function takeDamage() {
            this.health--;
            
            // Update health bar UI
            var healthPercent = Math.max(0, (this.health / 50) * 100);
            var healthBarId = this.name === 'brown' ? 'brownHealth' : 'greenHealth';
            var healthTextId = this.name === 'brown' ? 'brownHp' : 'greenHp';
            
            var healthBar = document.getElementById(healthBarId);
            var healthText = document.getElementById(healthTextId);
            if (healthBar) healthBar.style.width = healthPercent + '%';
            if (healthText) healthText.textContent = this.health + '/50';
            
            if (this.health <= 0) {
                this.game.stop();
                this.game.showWinner(this.name === 'brown' ? 'green' : 'brown');
            }
        }
    };


    // === Game

    function Game(turtles, canvas) {
        // the state of a game is: the position of the turtles, their internal
        // program execution state, and the state of the bullets
        this.turtles = turtles;
        this.bullets = [];
        this.explosions = [];
        this.canvas = canvas;
        this.w = canvas.width;
        this.h = canvas.height;
        this.alive = true;
        this.lastUpdate = Date.now();

        // The corners of the arena.
        var c = [
            {x: 0, y: 0},
            {x: this.w, y: 0},
            {x: this.w, y: this.h},
            {x: 0, y: this.h}
        ];
        this.walls = [
            [c[0], c[1]],
            [c[1], c[2]],
            [c[2], c[3]],
            [c[3], c[0]],
        ];

        var self = this;
        this.tick_callback = function () {
            if (self.alive) {
                self.update();
                setTimeout(self.tick_callback, 4);
            }
        };
        this.paint_callback = function () {
            if (self.alive) {
                self.update();
                self.paint();
                requestAnimFrame(self.paint_callback);
            }
        };
    }

    var WALL_THICKNESS = 4;

    Game.prototype = {
        start: function start() {
            var self = this;
            this.turtles.forEach(function(t) {
                t.game = self;
            });
            this.paint_callback();
            this.tick_callback();
        },

        paint: function paint() {
            var self = this;
            var ctx = this.canvas.getContext('2d');
            
            // Enhanced arena background
            ctx.fillStyle = "rgb(0,0,0)";
            ctx.fillRect(0, 0, this.w, this.h);
            
            // Create gradient background for playing field
            var gradient = ctx.createRadialGradient(this.w/2, this.h/2, 0, this.w/2, this.h/2, Math.max(this.w, this.h)/2);
            gradient.addColorStop(0, "rgb(35,100,20)");
            gradient.addColorStop(0.5, "rgb(23,84,7)");
            gradient.addColorStop(1, "rgb(15,60,5)");
            ctx.fillStyle = gradient;
            
            var W = WALL_THICKNESS;
            ctx.fillRect(W, W, this.w - (2*W), this.h - (2*W));
            
            // Add grid pattern
            ctx.strokeStyle = "rgba(0,0,0,0.1)";
            ctx.lineWidth = 1;
            var gridSize = 50;
            for (var x = W; x < this.w - W; x += gridSize) {
                ctx.beginPath();
                ctx.moveTo(x, W);
                ctx.lineTo(x, this.h - W);
                ctx.stroke();
            }
            for (var y = W; y < this.h - W; y += gridSize) {
                ctx.beginPath();
                ctx.moveTo(W, y);
                ctx.lineTo(this.w - W, y);
                ctx.stroke();
            }
            
            // Add decorative obstacles
            ctx.fillStyle = "rgba(0,0,0,0.3)";
            // Top-left obstacle
            ctx.beginPath();
            ctx.arc(W + 80, W + 80, 30, 0, Math.PI * 2);
            ctx.fill();
            // Top-right obstacle
            ctx.beginPath();
            ctx.arc(this.w - W - 80, W + 80, 30, 0, Math.PI * 2);
            ctx.fill();
            // Bottom-left obstacle
            ctx.beginPath();
            ctx.arc(W + 80, this.h - W - 80, 30, 0, Math.PI * 2);
            ctx.fill();
            // Bottom-right obstacle
            ctx.beginPath();
            ctx.arc(this.w - W - 80, this.h - W - 80, 30, 0, Math.PI * 2);
            ctx.fill();
            // Center obstacle
            ctx.beginPath();
            ctx.arc(this.w/2, this.h/2, 40, 0, Math.PI * 2);
            ctx.fill();

            this.turtles.forEach(function (t) {
                t.paintTo(ctx);
            });

            this.bullets.forEach(function (b) {
                b.paintTo(ctx);
            });

            this.explosions.forEach(function (e) {
                e.paintTo(ctx);
            });
        },

        update: function update() {
            var now = Date.now();
            var dt = now - this.lastUpdate;
            this.lastUpdate = now;

            for (var i = 0; this.alive && i < dt; i++) {
                this.turtles.forEach(function (t) {
                    if (t.thread.alive)
                        t.thread.step();
                });
                for (var j = 0; j < this.bullets.length;) {
                    var result = this.bullets[j].step(this.turtles);
                    if (result.hit) {
                        // Handle turtle damage
                        result.turtle.takeDamage();
                        // Create explosion at hit location
                        this.explosions.push(new Explosion(result.x, result.y, result.color));
                        // Remove the bullet
                        this.bullets.splice(j, 1);
                    } else {
                        j++;
                    }
                }
                
                // Update explosions
                for (var k = 0; k < this.explosions.length;) {
                    if (!this.explosions[k].step())
                        this.explosions.splice(k, 1);
                    else
                        k++;
                }
            }
        },

        stop: function stop() {
            this.alive = false;
        },
        
        showWinner: function showWinner(winnerName) {
            var statusEl = document.getElementById('gameStatus');
            if (statusEl) {
                statusEl.textContent = '🏆 ' + winnerName.toUpperCase() + ' WINS!';
                statusEl.classList.add('show');
                setTimeout(function() {
                    statusEl.classList.remove('show');
                }, 3000);
            }
        },
        
        resetHealthBars: function resetHealthBars() {
            document.getElementById('brownHealth').style.width = '100%';
            document.getElementById('greenHealth').style.width = '100%';
            document.getElementById('brownHp').textContent = '50/50';
            document.getElementById('greenHp').textContent = '50/50';
        }
    };

    return {Turtle: Turtle, Game: Game};
})();
