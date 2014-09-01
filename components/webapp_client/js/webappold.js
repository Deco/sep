// SEP!

var pixel_size = 1;
var exampleEventStream = [
    { time: 0, type: 'start' },
    { time: 0.3315, type: 'ambient_temp_recording', ambient_temp:  40 },
    { time: 0.3319, type: 'ambient_temp_recording', ambient_temp:  48 },
    { time: 0.3455, type: 'ambient_temp_recording', ambient_temp:  33 },
    { time: 0.3715, type: 'ambient_temp_recording', ambient_temp:  55 },
    { time: 0.4115, type: 'ambient_temp_recording', ambient_temp:  30 },
    { time: 0.4615, type: 'ambient_temp_recording', ambient_temp:  12 },
    { time: 1.3615, type: 'ambient_temp_recording', ambient_temp:   0 },
    { time: 1.7525, type: 'ambient_temp_recording', ambient_temp: 140 },
    { time: 2.0000, type: 'thermopile_reading',
        pitch: 0, yaw: 30, pixel_size: pixel_size,
        temperature_grid: [
            [1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4],
            [1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4],
            [1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4],
            [1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4]
        ],
    },
    { time: 2.4000, type: 'thermopile_reading',
        pitch: 15, yaw: 50, pixel_size: pixel_size,
        temperature_grid: [
            [1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4],
            [1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4],
            [1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4],
            [1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4]
        ],
    },
    { time: 3.0000, type: 'thermopile_reading',
        pitch: -7, yaw: 167, pixel_size: pixel_size,
        temperature_grid: [
            [1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4],
            [1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4],
            [1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4],
            [1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4]
        ],
    },
];

(function() {
    for(var i = 0; i < 250; i++) {
        exampleEventStream.push(
            { time: 4+i*0.05, type: 'thermopile_reading',
                pitch: -50+100*Math.random(),
                yaw  : -180+360*Math.random(),
                pixel_size: pixel_size,
                temperature_grid: [
                    [1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4],
                    [1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4],
                    [1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4],
                    [1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4]
                ],
            }
        );
    }
})();

JS.require('JS.Class', 'JS.Observable', function(Class) {
    
    var ApplicationUI = new Class({
        initialize: function(app) {
            this.app = app;
            this.buildMenuLayer();
            this.buildViewportLayer();
            this.buildOverlayLayer();
        },
        buildMenuLayer: function() {
            this.menuLayer = $("#menu-layer");
            this.menuLayer.css("pointer-events", "none");
            $("#menu-layer *").css("pointer-events", "initial");
            $("#close-popout-button").click(function(){
                $(".popout-pane").fadeOut(200);
            });
            $(".sidebar-button").click(function(){
                $(".popout-pane").fadeIn(200);
            });
        },
        buildViewportLayer: function() {
            this.pr = new PanoramaRenderer($('#maincanvascontainer'));
        },
        buildOverlayLayer: function() {
            this.svg = SVG('svgdoc-layer').size('100%', '100%');
            this.svg.rect(100,100)
                .draggable()
                .animate().fill('#f02').move(100,100)
            ;
            
            this.timeSlider = $("#timeslider");
            this.timeSlider.noUiSlider({
                start: [20, 80],
                connect: true,
                range: { 'min': 0, 'max': 0 }
            });
            
            var self = this;
            this.timeSlider.on({
                slide: function(){
                    var timeRange = self.timeSlider.val();
                    self.app.setEventTimeRange(timeRange[0], timeRange[1]);
                },
            });
        },
        updateTimeSlideRange: function(minTime, maxTime) {
            this.timeSlider.noUiSlider({
                range: { 'min': minTime, 'max': maxTime+1 }
            }, true);
        },
        updateEventDisplay: function() {
            $('#output').empty();
            this.app.relaventEvents.forEach(function(event, eventI){
                var el = $('<div></div>');
                el.text(event.time+': '+event.type);
                
                $('#output').append(el);
            });
            
            this.pr.startReset();
            var self = this;
            this.app.relaventEvents.forEach(function(event, eventI){
                self.pr.drawEvent(event);
            });
            this.pr.endReset();
        },
        update: function(currTime, deltaTime) {
            this.pr.update(currTime, deltaTime);
        },
    });
    
    var Application = new Class({
        initialize: function() {
            this.ui = new ApplicationUI(this);
            
            this.events = [];
            this.relaventEvents = [];
            this.minTime = 0; this.maxTime = 0;
        },
        loadEvents: function(events) {
            this.events = events;
            this.processEvents();
        },
        processEvents: function() {
            this.relaventEvents = this.events;
            
            this.minTime = 0;
            this.maxTime = this.events[this.events.length-1].time;
            
            this.ui.updateTimeSlideRange(this.minTime, this.maxTime);
        },
        setEventTimeRange: function(minTime, maxTime) {
            this.relaventEvents = this.events.filter(function(ev){
                return minTime <= ev.time && ev.time <= maxTime
            });
            
            this.ui.updateEventDisplay();
        },
        update: function(currTime, deltaTime) {
            this.ui.update(currTime, deltaTime);
        }, 
    });
    
    $(document).ready(function(){
        var app = new Application();
        app.loadEvents(exampleEventStream);
        
        //app.connectToEventSource(jdsklfjadklfjadslkfjasdf);
        
        var fpsDisplay = new Stats();
        fpsDisplay.setMode(0);
        fpsDisplay.domElement.style.position = 'absolute';
        fpsDisplay.domElement.style.top = '0px';
        fpsDisplay.domElement.style.left = '0px';
        document.body.appendChild(fpsDisplay.domElement);
        
        var prevTime = 0;
        function animate() {
            var currTime = (new Date()).getTime();
            var deltaTime = currTime-prevTime;
            
            fpsDisplay.begin();
            app.update(currTime, deltaTime);
            fpsDisplay.end();
            
            prevTime = currTime;
            
            requestAnimationFrame(animate);
        }
        animate();
    });
});

/*var maincanvas = document.getElementById("maincanvas");
var maincanvas_ctx = maincanvas.getContext("2d");
maincanvas_ctx.rect(0, 0, maincanvas.width, maincanvas.height);
maincanvas_ctx.fillStyle = "gray";
maincanvas_ctx.fill();*/

function buildWSService() {
    /*var connection = new autobahn.Connection({
        url: 'ws://127.0.0.1:9000/',
        realm: 'realm1'
    });
    connection.onopen = function(session){
        
        session.subscribe('com.myapp.hello', function(args){
            console.log("Event:", args[0]);
        });
        session.publish('com.myapp.hello', ['Hello, world!']);
        
        
        session.register('com.myapp.add2', function(args){
            return args[0] + args[1];
        });
        session.call('com.myapp.add2', [2, 3])
            .then(function(res){
                console.log("Result:", res);
            })
        ;
        
    };
    
    connection.open();
    */
}
