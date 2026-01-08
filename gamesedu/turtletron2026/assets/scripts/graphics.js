// Turtletron Graphics Enhancement Module
// Handles sprite rendering and visual effects

class TurtleGraphics {
    constructor() {
        this.sprites = {
            green: null,
            red: null,
            blue: null
        };
        this.useSprites = false;
        this.animationFrame = 0;
        this.particles = [];
    }
    
    // Load sprite images
    async loadSprites() {
        try {
            this.sprites.green = await this.loadImage('assets/images/turtles/turtle-green.svg');
            this.sprites.red = await this.loadImage('assets/images/turtles/turtle-red.svg');
            this.sprites.blue = await this.loadImage('assets/images/turtles/turtle-blue.svg');
            this.useSprites = true;
            console.log('Turtle sprites loaded successfully');
        } catch (error) {
            console.log('Failed to load sprites, using fallback rendering');
            this.useSprites = false;
        }
    }
    
    loadImage(src) {
        return new Promise((resolve, reject) => {
            const img = new Image();
            img.onload = () => resolve(img);
            img.onerror = reject;
            img.src = src;
        });
    }
    
    // Enhanced turtle rendering with sprites or fallback
    drawTurtle(ctx, obj) {
        ctx.save();
        
        // Translate to turtle position and rotate
        ctx.translate(obj.x, obj.y);
        ctx.rotate(obj.angle * Math.PI / 180);
        
        if (this.useSprites) {
            // Use sprite rendering
            const sprite = this.getSpriteForPlayer(obj);
            if (sprite) {
                ctx.drawImage(sprite, -20, -20, 40, 40);
            }
        } else {
            // Fallback to enhanced triangle rendering
            this.drawEnhancedTriangle(ctx, obj);
        }
        
        ctx.restore();
    }
    
    // Get appropriate sprite for player
    getSpriteForPlayer(obj) {
        if (obj === player1) return this.sprites.green;
        if (obj === player2) return this.sprites.red;
        return this.sprites.blue;
    }
    
    // Enhanced triangle rendering (fallback)
    drawEnhancedTriangle(ctx, obj) {
        // Set colors based on player
        let strokeColor, fillColor;
        if (obj === player1) {
            strokeColor = '#00FF00';
            fillColor = 'rgba(0, 255, 0, 0.3)';
        } else if (obj === player2) {
            strokeColor = '#FF0000';
            fillColor = 'rgba(255, 0, 0, 0.3)';
        } else {
            strokeColor = '#0066FF';
            fillColor = 'rgba(0, 102, 255, 0.3)';
        }
        
        // Draw turtle body
        ctx.fillStyle = fillColor;
        ctx.strokeStyle = strokeColor;
        ctx.lineWidth = 3;
        ctx.lineCap = 'round';
        ctx.lineJoin = 'round';
        
        // Draw triangle shape
        ctx.beginPath();
        ctx.moveTo(15, 0);  // Tip
        ctx.lineTo(-10, -12);  // Base corner 1
        ctx.lineTo(-10, 12);   // Base corner 2
        ctx.closePath();
        
        ctx.fill();
        ctx.stroke();
        
        // Draw eye
        ctx.fillStyle = '#FFFFFF';
        ctx.strokeStyle = strokeColor;
        ctx.lineWidth = 1;
        ctx.beginPath();
        ctx.arc(5, -3, 3, 0, Math.PI * 2);
        ctx.fill();
        ctx.stroke();
        
        // Draw pupil
        ctx.fillStyle = '#000000';
        ctx.beginPath();
        ctx.arc(5, -3, 1.5, 0, Math.PI * 2);
        ctx.fill();
        
        // Draw direction indicator
        ctx.fillStyle = 'rgba(255, 255, 255, 0.8)';
        ctx.beginPath();
        ctx.moveTo(15, 0);
        ctx.lineTo(10, -5);
        ctx.lineTo(10, 5);
        ctx.closePath();
        ctx.fill();
    }
    
    // Add particle effect for visual feedback
    addParticle(x, y, color, type = 'burst') {
        const particle = {
            x: x,
            y: y,
            color: color,
            type: type,
            life: 1.0,
            maxLife: 1.0,
            size: 5,
            velocity: { x: 0, y: 0 }
        };
        
        if (type === 'burst') {
            // Create burst effect
            for (let i = 0; i < 8; i++) {
                const angle = (Math.PI * 2 * i) / 8;
                const speed = 2 + Math.random() * 2;
                this.particles.push({
                    ...particle,
                    velocity: {
                        x: Math.cos(angle) * speed,
                        y: Math.sin(angle) * speed
                    }
                });
            }
        } else {
            this.particles.push(particle);
        }
    }
    
    // Update and render particles
    updateParticles(ctx) {
        this.particles = this.particles.filter(particle => {
            particle.life -= 0.02;
            
            if (particle.life <= 0) return false;
            
            // Update position
            particle.x += particle.velocity.x;
            particle.y += particle.velocity.y;
            particle.velocity.x *= 0.98; // Friction
            particle.velocity.y *= 0.98;
            
            // Render particle
            ctx.save();
            ctx.globalAlpha = particle.life;
            ctx.fillStyle = particle.color;
            ctx.beginPath();
            ctx.arc(particle.x, particle.y, particle.size * particle.life, 0, Math.PI * 2);
            ctx.fill();
            ctx.restore();
            
            return true;
        });
    }
    
    // Add grid background
    drawGrid(ctx, width, height, spacing = 30) {
        ctx.save();
        ctx.strokeStyle = 'rgba(200, 200, 200, 0.3)';
        ctx.lineWidth = 1;
        
        // Draw vertical lines
        for (let x = 0; x <= width; x += spacing) {
            ctx.beginPath();
            ctx.moveTo(x, 0);
            ctx.lineTo(x, height);
            ctx.stroke();
        }
        
        // Draw horizontal lines
        for (let y = 0; y <= height; y += spacing) {
            ctx.beginPath();
            ctx.moveTo(0, y);
            ctx.lineTo(width, y);
            ctx.stroke();
        }
        
        ctx.restore();
    }
    
    // Add visual feedback for collisions
    showCollisionEffect(ctx, x, y) {
        this.addParticle(x, y, '#FF6600', 'burst');
        
        // Flash effect
        ctx.save();
        ctx.globalAlpha = 0.3;
        ctx.fillStyle = '#FF6600';
        ctx.beginPath();
        ctx.arc(x, y, 30, 0, Math.PI * 2);
        ctx.fill();
        ctx.restore();
    }
    
    // Add visual feedback for line drawing
    showLineEffect(ctx, startX, startY, endX, endY, color) {
        // Add small particles along the line
        const distance = Math.sqrt(Math.pow(endX - startX, 2) + Math.pow(endY - startY, 2));
        const steps = Math.max(1, Math.floor(distance / 20));
        
        for (let i = 0; i <= steps; i++) {
            const t = i / steps;
            const x = startX + (endX - startX) * t;
            const y = startY + (endY - startY) * t;
            
            if (Math.random() > 0.7) { // Only show some particles
                this.addParticle(x, y, color, 'trail');
            }
        }
    }
}

// Create global graphics instance
const turtleGraphics = new TurtleGraphics();

// Initialize graphics when page loads
window.addEventListener('load', () => {
    turtleGraphics.loadSprites();
});