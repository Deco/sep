JS.require('JS.Class', 'JS.Observable', function(Class) {
    /*
    var SensorImageFeed = new Class({
        include: Observable,
        
        initialize: function(name) {
            this.name = name;
            this.image = ;
        },
    });
    */
    Number.prototype.mod = function(n) { return ((this%n)+n)%n; } // WTF JAVASCRIPT!?!?!?!?
    
    PanoramaRenderer = new Class({
        initialize: function(container) {
            this.buildRenderer(container);
            this.buildScene();
            this.buildCamera();
            this.buildPanoramaSphere();
            this.buildLatLonLines();
            this.projector = new THREE.Projector();
            
            var boundingBox = {x1: 0, x2: 360, y1: 0, y2: 180};
            //this.quadTree = new QuadTree(boundingBox, 0);


            //this.buildTestCube();
        },
        buildRenderer: function(container) {
            //this.targetCanvas = targetCanvas;
            this.container = container;
            
            // http://stackoverflow.com/a/16504494/837856
            var supportsWebGL = (function () {
                try {
                    return (
                            !!window.WebGLRenderingContext
                        &&  (
                                    !!document.createElement('canvas').getContext('experimental-webgl')
                                ||  !!document.createElement('canvas').getContext('webgl')
                            )
                    );
                } catch(e) {
                    return false;
                }
            })();
            var rendererClass = (
                    supportsWebGL ? THREE.WebGLRenderer // Use the device's GPU to render efficiently
                :                   THREE.CanvasRenderer
            );
            //console.log("WebGL? "+(!!document.createElement('canvas').getContext('webgl')));
            
            this.renderer = new rendererClass({
                //canvas: targetCanvas,
                antialias: true,
                // see for full list of parameters: http://threejs.org/docs/#Reference/Renderers/WebGLRenderer
            });
            this.rendererSize = new THREE.Vector2(container.innerWidth(), container.innerHeight());
            this.renderer.setSize(this.rendererSize.x, this.rendererSize.y);
            
            this.targetCanvas = this.renderer.domElement;
            container.append(this.targetCanvas);
            
            var self = this;
            $(window).resize(function(){ // DW: this should not be here
                console.log("blah");
                self.rendererSize = new THREE.Vector2(self.container.innerWidth(), self.container.innerHeight());
                self.renderer.setSize(self.rendererSize.x, self.rendererSize.y);
                if(self.camera) {
                    self.camera.aspect = self.container.innerWidth() / self.container.innerHeight();
                    self.camera.updateProjectionMatrix();
                }
            });
            
            this.wtfimage = new Image();
            this.wtfimage.src = "equirectangular_test.jpg";
            this.wtfkeyboard = new THREEx.KeyboardState();
        },
        buildScene: function() {
            this.scene = new THREE.Scene(); // holds the 3d environment (the sphere)
        },
        buildCamera: function() {
            this.camera = new THREE.PerspectiveCamera(
                75, // field of view
                //targetCanvas.innerWidth / targetCanvas.innerHeight, // aspect ratio
                this.container.innerWidth() / this.container.innerHeight(), // aspect ratio
                1, 5000 // near and far plane distances
            );
            this.camera.target = new THREE.Vector3(0, 0, 0);
            this.camLatRad = 0;
            this.camLonRad = 0;//-90;
        },
        buildPanoramaSphere: function() {
            var sphereGeometry = new THREE.SphereGeometry( // a sphere to apply the equirectangular texture to
                500, // radius
                60, 40 // number of width and height segments
            );
            
            sphereGeometry.applyMatrix(new THREE.Matrix4().makeScale(-1, 1, 1)); // invert it, so the polygons are facing inwards
            

            this.$panoramaCanvas = $('<canvas></canvas>');
            this.panoramaCanvasElement = this.$panoramaCanvas.get()[0];
            this.panoramaCanvasElement.width = 4096;
            this.panoramaCanvasElement.height = 2048;
            this.panoramaContext = this.panoramaCanvasElement.getContext("2d");
            /*
            var ctx = canvas.getContext("2d");
            var textHeight = 256;
            ctx.font = "normal " + textHeight + "px Arial";
            var metrics = ctx.measureText("Sample Text ");
            var textWidth = metrics.width;
            
            ctx.fillStyle = "gray";
            ctx.fillRect(0, 0, canvas.width, canvas.height);
            
            ctx.font = "normal " + textHeight + "px Arial";
            ctx.textAlign = "center";
            ctx.textBaseline = "middle";
            ctx.fillStyle = "#ff0000";
            ctx.fillText("Sample Text ", textWidth / 2, textHeight);
            */
            
            var tex = new THREE.Texture(this.$panoramaCanvas.get()[0]);
            //tex.wrapT = THREE.ClampToEdgeWrapping;
            //tex.wrapS = THREE.ClampToEdgeWrapping;
            
            var self = this;
            this.eqtex = THREE.ImageUtils.loadTexture('equirectangular_test.jpg', new THREE.UVMapping(), function() {
                //self.panoramaMaterial.map = self.eqtex;
                
                var imageObj = self.eqtex.image;
                
                //self.panoramaContext.drawImage(imageObj, 0, 0);
                
                self.panoramaMaterial.map.needsUpdate = true;
                
            });
            
            this.panoramaMaterial = new THREE.MeshBasicMaterial({ // equirectangular texture
                // image sourced from http://www.flickr.com/photos/jonragnarsson/2294472375/
                //wireframe: true, color: 'blue',
                //map: THREE.ImageUtils.loadTexture('equirectangular_test.jpg'),
                map: tex,
                useScreenCoordinates: false,
            });
            this.panoramaMaterial.map.needsUpdate = true;
            
            /*var testImage = new Image();
            testImage.src = 'equirectangular_test.jpg';
            var self = this;
            testImage.onload = function(){
                if(self.$panoramaCanvas && self.$panoramaCanvas.getContext) {
                    var ctx = self.$panoramaCanvas.getContext('2d');
                    ctx.drawImage(testImage, 0, 0);
                    self.panoramaMaterial.map.needsUpdate = true;
                }
            };*/
            
            this.sphereMesh = new THREE.Mesh(sphereGeometry, this.panoramaMaterial);
            this.scene.add(this.sphereMesh);

            this.$samscanvas = $("<canvas>");

            this.$samscanvas.appendTo("body");
        },

        drawReading: function(readingData) {
        
            var canvas = this.panoramaCanvasElement;
            var context = canvas.getContext("2d");

            var imageData = context.createImageData(readingData.dataWidth, readingData.dataHeight);
            imageData.data.set(readingData.data);

            /*var pos = 0; // index position into imagedata array
            var xoff = imgWidth / (100 / 3); // offsets to "center"
            var yoff = imgHeight / 3;

            for(var y = 0; y < imgHeight; y++) {
                for(var x = 0; x < imgWidth; x++) {
                    // calculate sine based on distance
                    var x2 = x - xoff;
                    var y2 = y - yoff;
                    var d = Math.sqrt(x2 * x2 + y2 * y2);
                    var t = Math.sin(d / 6.0);

                    // calculate RGB values based on sine
                    var r = t * 200;
                    var g = 125 + t * 80;
                    var b = 235 + t * 20;

                    // set red, green, blue, and alpha:
                    imageData.data[pos++] = Math.max(0, Math.min(255, r));
                    imageData.data[pos++] = Math.max(0, Math.min(255, g));
                    imageData.data[pos++] = Math.max(0, Math.min(255, b));
                    imageData.data[pos++] = 255; // opaque alpha
                }
            }*/

            this.drawRectTexture(
                readingData.lon - readingData.lonSize/2,
                readingData.lon + readingData.lonSize/2,
                readingData.lat - readingData.latSize/2,
                readingData.lat + readingData.latSize/2,
                imageData
            );
        },
        
        drawRectTexture: function(lonA, lonB, latA, latB, data) {

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
            
            var lonDiff = this.getShortAngleDiff(lonB, lonA);
            var canvasLonDiff = lonDiff / (2*Math.PI) * canvasWidth;

            var isLonSplit = (Math.abs(lonA-lonB) > Math.PI && lonDiff < Math.PI);
            
            var ctx = canvas.getContext("2d");
            
            var rectXMin = Math.min(canvasXA, canvasXB);
            var rectXMax = Math.max(canvasXA, canvasXB);
            var rectYMin = Math.min(canvasYA, canvasYB);
            var rectYMax = Math.max(canvasYA, canvasYB);
            var rectWidth = Math.abs(canvasXB-canvasXA);
            var rectHeight = Math.abs(canvasYB-canvasYA);

            var sc = this.$samscanvas[0];
            var scc = sc.getContext("2d");
            scc.canvas.width = data.width;
            scc.canvas.height = data.height;
            scc.putImageData(data, 0, 0);

            if(isLonSplit) {
                ctx.fillStyle = '#FFFFFF';
                var w = (canvasWidth-rectXMax)+rectXMin;
                var frac = (canvasWidth-rectXMax)/w;
                ctx.drawImage(
                    sc,
                    0, 0, frac*data.width, data.height,
                    rectXMax, rectYMin,
                    canvasWidth-rectXMax, rectHeight
                );
                ctx.drawImage(
                    sc,
                    frac*data.width, 0, (1-frac)*data.width, data.height,
                    0, rectYMin,
                    rectXMin, rectHeight
                );
            } else {
                ctx.fillStyle = '#FFFFFF';
                ctx.drawImage(
                    sc,
                    0, 0, data.width, data.height,
                    rectXMin, rectYMin,
                    rectWidth, rectHeight
                );
                console.log("blblablbalbal: "+rectXMin+", "+rectYMin+"; "+rectWidth+", "+rectHeight+"; "+data.width+", "+data.height);
            }

            /*ctx.fillStyle = '#FFFFFF';
            if(isLonSplit) {
                //ctx.fillStyle = '#'+Math.floor(Math.random()*(Math.pow(2, 24)-1)).toString(16);
                ctx.putImageData(
                    data,
                    0, rectYMin
                );
                //ctx.fillStyle = '#'+Math.floor(Math.random()*(Math.pow(2, 24)-1)).toString(16);
                ctx.putImageData(
                    data,
                    rectXMax, rectYMin
                );
            } else {
                //ctx.fillStyle = '#'+Math.floor(Math.random()*(Math.pow(2, 24)-1)).toString(16);
                ctx.putImageData(
                    data,
                    rectXMin, rectYMin
                );
            }*/
            this.panoramaMaterial.map.needsUpdate = true;
        },
        drawRect: function(lonA, lonB, latA, latB) {
            
            var lonStart = lonA;
            var latStart = latA;
            var lonDiff = this.getShortAngleDiff(lonB, lonA);
            var latDiff = this.getShortAngleDiff(latB, latA);
            
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
            
            var isLonSplit = (Math.abs(lonA-lonB) > Math.PI && lonDiff < Math.PI);
            
            var ctx = canvas.getContext("2d");
            
            var rectXMin = Math.min(canvasXA, canvasXB);
            var rectXMax = Math.max(canvasXA, canvasXB);
            var rectYMin = Math.min(canvasYA, canvasYB);
            var rectYMax = Math.max(canvasYA, canvasYB);
            var rectWidth = Math.abs(canvasXB-canvasXA);
            var rectHeight = Math.abs(canvasYB-canvasYA);
            
            if(isLonSplit) {
                ctx.fillStyle = '#'+Math.floor(Math.random()*(Math.pow(2, 24)-1)).toString(16);
                ctx.drawImage(
                    this.eqtex.image,
                    0, rectYMin,
                    rectXMin, rectHeight,
                    0, rectYMin,
                    rectXMin, rectHeight
                );
                ctx.fillStyle = '#'+Math.floor(Math.random()*(Math.pow(2, 24)-1)).toString(16);
                ctx.drawImage(
                    this.eqtex.image,
                    rectXMax, rectYMin,
                    canvasWidth-rectXMax, rectHeight,
                    rectXMax, rectYMin,
                    canvasWidth-rectXMax, rectHeight
                );
            } else {
                ctx.fillStyle = '#'+Math.floor(Math.random()*(Math.pow(2, 24)-1)).toString(16);
                ctx.drawImage(
                    this.eqtex.image,
                    rectXMin, rectYMin,
                    rectWidth, rectHeight,
                    rectXMin, rectYMin,
                    rectWidth, rectHeight
                );
            }
            this.panoramaMaterial.map.needsUpdate = true;
        },
        
        buildLatLonLines: function() {
            var lineMaterial = new THREE.LineBasicMaterial({
                color: 0x00ff00,
            });
            
            var sphereRadius = 500;
            var lineRadius = sphereRadius*0.95;
            var latCount = 4, lonCount = latCount*2;
            var segmentsPerHemisphere = 30;
            
            var lonI, latI, lon, lat, phi, theta;
            var lineGeometry, line;
            for(latI = 1; latI < latCount; latI++) {
                lat = -90+180*latI/latCount;
                
                lineGeometry = new THREE.Geometry();
                for(lonI = 0; lonI < segmentsPerHemisphere*2; lonI++) {
                    lon = 0+360*lonI/(segmentsPerHemisphere*2-1);
                    
                    phi = THREE.Math.degToRad(90-lat);
                    theta = THREE.Math.degToRad(lon);
                    
                    lineGeometry.vertices.push(new THREE.Vector3(
                        lineRadius * Math.sin(phi) * Math.cos(theta),
                        lineRadius * Math.cos(phi),
                        lineRadius * Math.sin(phi) * Math.sin(theta)
                    ));
                    //console.log(phi, theta)
                }
                line = new THREE.Line(lineGeometry, lineMaterial);
                this.scene.add(line);
            }
            
            for(lonI = 0; lonI < lonCount; lonI++) {
                lon = 0+360*lonI/lonCount;
                
                lineGeometry = new THREE.Geometry();
                for(latI = 0; latI < segmentsPerHemisphere; latI++) {
                    lat = -90+180*latI/(segmentsPerHemisphere-1);
                    
                    phi = THREE.Math.degToRad(90-lat);
                    theta = THREE.Math.degToRad(lon);
                    
                    lineGeometry.vertices.push(new THREE.Vector3(
                        lineRadius * Math.sin(phi) * Math.cos(theta),
                        lineRadius * Math.cos(phi),
                        lineRadius * Math.sin(phi) * Math.sin(theta)
                    ));
                }
                line = new THREE.Line(lineGeometry, lineMaterial);
                this.scene.add(line);
            }
        },
        getShortAngleDiff: function(aRad, bRad) {
            var diff = (aRad-bRad+Math.PI).mod(2*Math.PI)-Math.PI;
            
            return (diff < Math.PI ? diff : diff-Math.PI);
        },
        buildFancyLines: function(lonA, lonB, latA, latB) {
            this.drawRect(lonA, lonB, latA, latB);
            //var byteArray = [];
            //this.drawRectTexture(lonA, lonB, latA, latB, byteArray);
            //return; // comment me to get fancy lines
            
            var lineMaterial = new THREE.LineBasicMaterial({
                color: 0xff0000,
            });
            
            var lonStart = lonA;
            var latStart = latA;
            var lonDiff = this.getShortAngleDiff(lonB, lonA);
            var latDiff = this.getShortAngleDiff(latB, latA);
            
            var sphereRadius = 500;
            var lineRadius = sphereRadius*0.85;
            var degreesPerInterval = 5;
            var lonCount = 2;//Math.max(2, Math.ceil(Math.abs(lonDiff)/degreesPerInterval));
            var latCount = 2;//Math.max(2, Math.ceil(Math.abs(latDiff)/degreesPerInterval));
            var segmentCount = 30;
            
            lonA = lonA.mod(2*Math.PI);
            lonB = lonB.mod(2*Math.PI);
            latA = latA.mod(2*Math.PI);
            latB = latB.mod(2*Math.PI);
            
            // 270   000   090   180   270   360   090
            //  |     X     |     |     |     X     |
            //              A-----B
            //              B-----A
            //                          A-----------B
            //                          B-----------A
            
            var latI, lonI, lat, lon, phi, theta;
            var lineGeometry, line;
            for(lonI = 0; lonI < lonCount; lonI++) {
                lon = lonStart+lonDiff*lonI/(lonCount-1);
                
                lineGeometry = new THREE.Geometry();
                for(latI = 0; latI < segmentCount*2; latI++) {
                    lat = latStart+latDiff*latI/(segmentCount*2-1);
                    
                    phi = THREE.Math.degToRad(90)-lat;
                    theta = lon;
                    
                    lineGeometry.vertices.push(new THREE.Vector3(
                        lineRadius * Math.sin(phi) * Math.cos(theta),
                        lineRadius * Math.cos(phi),
                        lineRadius * Math.sin(phi) * Math.sin(theta)
                    ));
                    //console.log(phi, theta)
                }
                line = new THREE.Line(lineGeometry, lineMaterial);
                this.scene.add(line);
            }
            
            for(latI = 0; latI < latCount; latI++) {
                lat = latStart+latDiff*latI/(latCount-1);
                
                lineGeometry = new THREE.Geometry();
                for(lonI = 0; lonI < segmentCount; lonI++) {
                    lon = lonStart+lonDiff*lonI/(segmentCount-1);
                    
                    phi = THREE.Math.degToRad(90)-lat;
                    theta = lon;
                    
                    lineGeometry.vertices.push(new THREE.Vector3(
                        lineRadius * Math.sin(phi) * Math.cos(theta),
                        lineRadius * Math.cos(phi),
                        lineRadius * Math.sin(phi) * Math.sin(theta)
                    ));
                }
                line = new THREE.Line(lineGeometry, lineMaterial);
                this.scene.add(line);
            }
        },
        buildTestCube: function() {
            var geometry = new THREE.BoxGeometry(1,1,1);
            geometry.applyMatrix(new THREE.Matrix4().makeScale(1, 1, 1));
            
            var tex = new THREE.Texture(this.$panoramaCanvas.get()[0]);
            
            var material = new THREE.MeshBasicMaterial({ // equirectangular texture
                // image sourced from http://www.flickr.com/photos/jonragnarsson/2294472375/
                //wireframe: true, color: 'blue',
                //map: THREE.ImageUtils.loadTexture('equirectangular_test.jpg'),
                map: tex,
            });
            material.map.needsUpdate = true;
            
            //var material = new THREE.MeshPhongMaterial({ambient: 0x034303, color: 0xddffdd, specular: 0x009900, shininess: 30, shading: THREE.FlatShading})
            this.cube = new THREE.Mesh( geometry, material );
            //this.cube.rotation.y += 50;
            //this.cube.rotation.z += 50;
            this.cube.position.z -= 5;
            this.scene.add(this.cube);
            
            var directionalLight = new THREE.DirectionalLight(0xffffff);
            directionalLight.position.set(1, 1, 1).normalize();
            this.scene.add(directionalLight);
            var ambientLight = new THREE.AmbientLight(0x222222);
            this.scene.add(ambientLight);
            
            //Test draw rectangle
            /*var squareGeometry = new THREE.Geometry();
                 squareGeometry.vertices.push(new THREE.Vector3(-1.0,  1.0, 0.0));
                 squareGeometry.vertices.push(new THREE.Vector3( 1.0,  1.0, 0.0));
                 squareGeometry.vertices.push(new THREE.Vector3( 1.0, -1.0, 0.0));
                 squareGeometry.vertices.push(new THREE.Vector3(-1.0, -1.0, 0.0));
                 squareGeometry.faces.push(new THREE.Face4(0, 1, 2, 3));
 
                 // Create a white basic material and activate the 'doubleSided' attribute.
                 var squareMaterial = new THREE.MeshBasicMaterial({
                     color:0xFFFFFF,
                     side:THREE.DoubleSide
                 });

                 var squareMesh = new THREE.Mesh(squareGeometry, squareMaterial);
                 squareMesh.position.set(1.5, 0.0, -5);
                 this.scene.add(squareMesh);*/
        },
        dragCamera: function(moveX, moveY) {
            this.camLonRad += (moveX/10)/180*Math.PI;
            this.camLatRad += (moveY/10)/180*Math.PI;
        },
        panPanorama: function(newcamLonRad) { // For the love of god, rename this functon!
            this.camLonRad = newcamLonRad;
        },
        tiltPanorama: function(newcamLatRad) {
            this.camLatRad = newcamLatRad;
        },
        update: function(currTime, deltaTime) {
			//this.camLonRad += 0.01*deltaTime;
			this.camLatRad = Math.max(-Math.PI/2, Math.min(Math.PI/2-0.01, this.camLatRad));
			//this.camLonRad = Math.max(-150, Math.min(150, this.camLonRad));
            
            if(this.cube) {
                this.cube.rotation.y += 0.001*deltaTime;
            }
            
            //this.panoramaMaterial.map.needsUpdate = true;
            this.render();
            //console.log(this.camLonRad);
            //console.log(this.camLatRad);
            if(this.wtfkeyboard.pressed("left"            )) { this.camLonRad -= 0.08/180*Math.PI*deltaTime; }
            if(this.wtfkeyboard.pressed("right"           )) { this.camLonRad += 0.08/180*Math.PI*deltaTime; }
            if(this.wtfkeyboard.pressed("up"              )) { this.camLatRad += 0.08/180*Math.PI*deltaTime; }
            if(this.wtfkeyboard.pressed("down"            )) { this.camLatRad -= 0.08/180*Math.PI*deltaTime; }
            if(this.wtfkeyboard.pressed("pagedown"        )) { this.camera.fov -= 0.05*deltaTime; this.camera.updateProjectionMatrix(); }
            if(this.wtfkeyboard.pressed("pageup"          )) { this.camera.fov += 0.05*deltaTime; this.camera.updateProjectionMatrix(); }
            
            var theta = this.camera.rotation.y;
            var bbbbb = theta-Math.PI/2;
            var speed = 2.5;
            if(this.wtfkeyboard.pressed("w"               )) { this.camera.position.add((new THREE.Vector3( 0, 0, -1)).applyEuler(this.camera.rotation).multiplyScalar(speed)); }
            if(this.wtfkeyboard.pressed("s"               )) { this.camera.position.add((new THREE.Vector3( 0, 0,  1)).applyEuler(this.camera.rotation).multiplyScalar(speed)); }
            if(this.wtfkeyboard.pressed("a"               )) { this.camera.position.add((new THREE.Vector3(-1, 0,  0)).applyEuler(this.camera.rotation).multiplyScalar(speed)); }
            if(this.wtfkeyboard.pressed("d"               )) { this.camera.position.add((new THREE.Vector3( 1, 0,  0)).applyEuler(this.camera.rotation).multiplyScalar(speed)); }
            if(this.wtfkeyboard.pressed("space"           )) { this.camera.position.y += 1*deltaTime; }
            if(this.wtfkeyboard.pressed("ctrl"            )) { this.camera.position.y -= 1*deltaTime; }
            if(this.wtfkeyboard.pressed("r"               )) { this.camera.position.set(0, 0, 0); }
            //console.log("camLatRad: "+this.camLatRad);
            //console.log("camLonRad: "+this.camLonRad);
        },
        render: function() {
            // adapted from http://mrdoob.github.io/three.js/examples/webgl_panorama_equirectangular.html
			var phi = Math.PI/2-this.camLatRad;
			var theta = this.camLonRad;
			
			this.camera.target.x = 500 * Math.sin(phi) * Math.cos(theta);
			this.camera.target.y = 500 * Math.cos(phi);
			this.camera.target.z = 500 * Math.sin(phi) * Math.sin(theta);
			
			this.camera.lookAt(this.camera.target);
			
            this.renderer.render(this.scene, this.camera);
        },
        updatePanoramaMaterial: function(events) {
            // 
        },
        angleToWorldPos: function(latRad, lonRad, dist) {
			var phi = Math.PI/2-lonRad;
			var theta = latRad;
            return new THREE.Vector3(
			    dist * Math.sin(phi) * Math.cos(theta),
			    dist * Math.cos(phi),
			    dist * Math.sin(phi) * Math.sin(theta)
		    );
        },
        screenPosToAngle: function(x, y) {
            
            // x and y range between -1 and 1 (screen coordinates)
            var screenVector = new THREE.Vector3(
                 x/this.rendererSize.x*2 - 1,
                -y/this.rendererSize.y*2 + 1,
                0.5
            );
            
            // the point at worldVector lies on a rectangle 0.5 units away from the camera
            // (imagine this is the rectangle the user clicks on)
            // unproject gets the WORLD coordinates of that point
            var worldVector = this.projector.unprojectVector(screenVector, this.camera);
            
            // +x is forward
            // +y is up
            // +z is right
            
            // dir is a unit vector giving the direction from the camera to the point on the rectangle
            var dir = worldVector.sub(this.camera.position).normalize();
            
            // theta is the yaw of dir (actuator coords in radians)
            var theta = Math.atan2(dir.z, dir.x);
            // phi is the pitch of dir (actuator coords in radians)
            var phi = Math.atan2(dir.y, Math.sqrt(Math.pow(dir.x, 2)+Math.pow(dir.z, 2)));
            
            //this.camLonRad = theta/Math.PI*180;
            //this.camLatRad = phi/Math.PI*180;
            //console.log(theta, phi);
            
            /*var lol = THREE.Math.degToRad(1.8);
            this.makeNiceQuadPolyThing(
                this.angleToWorldPos(theta-lol, phi-lol, 350), // tl
                this.angleToWorldPos(theta+lol, phi-lol, 350), // tr
                this.angleToWorldPos(theta-lol, phi+lol, 350), // bl
                this.angleToWorldPos(theta+lol, phi+lol, 350)  // br
            );*/
            
            return new THREE.Euler(0, theta, phi);
        },
        makeNiceQuadPolyThing: function(vertexList) {
            var geometry = new THREE.Geometry();
            
            geometry.vertices = vertexList;
            
            geometry.faces.push(new THREE.Face3(0, 1, 2));
            geometry.faces.push(new THREE.Face3(2, 1, 3));
            
            geometry.computeBoundingSphere();
            
            var material = new THREE.MeshBasicMaterial({
                color: '#'+(0x1000000+(Math.random())*0xffffff).toString(16).substr(1,6),
                side: THREE.DoubleSide,
            });
            var mesh = new THREE.Mesh(geometry, material);
            this.scene.add(mesh);
            
            mesh.geometry.dynamic = true;
            mesh.doubleSided = true;
            
            return mesh;
        },
        makeNiceLineThing: function(vertexList) {
            var geometry = new THREE.Geometry();
            
            geometry.vertices = vertexList;
            
            var material = new THREE.LineBasicMaterial( {
                color: 0xffffff,
                //color: '#'+(0x1000000+(Math.random())*0xffffff).toString(16).substr(1,6),
            } );
            var mesh = new THREE.Line(geometry, material, THREE.LineStrip);
            
            this.scene.add(mesh);
            
            mesh.geometry.dynamic = true;
            
            return mesh;
        },
        removeNiceQuadPolyThing: function(mesh) {
            this.scene.remove(mesh);
        },
        startReset: function() {
            this.panoramaContext.clearRect(0, 0, this.panoramaCanvasElement.width, this.panoramaCanvasElement.width);
            var ctx = this.panoramaContext;
            ctx.drawImage(this.wtfimage, 0, 0, this.wtfimage.width, this.wtfimage.height, 0, 0, this.panoramaCanvasElement.width, this.panoramaCanvasElement.height);
        },
        drawEvent: function(event) {
            var ctx = this.panoramaContext;
            if(event.type == 'thermopile_reading') {
                var xCount = event.temperature_grid.length;
                var yCount = event.temperature_grid[0].length;
                var psize = event.pixel_size;
                for (var i = 0; i < xCount; i++) {
                    for (var j = 0; j < yCount; j++) {
                        
                        var temp = event.temperature_grid[i][j];
                        
                        var yaw   = event.yaw   -xCount/2*psize +i*psize;
                        var pitch = event.pitch -yCount/2*psize +j*psize;
                        
                        //console.log(pitch, pitch+psize);
                        
                        var minTemp = 0, maxTemp = 10;
                        var fraction = (temp-minTemp)/(maxTemp-minTemp);
                        ctx.fillStyle = '#'+Math.floor(fraction*(Math.pow(2, 24)-1)).toString(16);
                        ctx.strokeStyle = 'red';
                        ctx.beginPath();
                        ctx.rect(
                            this.panoramaCanvasElement.height/180*(pitch+90)*2,
                            this.panoramaCanvasElement.width/360*(yaw+180),
                            psize*2*2, psize*2
                        );
                        ctx.fill();
                    }
                }
            }
        },
        endReset: function() {
            this.panoramaMaterial.map.needsUpdate = true;
        },
    });
});




