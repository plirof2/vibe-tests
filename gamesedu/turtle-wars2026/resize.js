$(function () {
    var isDragging = false;
    var startX = 0;
    var startY = 0;
    var startArenaWidth = 0;
    var startArenaHeight = 85;
    var currentRatio = 85;
    var headerHeight = 0;
    
    // Browser compatibility detection
    function isOldChrome() {
        var match = navigator.userAgent.match(/Chrome\/([0-9]+)/);
        return match && parseInt(match[1]) < 68;
    }
    
    if (isOldChrome()) {
        $('body').addClass('old-browser');
    }
    
    // Get panel widths
    function getLeftPanelWidth() {
        return $('#leftPanel').length ? $('#leftPanel').outerWidth() : 200;
    }
    
    function getRightPanelWidth() {
        return $('#rightPanel').length ? $('#rightPanel').outerWidth() : 200;
    }
    
    // Load saved ratio from localStorage
    try {
        var savedRatio = localStorage.getItem('turtleWarsArenaRatio');
        if (savedRatio) {
            currentRatio = parseInt(savedRatio);
            startArenaHeight = currentRatio;
        }
    } catch (e) {
        // Fallback if localStorage fails
        console.log('LocalStorage not available, using default ratio');
    }
    
    // Initialize layout after DOM is ready
    updateLayout(currentRatio || 85);
    
    function updateLayout(arenaPercent) {
        currentRatio = arenaPercent;
        var leftPanelWidth = getLeftPanelWidth();
        var rightPanelWidth = getRightPanelWidth();
        
        // Update arena position and height
        $('#arena').css({
            'left': leftPanelWidth + 'px',
            'right': rightPanelWidth + 'px',
            'height': arenaPercent + '%'
        });
        
        // Calculate code area position (below arena)
        var codeHeight = Math.max(15, 100 - arenaPercent - 2); // 2% buffer
        $('#turtle1, #turtle2').css('height', codeHeight + '%');
        
        // Position code areas accounting for side panels
        $('#turtle1').css('left', '0');
        $('#turtle2').css('right', rightPanelWidth + 'px');
        
        // Update slider and display
        $('#resizeSlider').val(arenaPercent);
        $('#arenaRatio').text(arenaPercent + '%');
        
        // Update preset button active state
        $('.preset-buttons button').removeClass('active');
        $('.preset-buttons button[data-ratio="' + arenaPercent + '"]').addClass('active');
        
        // Save to localStorage
        try {
            localStorage.setItem('turtleWarsArenaRatio', arenaPercent);
        } catch (e) {
            console.log('Could not save to localStorage');
        }
    }
    
    // No splitter functionality needed in new layout
    
    
    
    // Slider control
    $('#resizeSlider').on('input', function() {
        updateLayout(parseInt($(this).val()));
    });
    
    // Preset buttons
    $('.preset-buttons button').on('click', function() {
        var ratio = parseInt($(this).data('ratio'));
        updateLayout(ratio);
    });
    
    // Window resize handler
    $(window).on('resize', function() {
        // Recalculate header height and layout
        headerHeight = getHeaderHeight();
        updateLayout(currentRatio);
    });
    
    // Initialize layout after a small delay to ensure DOM is ready
    setTimeout(function() {
        headerHeight = getHeaderHeight();
        updateLayout(currentRatio);
    }, 100);
});