JS.require('JS.Class', 'JS.Observable', function(Class) {
  SensorDataStructure = new Class({
        initialize: function() {
            this.$panoramaCanvas = $('<canvas></canvas>');
            this.panoramaCanvasElement = this.$panoramaCanvas.get()[0];
            this.panoramaCanvasElement.width = 4096;
            this.panoramaCanvasElement.height = 2048;
            this.panoramaContext = this.panoramaCanvasElement.getContext("2d");
            
            
            var boundingBox = {x1: 0, x2: 360, y1: 0, y2: 180};
            this.quadTree = new QuadTree(boundingBox, 0);
            
            // Testing quad tree:
            for (var lon = 0; lon < 360; lon += 16) {
                for (var lat = 0; lat < 180; lat += 4) {
                    // console.log("Lon: " + lon + " Lat: " + lat);
                    var temp = Math.floor((Math.random() * 45) + 1); 

                    var tempBox = {x1: lon, x2: lon+16, y1: lat, y2: lat+4};
                    var reading = {boundingBox: tempBox, tag: index, temperature: temp};
                    
                    this.quadTree.insert(reading);
                }
              
            }

            var array = this.retrieveValuesInWindow(boundingBox);
            for (var index = 0; index < array.length; index++) {
                // console.log(array[index].boundingBox.x1 + " " + array[index].boundingBox.x2 + " " 
                            // + array[index].boundingBox.y1 + " " + array[index].boundingBox.y2);
                //console.log("Temperature: " + array[index].temperature);
            }
            //console.log("Total number of readings: " + array.length);
        },
        onNewReading: function(reading) {
            this.quadTree.insert(reading);
            
        },
        retrieveValuesInWindow: function(boundingBox) {
            return this.quadTree.retrieve(boundingBox);
        }, 
        drawTextureFromSelection: function(lonA, lonB, latA, latB, tex) {
            lonA = lonA.mod(2*Math.PI);
            lonB = lonB.mod(2*Math.PI);
            latA = latA.mod(2*Math.PI);
            latB = latB.mod(2*Math.PI);
            
            var canvas = this.panoramaCanvasElement;
            
            var canvasWidth  = canvas.width;
            var canvasHeight = canvas.height;
            
            var canvasXA = lonA / (2*Math.PI) * canvasWidth;
            var canvasXB = lonB / (2*Math.PI) * canvasWidth;
            
            var canvasYA = (Math.PI-(latA+Math.PI/2).mod(2*Math.PI))/Math.PI * canvasHeight;
            var canvasYB = (Math.PI-(latB+Math.PI/2).mod(2*Math.PI))/Math.PI * canvasHeight;
            
            //    180 | 180
            //     90 |  90
            //      0 |   0
            //   - 90 | 270
            //   -180 | 180
            var lonDiff = this.getShortAngleDiff(lonB, lonA);
            var latDiff = this.getShortAngleDiff(latB, latA);
            var isLonSplit = (Math.abs(lonA-lonB) > Math.PI && lonDiff < Math.PI);
            
            var ctx = canvas.getContext("2d");
            
            var rectXMin = Math.min(canvasXA, canvasXB);
            var rectXMax = Math.max(canvasXA, canvasXB);
            var rectYMin = Math.min(canvasYA, canvasYB);
            var rectYMax = Math.max(canvasYA, canvasYB);
            var rectWidth = Math.abs(canvasXB-canvasXA);
            var rectHeight = Math.abs(canvasYB-canvasYA);
            
            var imageData = ctx.createImageData(rectWidth, rectHeight);
            var pos = 0;
            
            console.log("AAA: "+rectXMin+", "+rectYMin);
            console.log("BBB: "+rectWidth+", "+rectHeight);
            
            for (y = 0; y < Math.floor(rectHeight); y++) {
                for (x = 0; x < Math.floor(rectWidth); x++) {
                    // calculate sine based on distance
                    var x2 = Math.floor(rectXMin + x);
                    var y2 = Math.floor(rectYMin + y);
                    var t1 = Math.sin(x2/17.0);
                    var t2 = Math.sin(y2/19.0);

                    /*var r = Math.random() * 255;
                    var g = Math.random() * 255;
                    var b = Math.random() * 255;*/
                    // calculate RGB values based on sine
                    var r = x2.mod(256);
                    var g = y2.mod(256);
                    var b = (x2+y2).mod(256);

                    // set red, green, blue, and alpha:
                    imageData.data[pos++] = Math.max(0,Math.min(255, r));
                    imageData.data[pos++] = Math.max(0,Math.min(255, g));
                    imageData.data[pos++] = Math.max(0,Math.min(255, b));
                    imageData.data[pos++] = 255; // opaque alpha
                }
            }
            
            ctx.fillStyle = '#FFFFFF';
            if(isLonSplit) {
                //ctx.fillStyle = '#'+Math.floor(Math.random()*(Math.pow(2, 24)-1)).toString(16);
                ctx.putImageData(
                    imageData,
                    0, rectYMin
                );
                //ctx.fillStyle = '#'+Math.floor(Math.random()*(Math.pow(2, 24)-1)).toString(16);
                ctx.putImageData(
                    imageData,
                    rectXMax, rectYMin
                );
            } else {
                //ctx.fillStyle = '#'+Math.floor(Math.random()*(Math.pow(2, 24)-1)).toString(16);
                ctx.putImageData(
                    imageData,
                    rectXMin, rectYMin
                );
            }
            this.panoramaMaterial.map.needsUpdate = true;
        },
        getShortAngleDiff: function(aRad, bRad) {
            var diff = (aRad-bRad+Math.PI).mod(2*Math.PI)-Math.PI;
            
            return (diff < Math.PI ? diff : diff-Math.PI);
        },
  });
});