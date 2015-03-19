
Math.logb = function(n, b) {
    return Math.log(n) / Math.log(b);
}

JS.require('JS.Class', 'JS.Observable', function(Class) {
    window.App = {
        // Enumerator for touch event handling
        MODE_MANUAL_CONTROL: 0,
        MODE_DRAW_RECTANGLE: 1,
        MODE_DRAW_SQUIGGLE: 2,
        MODE_MOVE_ACTUATOR: 3,
        //  The mode controls how the system interacts with mouse/touch events
        //  throughout the application. The mode can be set when various menu 
        //  options are selected.

        //  For storing old position    
        oldPos: { x: 0, y: 0 },
        startPoint: { x: 0, y: 0},
        endPoint: { x: 0, y: 0},
        // Main Menu structure
        mainButtonInfoPage: {
            pageTitle: "Main Menu",
            buttonList: [
                { label: "Manual Control", cb: function() {
                    App.changeSidebarButtonPage(App.manControlButtonPage);
                    App.mode = App.MODE_MANUAL_CONTROL;
                }, },
                { label: "Scan Region", cb: function() {
                    App.changeSidebarButtonPage(App.scanControlButtonPage);
                }, },
                { label: "Full <br/> Scan", cb: function() {
                    App.changeSidebarButtonPage(App.fullScanControlButtonPage);
                }, },
                { label: "Move <br/> Actuator", cb: function() {
                    App.changeSidebarButtonPage(App.placeProbeButtonPage);
                    App.mode = App.MODE_MOVE_ACTUATOR;
                }, },
                { label: "Quick Settings", cb: function() {
                    App.openSettings(App.quickSettingsPage);
                }, },
                { label: "Advanced Settings", cb: function() {
                    App.openSettings(App.advSettingsPage);
                }, },
            ], 
        },
        
        // Manual Control menu structure 
        manControlButtonPage: {
            pageTitle: "Manual Control",
            buttonList: [
                { label: "Back", cb: function() { App.changeSidebarButtonPage(App.mainButtonInfoPage); }, }, 
            ], 
        },
    
        // Place Probe menu structure
        placeProbeButtonPage: {
            pageTitle: "Move Actuator",
            buttonList: [
                { label: "Back", cb: function() { App.changeSidebarButtonPage(App.mainButtonInfoPage); }, },
            ],
        },
        
        // Region Scanning menu structure 
        scanControlButtonPage: {
            pageTitle: "Scan Region",
            buttonList: [
                { label: "Draw Rectangle", cb: function() {
                    App.mode = App.MODE_DRAW_RECTANGLE;
                }, },
                { label: "Draw Squiggle", cb: function() {
                    App.mode = App.MODE_DRAW_SQUIGGLE;
                }, },
                { label: "Start Scanning", cb: function() { }, },
                { label: "Stop Scanning" , cb: function() { }, },
                { label: "Back", cb: function() { 
                    App.changeSidebarButtonPage(App.mainButtonInfoPage); 
                    App.mode = null;
                }, },
            ], 
        },
        
        // Full Scan menu structure
        fullScanControlButtonPage: {
            pageTitle: "Full Scan",
            buttonList: [
                { label: "Start Full Scan", sb: function() { }, },
                { label: "Stop Scanning", sb: function() { }, },
                { label: "Back", cb: function() { App.changeSidebarButtonPage(App.mainButtonInfoPage); }, },
            ],
        },
        
        // Quick Settings menu page
        quickSettingsPage: {
            paneTitle: "<b>Quick Settings</b>",
            content: [
                { label: "what", value: "10"},
                { label: "Priority", value: "Speed"},
                { label: "Slideyman", value: "<div class='settings-sliders-regular' contenteditable='false'></div>"},
                { label: "Toggle me", value: "<div class='settings-sliders-toggle' contenteditable='false'></div>"},
                { label: "Range slider", value: "<div class='settings-sliders-range' contenteditable='false'></div>"},
            ],
        },
        
        // Advanced Settings menu page
        advSettingsPage: {
            paneTitle: "<b>Advanced Settings</b>",
            content: [
                { label: "blah blah", value: "25"},
                { label: "Captain", value: "Falcon"},
                { label: "Dark", value: "Pit"},
                { label: "string", value: "Wow this feels stringy"},
                { label: "string", value: "Wow this feels stringy"},
                { label: "string", value: "Wow this feels stringy"},
                { label: "string", value: "Wow this feels stringy"},
                { label: "string", value: "Wow this feels stringy"},
                { label: "string", value: "Wow this feels stringy"},
                { label: "string", value: "Wow this feels stringy"},
                { label: "string", value: "Wow this feels stringy"},
                { label: "string", value: "Wow this feels stringy"},
                { label: "string", value: "Wow this feels stringy"},
                { label: "string", value: "Wow this feels stringy"},
                { label: "string", value: "Wow this feels stringy"},
                { label: "string", value: "Wow this feels stringy"},
                { label: "string", value: "Wow this feels stringy"},
                { label: "string", value: "Wow this feels stringy"},
                { label: "string", value: "Wow this feels stringy"},
                { label: "string", value: "Wow this feels stringy"},
                { label: "string", value: "Wow this feels stringy"},
                { label: "string", value: "Wow this feels stringy"},
            ],
        },
    };
    
    App.onDocumentReady = function() {
        
        // Variable init
        //App.mode = this.MODE_MANUAL_CONTROL;HAHA
        //App.mode = this.MODE_DRAW_RECTANGLE; // BHALRHGLAHBLAHRLGAHLRGHALBHALRHGLAHBLARHGLAHBLARHG
        this.isDragging = false;
        
        // Comment me!
        var $allLayers = $(".layer");
        $allLayers.css("pointer-events", "none");
        $(".layer *").css("pointer-events", "initial");
        
        // FIX ME FIX ME
        this.$svgLayer = $("#svg-layer");
        this.$svgLayer.css("pointer-events", "initial");
        // FIX ME FIX ME
        
        // Comment me!
        this.$menuLayer = $("#menu-layer");
        this.$mainCanvas = $("#main-canvas");
    
        // Comment me!
        this.cameraTickExpPower = 1/Math.exp(1);
    
        // Comment me!
        this.$svgLayer.bind('touchstart', function(event) { App.onViewportEventStart(event, "touch") });
        this.$svgLayer.bind('touchmove', function(event) { App.onViewportEventMove(event, "touch") });
        this.$svgLayer.bind('touchend'  , function(event) { App.onViewportEventEnd(event, "touch") });
        
        
        this.$svgLayer.bind('mousedown', function(event) { App.onViewportEventStart(event, "mouse") });
        this.$svgLayer.bind('mousemove', function(event) { App.onViewportEventMove(event, "mouse") });
        this.$svgLayer.bind('mouseup'  , function(event) { App.onViewportEventEnd(event, "mouse") });
        
        // Comment me!
        this.changeSidebarButtonPage(this.mainButtonInfoPage);
    
        // Comment me!
        this.pr = new PanoramaRenderer($('#main-canvas-container'));

        this.wsUri = "ws://"+document.location.host;
        this.ws = new WebSocket(this.wsUri);

        this.wsIsConnected = false;

        if(true) {
            var self = this;
            this.ws.onopen = function (event) {
                //self.ws.send("AAAAAHHH");
                self.wsIsConnected = true;
            }

            this.ws.onmessage = function(ev) {
                //console.log("WS receive: "+ev.data);
                var msg = JSON.parse(ev.data);
                if(msg.type == "thermo_data") {
                    var dataWidth = 4;//16;
                    var dataHeight = 16;//4;
                    var bufferArray = self.convertB64ToBuffer(msg.data);
                    var byteArray = new Uint8Array(bufferArray);
                    var reading = {
                        data: byteArray,
                        dataWidth: 16,
                        dataHeight: 4,
                        lon: THREE.Math.degToRad(msg.yaw), lonSize: THREE.Math.degToRad(60),
                        lat: THREE.Math.degToRad(msg.pitch), latSize: THREE.Math.degToRad(16.4),
                    };
                    self.pr.drawReading(reading);
                } else if(msg.type == "rgb_data") {
                    var dataWidth = msg.dataWidth;
                    var dataHeight = msg.dataHeight;
                    var bufferArray = self.convertB64ToBuffer(msg.data);
                    var byteArray = new Uint8Array(bufferArray);
                    var reading = {
                        data: byteArray,
                        dataWidth: dataWidth,
                        dataHeight: dataHeight,
                        lon: THREE.Math.degToRad(msg.yaw), lonSize: THREE.Math.degToRad(msg.yawSize),
                        lat: THREE.Math.degToRad(msg.pitch), latSize: THREE.Math.degToRad(msg.pitchSize),
                    };
                    self.pr.drawReading(reading);
                }
            }
        }

        var prevTime = 0;
        var self = this;
        function __app_animate() {
            var currTime = (new Date()).getTime();
            var deltaTime = currTime-prevTime;
            self.onAnimationFrame(currTime, deltaTime);
            prevTime = currTime;
            requestAnimationFrame(__app_animate);
        }
        
        //this.$rectElement = null;
        this.svgContext = SVG('main-svg-container');
        
        /*this.testingRect = this.svgContext.rect(100, 100).attr({
            x: 50, y: 50,
            width: 100, height: 100,
            fill: 'transparent',
            stroke: 'red',
            'stroke-width': 5,
        });*/
        
        __app_animate();

    };
    
    App.onAnimationFrame = function(currTime, deltaTime) {
        this.pr.update(currTime, deltaTime);
        
        this.updateTickbars(currTime, deltaTime);
        
        
    };
    
    /*App.updateCameraTicks = function() {
        var x = 1;
        var xStr = "";
        
        var height = 5;
        var heightStr = "";
        var ticksArray = [];
        for(i=0; i < 7; i++) {
            xStr += x + "%";
            heightStr = 92.5 + ((7-i)*0.6) + "%";
            console.log(heightStr);
            this.panTicks = this.svgContext.rect(0, 0);
            this.panTicks.attr({ 
                x: xStr, y: heightStr,
                width: "0.5%", height: "7.5%",
                fill: 'white',
                'fill-opacity': 0.5,
                stroke: 'black',
                'stroke-width': 1,
            });
            ticksArray[ticksArray.length] = this.panTicks;
            console.log(ticksArray[i].attr('x'));
            x = x*1.8;
            xStr = "";
            heightStr = "";
        }
        
        this.panTicks = this.svgContext.rect(0, 0);
        this.panTicks.attr({ 
            x: "47.5%", y: "92.5%",
            width: "0.5%", height: "7.5%",
            fill: 'white',
            'fill-opacity': 0.5,
            stroke: 'black',
            'stroke-width': 1,
        });
        ticksArray[ticksArray.length] = this.panTicks;
        
        for(j=6; j >= 0; j--) {
            x = 47.5-parseFloat(ticksArray[j].attr('x'));
            xStr += x+47.5 + "%";
            console.log(x);
            this.panTicks = this.svgContext.rect(0, 0);
            this.panTicks.attr({ 
                x: xStr, y: "95%",
                width: "0.5%", height: "7.5%",
                fill: 'white',
                'fill-opacity': 0.5,
                stroke: 'black',
                'stroke-width': 1,
            });
            ticksArray[ticksArray.length] = this.panTicks;
            //console.log(ticksArray.length);
            xStr = "";
        }
        
        //ticksArray[9].animate().move("50%", "95%");
    };*/
    
    App.updateTickbars = function() {
        
        // tick = individual tick
        // tickbar = a bar of ticks (either lattitude or longitude)
        
        var shouldCreateTickbars = false;
        if(!this.areCameraTickbarsCreated) {
            this.areCameraTickbarsCreated = true;
            shouldCreateTickbars = true;
            
            this.cameraTickbarMap = {};
        }
        
        for(var tickbarI = 0; tickbarI < 2; tickbarI++) {
            // tickbarI == 0 -> longitude ticks
            // tickbarI == 1 -> latitude ticks
            
            var tickbarName = (tickbarI == 0 ? "lon" : "lat");
            var tickbar = this.cameraTickbarMap[tickbarName];
            
            var tickBreadthPc = 0.5;
            var tickDepthPc = (tickbarName == "lon" ? 7 : 4);
            
            if(shouldCreateTickbars) {
                tickbar = {
                    tickCount: (tickbarName == "lon" ? 64 : 32),
                    tickList: [],
                    prevCamAngRad: 0/0,
                };
                this.cameraTickbarMap[tickbarName] = tickbar;
                
                for(var tickI = 0; tickI < tickbar.tickCount; tickI++) {
                    var tickAngRad = Math.PI*2 / tickbar.tickCount * tickI; // four means 0, 90, 180, 270
                    
                    var rect = this.svgContext.rect(0, 0);
                    rect.attr({
                        width : (tickbarName == "lon" ? tickBreadthPc : tickDepthPc  )+"%",
                        height: (tickbarName == "lon" ? tickDepthPc   : tickBreadthPc)+"%",
                        
                        fill: (
                                tickI == Math.floor(tickbar.tickCount/2) ? 'blue'
                            :   tickI == 0 ? 'red'
                            :   'white'
                        ),
                        'fill-opacity': 0.5,
                        stroke: 'black', 'stroke-width': 1,
                    });
                    
                    tickbar.tickList[tickI] = {
                        angRad: tickAngRad,
                        rect: rect,
                    };
                }
            }
            
            var shouldUpdate = false;
            
            var camAngRad = (tickbarName == "lon" ? this.pr.camLonRad : -this.pr.camLatRad);
            
            if(camAngRad != tickbar.prevCamAngRad) {
                shouldUpdate = true;
                tickbar.prevCamAngRad = camAngRad;
            }
            
            if(shouldUpdate) {
                for(var tickI = 0; tickI < tickbar.tickCount; tickI++) {
                    
                    /* #### [2014-09-30] Written by Declan */
                    
                    //Get tick from array.
                    var tick = tickbar.tickList[tickI];
                    
                    // 0 (left) to 2*PI (right)
                    var tickAngRad = (-tick.angRad - camAngRad).mod(2*Math.PI);
                    
                    // 0 (left) to 1 (right)
                    var tickFrac = tickAngRad/(Math.PI*2);
                    
                    // -1 (left) to 1 (right)
                    var tickOffset = (tickFrac-0.5)*2;
                    
                    // 0 (middle) to 1 (left or right, depending on sign of tickOffset)
                    var absTickOffset = Math.abs(tickOffset);
                    
                    // [LOGARITHMICALLY SCALED] 0 (middle) to 1 (left or right, depending on sign of tickOffset)
                    var absLogOffset = Math.pow(absTickOffset, this.cameraTickExpPower);
                    
                    // [LOGARITHMICALLY SCALED] 0 (left) to 1 (right)
                    // (Uses sign of screenFrac to establish if the click was left or right of centre)
                    // (And then adds the logarithmically scaled distance respectively)
                    var logFrac = 0.5+(tickOffset < 0 ? -absLogOffset : absLogOffset)/2;
                    
                    // [LOGARITHMICALLY SCALED] 0 (left) to 1 (right)
                    var screenFrac = logFrac;
                    
                    var breadth = ((0.2+0.8*(1-absLogOffset))*tickBreadthPc);
                    var depth   = ((0.2+0.8*(1-absLogOffset))*tickDepthPc  );
                    
                    //Update each tick position
                    tick.rect.attr({
                        x: (tickbarName == "lon" ? (screenFrac*100) : 0                 )+"%",
                        y: (tickbarName == "lon" ? 100-depth        : screenFrac*(100-7))+"%",
                        
                        width : (tickbarName == "lon" ? breadth : depth  )+"%",
                        height: (tickbarName == "lon" ? depth   : breadth)+"%",
                    });
                    
                    /* #### [2014-09-30] End written by Declan */
                }
            }
            
        }
        
    };
    
    
    App.onViewportEventStart = function(ev, eventType) {
        this.mouseCurrentPos = { 
            x: (eventType == "mouse" ? ev.pageX : ev.originalEvent.touches[0].pageX), 
            y: (eventType == "mouse" ? ev.pageY : ev.originalEvent.touches[0].pageY)
        };
        //console.log(screen.width, screen.height);
        //console.log(ev.pageY);
        if(ev.pageY > (0.93 * $("#rootdiv").innerHeight())) {
            
            /* #### [2014-09-30] Written by Declan */
            
            // 0 (leftmost) to 1 (rightmost)
            var mouseFrac = this.mouseCurrentPos.x / $(this.svgContext.node).innerWidth();
            
            // -1 (leftmost) to 1 (rightmost)
            var mouseOffset = (mouseFrac-0.5)*2;
            
            // 0 (middle) to 1 (left or right, depending on sign of mouseOffset)
            var absMouseOffset = Math.abs(mouseOffset);
            
            // [LOGARITHMICALLY SCALED] 0 (middle) to 1 (left or right, depending on sign of mouseOffset)
            var absLogOffset = Math.exp(Math.log(absMouseOffset)/this.cameraTickExpPower);
            
            // [LOGARITHMICALLY SCALED] -1 (leftmost) to 1 (rightmost)
            var logOffset = (mouseOffset < 0 ? -absLogOffset : absLogOffset);
            
            // Camera angle difference to animate by
            // (haha, that variable name: the delta of the camera latitude angle in radians)
            var camLonAngRadDelta = logOffset * Math.PI;
            
            var camLonAngRadCurr = this.pr.camLonRad;
            var camLonAngRadNew = (camLonAngRadCurr + camLonAngRadDelta).mod(2*Math.PI);
            
            var camLonAngRadShortestDiff = this.pr.getShortAngleDiff(camLonAngRadNew, camLonAngRadCurr);
            
            //console.log("mouseFrac: ", mouseFrac);
            //console.log("mouseOffset: ", mouseOffset);
            //console.log("absMouseOffset: ", absMouseOffset);
            //console.log("absLogOffset: ", absLogOffset);
            //console.log("logOffset: ", logOffset);
            //console.log("camLonAngRadDelta: ", camLonAngRadDelta);
            //console.log("camLonAngRadCurr: ", camLonAngRadCurr);
            //console.log("camLonAngRadNew: ", camLonAngRadCurr+camLonAngRadShortestDiff);
            //console.log("camLonAngRadShortestDiff: ", camLonAngRadShortestDiff);
            
            var self = this;
            App.animate(500, camLonAngRadCurr, camLonAngRadCurr+camLonAngRadDelta, function(camLonAngRad) {
                //console.log(camLonAngRad);
                self.pr.panPanorama(camLonAngRad);
            });
            /* #### [2014-09-30] End written by Declan */
            
            return;
        }
        
        if(ev.pageX < (0.04 * $("#rootdiv").innerWidth())) { // TODO: Find a way to merge this with the above code for deduplication.
            
            /* #### [2014-09-30] Written by Declan */
            
            // 0 (leftmost) to 1 (rightmost)
            var mouseFrac = this.mouseCurrentPos.y / ($(this.svgContext.node).innerHeight()*0.93);
            
            // -1 (leftmost) to 1 (rightmost)
            var mouseOffset = (mouseFrac-0.5)*2;
            
            // 0 (middle) to 1 (left or right, depending on sign of mouseOffset)
            var absMouseOffset = Math.abs(mouseOffset);
            
            // [LOGARITHMICALLY SCALED] 0 (middle) to 1 (left or right, depending on sign of mouseOffset)
            var absLogOffset = Math.exp(Math.log(absMouseOffset)/this.cameraTickExpPower);
            
            // [LOGARITHMICALLY SCALED] -1 (leftmost) to 1 (rightmost)
            var logOffset = (mouseOffset < 0 ? -absLogOffset : absLogOffset);
            
            // Camera angle difference to animate by
            // (haha, that variable name: the delta of the camera latitude angle in radians)
            var camLatAngRadDelta = - logOffset * Math.PI;
            
            var camLatAngRadCurr = this.pr.camLatRad;
            var camLatAngRadNew = (camLatAngRadCurr + camLatAngRadDelta).mod(2*Math.PI);
            
            var camLatAngRadShortestDiff = this.pr.getShortAngleDiff(camLatAngRadNew, camLatAngRadCurr);
            
            //console.log("mouseFrac: ", mouseFrac);
            //console.log("mouseOffset: ", mouseOffset);
            //console.log("absMouseOffset: ", absMouseOffset);
            //console.log("absLogOffset: ", absLogOffset);
            //console.log("logOffset: ", logOffset);
            //console.log("camLatAngRadDelta: ", camLatAngRadDelta);
            //console.log("camLatAngRadCurr: ", camLatAngRadCurr);
            //console.log("camLatAngRadNew: ", camLatAngRadCurr+camLatAngRadShortestDiff);
            //console.log("camLatAngRadShortestDiff: ", camLatAngRadShortestDiff);
            
            var self = this;
            App.animate(500, camLatAngRadCurr, camLatAngRadCurr+camLatAngRadDelta, function(camLatAngRad) {
                //console.log(camLatAngRad);
                self.pr.tiltPanorama(camLatAngRad);
            });
            /* #### [2014-09-30] End written by Declan */

            return;
        }
        
        if(!this.isDragging) {
            this.isDragging = true;
            this.dragMouseStartPos = { 
                x: (eventType == "mouse" ? ev.pageX : ev.originalEvent.touches[0].pageX), 
                y: (eventType == "mouse" ? ev.pageY : ev.originalEvent.touches[0].pageY)
            };
            
            this.maxDrawBoundaries = { x: 0, y: 0 };
            this.minDrawBoundaries = { x: 2000, y: 2000};
            
            //
            this.pr.screenPosToAngle(this.dragMouseStartPos.x, this.dragMouseStartPos.y);
            //
            
            switch (this.mode)
            {
                case this.MODE_MANUAL_CONTROL: {
                    console.log("Here");
                    this.pr.screenPosToAngle(this.dragMouseStartPos.x, this.dragMouseStartPos.y);
                    App.oldPos.x = this.dragMouseStartPos.x;
                    App.oldPos.y = this.dragMouseStartPos.y;
                }
                break;
                case this.MODE_DRAW_RECTANGLE: {
                    //console.log(this.dragMouseStartPos);
                    /*this.svgContext.clear();
                    this.draggingRect = this.svgContext.rect(0, 0);
                    this.draggingRect.attr({ 
                        x: this.dragMouseStartPos.x, y: this.dragMouseStartPos.y,
                        width: 0, height: 0,
                        fill: 'transparent',
                        stroke: 'white',
                        'stroke-width': 5,
                    });
                    console.log("draw rect");*/
                    App.startPoint.x = this.dragMouseStartPos.x;
                    App.startPoint.y = this.dragMouseStartPos.y;
                    
                    this.dragAngleStart = this.pr.screenPosToAngle(this.dragMouseStartPos.x, this.dragMouseStartPos.y);
                    
                    var lol = THREE.Math.degToRad(2);
                    this.dragQuadVertexList = [
                        this.pr.angleToWorldPos(this.dragAngleStart.y    , this.dragAngleStart.z    , 350), // tl
                        this.pr.angleToWorldPos(this.dragAngleStart.y+lol, this.dragAngleStart.z    , 350), // tr
                        this.pr.angleToWorldPos(this.dragAngleStart.y    , this.dragAngleStart.z+lol, 350), // bl
                        this.pr.angleToWorldPos(this.dragAngleStart.y+lol, this.dragAngleStart.z+lol, 350)  // br
                    ]
                    
                    // you probably want to move a lot of this logic into panorama.js later
                    // (so that webapp.js doesn't make any calls to THREE.js-specific stuff)
                    this.dragQuadMesh = this.pr.makeNiceQuadPolyThing(this.dragQuadVertexList);
                    
                }
                break;
                case this.MODE_DRAW_SQUIGGLE: {
                    /*console.log("SQUIGGLE!");
                    this.svgContext.clear();
                    this.freeDraw = this.svgContext.path();
                    this.pathStr = "M " + this.dragMouseStartPos.x + " " + this.dragMouseStartPos.y;*/
                    
                    var currentDragAngle = this.pr.screenPosToAngle(this.mouseCurrentPos.x, this.mouseCurrentPos.y);
                    
                    this.dragLineVertexList = [];
                    var count = 5000;
                    this.dragLineVertexListI = count-1;
                    for(var i = 0; i < count; i++) {
                        this.dragLineVertexList.push(this.pr.angleToWorldPos(currentDragAngle.y, currentDragAngle.z, 340));
                    }
                    
                    this.dragLineMesh = this.pr.makeNiceLineThing(this.dragLineVertexList);
                    
                }
                break;
                case this.MODE_MOVE_ACTUATOR: {
                    var mouseX = this.mouseCurrentPos.x / this.pr.rendererSize.x;
                    var mouseY = this.mouseCurrentPos.y / this.pr.rendererSize.y;

                    var ang = this.pr.screenPosToAngle(this.mouseCurrentPos.x, this.mouseCurrentPos.y);

                    if(this.wsIsConnected) {
                        var msg = {
                            type: "move_actuator",
                            yaw: ang.y,
                            pitch: ang.z
                        };
                        var msgStr = JSON.stringify(msg);
                        console.log("sending: "+msgStr);
                        this.ws.send(msgStr);
                    }
                }
                break;
            }
        }
    };
    
    App.onViewportEventMove = function(ev, eventType) {
        this.mouseCurrentPos = {
            x: (eventType == "mouse" ? ev.pageX : ev.originalEvent.touches[0].pageX), 
            y: (eventType == "mouse" ? ev.pageY : ev.originalEvent.touches[0].pageY)
        };
        if(this.isDragging) {

            var mouseDiffX2 = this.mouseCurrentPos.x-App.oldPos.x;
            var mouseDiffY2 = App.oldPos.y-this.mouseCurrentPos.y;
            
            App.oldPos.x = this.mouseCurrentPos.x;
            App.oldPos.y = this.mouseCurrentPos.y;
            
            switch (this.mode)
            {
                case this.MODE_MANUAL_CONTROL: {
                    //console.log(mouseDiffX2);
                    //console.log(mouseDiffY2);
                    this.pr.dragCamera(-mouseDiffX2, -mouseDiffY2);
                }
                break;
                case this.MODE_DRAW_RECTANGLE: {
                    
                    var currentDragAngle = this.pr.screenPosToAngle(this.mouseCurrentPos.x, this.mouseCurrentPos.y);
                    
                    this.dragQuadVertexList = [
                        this.pr.angleToWorldPos(this.dragAngleStart.y, this.dragAngleStart.z, 350), // tl
                        this.pr.angleToWorldPos(currentDragAngle.y   , this.dragAngleStart.z, 350), // tr
                        this.pr.angleToWorldPos(this.dragAngleStart.y, currentDragAngle.z   , 350), // bl
                        this.pr.angleToWorldPos(currentDragAngle.y   , currentDragAngle.z   , 350)  // br
                    ];
                    this.dragQuadMesh.geometry.vertices = this.dragQuadVertexList;
                    this.dragQuadMesh.geometry.verticesNeedUpdate = true;
                    this.dragQuadMesh.geometry.computeBoundingSphere();
                    
                    /*console.log("move");
                    var mouseDiffX = Math.abs(this.mouseCurrentPos.x-this.dragMouseStartPos.x);
                    var mouseDiffY = Math.abs(this.mouseCurrentPos.y-this.dragMouseStartPos.y);
                    //console.log(mouseDiffX, mouseDiffY);
                    
                    this.draggingRect.attr({
                        x: Math.min(this.dragMouseStartPos.x, this.mouseCurrentPos.x),
                        y: Math.min(this.dragMouseStartPos.y, this.mouseCurrentPos.y),
                        width: mouseDiffX, height: mouseDiffY
                    });*/
                }
                break;
                case this.MODE_DRAW_SQUIGGLE: {
                    /*this.pathStr += " L " + this.mouseCurrentPos.x + " " + this.mouseCurrentPos.y;
                    this.freeDraw.plot(this.pathStr);
                    this.freeDraw.attr({
                        fill: 'transparent',
                        stroke: 'white',
                        'stroke-width': 3,
                    })
                    if(this.maxDrawBoundaries.x < this.mouseCurrentPos.x) {
                        this.maxDrawBoundaries.x = this.mouseCurrentPos.x;
                    }
                    if(this.maxDrawBoundaries.y < this.mouseCurrentPos.y) {
                        this.maxDrawBoundaries.y = this.mouseCurrentPos.y;
                    }
                    if(this.minDrawBoundaries.x > this.mouseCurrentPos.x) {
                        this.minDrawBoundaries.x = this.mouseCurrentPos.x;
                    }
                    if(this.minDrawBoundaries.y > this.mouseCurrentPos.y) {
                        this.minDrawBoundaries.y = this.mouseCurrentPos.y;
                    }*/
                    
                    var currentDragAngle = this.pr.screenPosToAngle(this.mouseCurrentPos.x, this.mouseCurrentPos.y);
                    
                    console.log(this.dragLineVertexListI, this.dragLineVertexList.length);
                    this.dragLineVertexList[this.dragLineVertexListI] = this.pr.angleToWorldPos(currentDragAngle.y, currentDragAngle.z, 340);
                    this.dragLineVertexList[this.dragLineVertexListI+1] = this.dragLineVertexList[this.dragLineVertexListI];
                    this.dragLineVertexListI = (this.dragLineVertexListI-2).mod(this.dragLineVertexList.length);
                    
                    this.dragLineMesh.geometry.vertices = this.dragLineVertexList;
                    this.dragLineMesh.geometry.verticesNeedUpdate = true;
                    this.dragLineMesh.geometry.computeBoundingSphere();
                }
                break;
                case this.MODE_MOVE_ACTUATOR: {
                    
                }
                break;
            }
        } 
    };
    
    App.onViewportEventEnd = function(ev, eventType) {
        
        this.mouseCurrentPos = {
            x: (eventType == "mouse" ? ev.pageX : ev.originalEvent.touches[0].pageX), 
            y: (eventType == "mouse" ? ev.pageY : ev.originalEvent.touches[0].pageY)
        };
        
        if(this.isDragging) {
            this.isDragging = false;
            
            switch (this.mode)
            {
                case this.MODE_MANUAL_CONTROL: {
                    
                }
                break;
                case this.MODE_DRAW_RECTANGLE: {
                    console.log("mouseup");
                    
                    this.pr.removeNiceQuadPolyThing(this.dragQuadMesh);
                    
                    var currentDragAngle = this.pr.screenPosToAngle(this.mouseCurrentPos.x, this.mouseCurrentPos.y);
                    
                    this.pr.buildFancyLines(this.dragAngleStart.y, currentDragAngle.y, this.dragAngleStart.z, currentDragAngle.z);
                    
                    /*var readingData = {
                        lat: 40 + Math.random()*80, // centre of reading
                        lon: 40 + Math.random()*80, // centre of reading
                        latSize: 10+Math.random()*30,
                        lonSize: 10+Math.random()*30,
                        dataWidth: 60,
                        dataHeight: 40,
                    }
                    readingData.data = new Uint8Array(readingData.dataWidth * readingData.dataHeight);
                    
                    for(var i = 0; i < readingData.dataWidth * readingData.dataHeight; i++) {
                        readingData.data[i] = Math.floor(Math.random()*255);
                    }*/
                    
                    App.endPoint.x = this.mouseCurrentPos.x;
                    App.endPoint.y = this.mouseCurrentPos.y;
                }
                break;
                case this.MODE_DRAW_SQUIGGLE: {
                    /*this.squiggleRect = this.svgContext.rect(0, 0);
                    this.squiggleRect.attr({ 
                        x: this.minDrawBoundaries.x, y: this.minDrawBoundaries.y,
                        width: this.maxDrawBoundaries.x-this.minDrawBoundaries.x, height: this.maxDrawBoundaries.y-this.minDrawBoundaries.y,
                        fill: 'transparent',
                        stroke: 'white',
                        'stroke-width': 5,
                    });*/
                }
                break;
                case this.MODE_MOVE_ACTUATOR: {
                    
                }
                break;
            }
        } 
    };
    
    App.changeSidebarButtonPage = function(inPage) {
    
        // Empty the current content
        $("#sidebar-content").empty();
        
        // Set the current page to the inPage.
        this.currentSidebarPage = inPage;
        
    
        var $headerText = $("<div class='sidebar-header' align='center'></div>");
        $headerText.text(inPage.pageTitle);
        
        var $header = $("<span></span>");
        $header.append($headerText);
        var $headerli = $("<li class='sidebar-header-row'></li>");
        $headerli.append($header);
        $("#sidebar-content").append($headerli);
        
        this.currentSidebarPage.buttonList.forEach(function(info) {
            var $button = $("<button class='sidebar-element'></button>");
            $button.html(info.label);
            $button.click(info.cb);
            var $span = $("<span></span>");
            $span.append($button);
            var $li = $("<li></li>");
            $li.append($span);
            $("#sidebar-content").append($li);
        });
    };
    
    App.openSettings = function(inPage) {
        
        // Render panel (does nothing if already open)
        $(".popout-pane").fadeIn(200);
        
        // Set current settings page to the inPage.
        this.currentSettingsPage = inPage;
        
        // Empty existing header and body content
        $("#settings-header").empty();
        $("#settings-body").empty();
        
        // Set settings panel title
        $("#settings-header").append(App.currentSettingsPage.paneTitle);
        
        /*Iterates through content lists, wrapping them in appropriate containers
          and appending them to the settings panel's body area*/
        this.currentSettingsPage.content.forEach(function(info){
           var $settingDiv = $("<div class='settings-row'</div>");
           
           var $valueDiv = $("<div class='settings-value' contenteditable='true'></div>");
           $valueDiv.html(info.value);
           var $labelDiv = $("<div class='settings-label'></div>");
           $labelDiv.html(info.label);
           
           $settingDiv.append($labelDiv);
           $settingDiv.append($valueDiv);
    
           $("#settings-body").append($settingDiv);
       
        });
        
        //Initalises all sliders the same atm, will need to be added to above loop at some point to load dynamically
        $(".settings-sliders-regular").noUiSlider({
            start: 30,
            connect: "lower",
            orientation: "horizontal",
            step: 10,
            range: {
                'min': 0,
                'max': 100 },
        });
        
        $(".settings-sliders-toggle").noUiSlider({
           start: 0,
           connect: "lower",
           orientation: "horizontal",
           range: {
               'min': [0,1],
               'max': 1 },
        });
        
        $(".settings-sliders-range").noUiSlider({
            start: [10, 40],
            connect: true,
            orientation: "horizontal",
            step: 5,
            range: {
                'min': [0],
                'max': [50] }
        });
        
    };
    
    /* App.animate
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-30 DWW] Created.
    */
    App.animate = function(duration, min, max, stepFunc) {
        // This is the worst hack.... jQuery, why?
        var $aaahahahahahahaha = $('<div/>');
        $aaahahahahahahaha.css({ top: min });
        $aaahahahahahahaha.animate(
            { top: max },
            {
                step: function(youreSerious) {
                    stepFunc(youreSerious); // https://www.youtube.com/watch?v=ztVMib1T4T4
                }
            }
        );
    };
    
    $("#close-popout-button").click(function(){
        $(".popout-pane").fadeOut(200);
    });
    
    
    $(document).ready(function() { App.onDocumentReady(); });


    // http://jsperf.com/json-vs-base64
    App.base64String = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    App.base64DecodeLookup = function() {
      var coreArrayBuffer = new ArrayBuffer( 256 );
      var base64DecodeLookupTable = new Uint8Array( coreArrayBuffer );
      for( var i = 0; i < App.base64String.length; i ++ ) {
        base64DecodeLookupTable[ App.base64String[ i ].charCodeAt( 0 ) ] = i;
      }
      return base64DecodeLookupTable;
    }();
    App.convertB64ToBuffer = function() {
      var base64DecodeLookup = App.base64DecodeLookup;
      return function(base64) {
        var bufferLength = base64.length * 0.75,
          len = base64.length, i, p = 0,
          encoded1, encoded2, encoded3, encoded4;
    
        if (base64[base64.length - 1] === "=") {
          bufferLength--;
          if (base64[base64.length - 2] === "=") {
            bufferLength--;
          }
        }
        var arraybuffer = new ArrayBuffer(bufferLength),
        bytes = new Uint8Array(arraybuffer);
        for (i = 0; i < len; i+=4) {
          encoded1 = base64DecodeLookup[base64.charCodeAt(i)];
          encoded2 = base64DecodeLookup[base64.charCodeAt(i+1)];
          encoded3 = base64DecodeLookup[base64.charCodeAt(i+2)];
          encoded4 = base64DecodeLookup[base64.charCodeAt(i+3)];
    
          bytes[p++] = (encoded1 << 2) | (encoded2 >> 4);
          bytes[p++] = ((encoded2 & 15) << 4) | (encoded3 >> 2);
          bytes[p++] = ((encoded3 & 3) << 6) | (encoded4 & 63);
        }
        return arraybuffer;
      };
      console.log("wadlfhasjkdhfalsdf");
    }();

});
