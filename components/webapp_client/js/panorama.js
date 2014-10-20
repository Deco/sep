
JS.require('JS.Class', 'JS.Observable', function(Class) {
    /*
    var SensorImageFeed = new Class({
        include: Observable,
        
        initialize: function(name) {
            this.name = name;
            this.image = 0;
        },
    });
    */
    
    PanoramaRenderer = new Class({
        initialize: function(container) {
            this.buildRenderer(container);
            this.buildScene();
            this.buildCamera();
            this.buildPanoramaSphere();
            this.buildLatLonLines();
            if(true) { this.buildTestCube(); }
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
            
            this.renderer = new rendererClass({
                //canvas: targetCanvas,
                antialias: true,
                // see for full list of parameters: http://threejs.org/docs/#Reference/Renderers/WebGLRenderer
            });
            this.renderer.setSize(container.innerWidth(), container.innerHeight());
            
            this.targetCanvas = this.renderer.domElement;
            container.append(this.targetCanvas);
            
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
            this.camLat = 0;
            this.camLon = -90;
        },
        buildPanoramaSphere: function() {
            var sphereGeometry = new THREE.SphereGeometry( // a sphere to apply the equirectangular texture to
                500, // radius
                60, 40 // number of width and height segments
            );
            sphereGeometry.applyMatrix(new THREE.Matrix4().makeScale(-1, 1, 1)); // invert it, so the polygons are facing inwards
            
            this.panoramaCanvas = $('<canvas></canvas>');
            this.panoramaCanvasElement = this.panoramaCanvas.get()[0];
            var canvas = this.panoramaCanvas.get()[0];
            var ctx = canvas.getContext("2d");
            /*ctx.fillStyle = "red";
            ctx.fillRect(this.panoramaCanvas.width()-10, 0, 10, this.panoramaCanvas.height());
            ctx.fillStyle = "red";
            ctx.fillRect(0, 0, this.panoramaCanvas.width(), 10);
            console.log(this.panoramaCanvas.get()[0].clientWidth);*/
            var textHeight = 256;
            ctx.font = "normal " + textHeight + "px Arial";
            var metrics = ctx.measureText("Sample Text");
            var textWidth = metrics.width;
            
            canvas.width = textWidth;
            canvas.height = textHeight*2;
            
            ctx.fillStyle = "gray";
            ctx.fillRect(0, 0, canvas.width, canvas.height);
            
            ctx.font = "normal " + textHeight + "px Arial";
            ctx.textAlign = "center";
            ctx.textBaseline = "middle";
            ctx.fillStyle = "#ff0000";
            ctx.fillText("Sample Text", textWidth / 2, textHeight);
            
            this.panoramaContext = ctx;
            
            /*
            for(var rectI = 0; rectI < 50; rectI++) {
                ctx.fillStyle = "red";
                ctx.fillRect(
                    this.panoramaCanvas.width()/50*rectI,
                    this.panoramaCanvas.height()/50*rectI,
                    50, 50
                );
                console.log(this.panoramaCanvas.width()/50*rectI);
            }*/
            var tex = new THREE.Texture(this.panoramaCanvas.get()[0]);
            //tex.wrapT = THREE.ClampToEdgeWrapping;
            //tex.wrapS = THREE.ClampToEdgeWrapping;
            
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
                if(self.panoramaCanvas && self.panoramaCanvas.getContext) {
                    var ctx = self.panoramaCanvas.getContext('2d');
                    ctx.drawImage(testImage, 0, 0);
                    self.panoramaMaterial.map.needsUpdate = true;
                }
            };*/
            
            this.sphereMesh = new THREE.Mesh(sphereGeometry, this.panoramaMaterial);
            this.scene.add(this.sphereMesh);
        },
        buildLatLonLines: function() {
            var lineMaterial = new THREE.LineBasicMaterial({
                color: 0x00ff00,
            });
            
            var sphereRadius = 500;
            var lineRadius = sphereRadius*0.95;
            var latCount = 12, lonCount = latCount*2;
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
        buildTestCube: function() {
            var geometry = new THREE.BoxGeometry(1,1,1);
            geometry.applyMatrix(new THREE.Matrix4().makeScale(1, 1, 1));
            
            var tex = new THREE.Texture(this.panoramaCanvas.get()[0]);
            
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
        },
        update: function(currTime, deltaTime) {
			//this.camLon += 0.01*deltaTime;
			this.camLat = Math.max(-85, Math.min(85, this.camLat));
			
            this.cube.rotation.y += 0.001*deltaTime;
            
            //this.panoramaMaterial.map.needsUpdate = true;
            this.render();
            
            if(this.wtfkeyboard.pressed("left" )) { this.camLon -= 0.08*deltaTime; }
            if(this.wtfkeyboard.pressed("right")) { this.camLon += 0.08*deltaTime; }
            if(this.wtfkeyboard.pressed("up"   )) { this.camLat += 0.08*deltaTime; }
            if(this.wtfkeyboard.pressed("down" )) { this.camLat -= 0.08*deltaTime; }
        },
        render: function() {
            // adapted from http://mrdoob.github.io/three.js/examples/webgl_panorama_equirectangular.html
			var phi = THREE.Math.degToRad(90-this.camLat);
			var theta = THREE.Math.degToRad(this.camLon);
			
			this.camera.target.x = 500 * Math.sin(phi) * Math.cos(theta);
			this.camera.target.y = 500 * Math.cos(phi);
			this.camera.target.z = 500 * Math.sin(phi) * Math.sin(theta);
			
			this.camera.lookAt(this.camera.target);
			
            this.renderer.render(this.scene, this.camera);
        },
        updatePanoramaMaterial: function(events) {
            // 
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
