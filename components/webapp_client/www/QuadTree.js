JS.require('JS.Class', 'JS.Observable', function(Class) {
    
    QuadTree = new Class({
        initialize: function(boundingBox, curDepth) {
            this.depth = curDepth + 1;
            //console.log("Initializing Quad Tree...");
            //console.log("Depth: " + this.depth);
            this.centerPoint = {x: ((boundingBox.x2 - boundingBox.x1)/2) + boundingBox.x1,
                                y: ((boundingBox.y2 - boundingBox.y1)/2) + boundingBox.y1};
                                
            this.halfWidth = Math.abs(boundingBox.x1 - boundingBox.x2)/2;
            this.halfHeight = Math.abs(boundingBox.y1 - boundingBox.y2)/2;
            
            this.maxCapacity = 4;
            this.arrayOfReadings = [];
            
            this.topLeft = null;
            this.topRight = null;
            this.bottomLeft = null;
            this.bottomRight = null;
            this.parentTree = null;
            /*console.log("Center = X: " + this.centerPoint.x + " Y: " + this.centerPoint.y);
            console.log("Width: " + this.halfWidth*2 + " Height: " + this.halfHeight*2);*/
        },
        insert: function(reading) {
            // console.log("Insertion, reading number : " + reading.tag);
            // console.log(reading.boundingBox.x1 + " " + reading.boundingBox.x2 + " " 
             //         + reading.boundingBox.y1 + " " + reading.boundingBox.y2);

            var rect = {x1: this.centerPoint.x - this.halfWidth,
                        x2: this.centerPoint.x + this.halfWidth,
                        y1: this.centerPoint.y - this.halfHeight,
                        y2: this.centerPoint.y + this.halfHeight};
                        
            // // console.log("Into");
            // console.log(rect.x1 + " " + rect.x2 + " " + rect.y1 + " " + rect.y2);
            if (!this.rectContainsEntireRect(rect, reading.boundingBox)) {
                return false;
            }
            if (this.arrayOfReadings.length < this.maxCapacity) {
                // console.log("Adding reading");
                // console.log(this.depth);
                this.arrayOfReadings[this.arrayOfReadings.length] = reading;
                return true;
            }
            if (this.topLeft === null) {
                this.subdivide();
            }
            if (this.topLeft.insert(reading)) return true;
            if (this.topRight.insert(reading)) return true;
            if (this.bottomLeft.insert(reading)) return true;
            if (this.bottomRight.insert(reading)) return true;
            
            // console.log("Can't fit into children, adding to parent");
            this.arrayOfReadings[this.arrayOfReadings.length] = reading;
            return false;
        },
        subdivide: function() {
            var boundingBox = {x1: this.centerPoint.x - this.halfWidth, 
                               x2: this.centerPoint.x,
                               y1: this.centerPoint.y - this.halfHeight,
                               y2: this.centerPoint.y};
                               
            this.topLeft = new QuadTree(boundingBox, this.depth);
            this.topLeft.parentTree = this;
            
            boundingBox.x1 = this.centerPoint.x;
            boundingBox.x2 = this.centerPoint.x + this.halfWidth;
            boundingBox.y1 = this.centerPoint.y - this.halfHeight;
            boundingBox.y2 = this.centerPoint.y;

            this.topRight = new QuadTree(boundingBox, this.depth);
            this.topRight.parentTree = this;
            
            boundingBox.x1 = this.centerPoint.x - this.halfWidth;
            boundingBox.x2 = this.centerPoint.x;
            boundingBox.y1 = this.centerPoint.y;
            boundingBox.y2 = this.centerPoint.y + this.halfHeight;

            this.bottomLeft = new QuadTree(boundingBox, this.depth);
            this.bottomLeft.parentTree = this;
            
            boundingBox.x1 = this.centerPoint.x;
            boundingBox.x2 = this.centerPoint.x + this.halfWidth;
            boundingBox.y1 = this.centerPoint.y;
            boundingBox.y2 = this.centerPoint.y + this.halfHeight;
            
            this.bottomRight = new QuadTree(boundingBox, this.depth);
            this.bottomRight.parentTree = this;
            
            for (var index = 0; index < this.arrayOfReadings.length; index++) {
                // console.log("Removing old readings!");
                if (!this.topLeft.insert(this.arrayOfReadings[index])) {
                    if (!this.topRight.insert(this.arrayOfReadings[index])) {
                        if (!this.bottomLeft.insert(this.arrayOfReadings[index])) {
                            this.bottomRight.insert(this.arrayOfReadings[index]);
                        }
                    }
                }
            }
            while(this.arrayOfReadings.length > 0) {
                this.arrayOfReadings.pop();
            }
            // console.log("Readings removed, this should be 0 : " + this.arrayOfReadings.length);
        },
        retrieve: function(boundingBox) {
            var readingsToReturn = [];
            var rect = {x1: this.centerPoint.x - this.halfWidth,
                        x2: this.centerPoint.x + this.halfWidth,
                        y1: this.centerPoint.y - this.halfHeight,
                        y2: this.centerPoint.y + this.halfHeight};
            if (!this.rectIntersectsRect(rect, boundingBox)) {
                return readingsToReturn;
            }

            for (var index = 0; index < this.arrayOfReadings.length; index++) {
                if (this.rectIntersectsRect(boundingBox, this.arrayOfReadings[index].boundingBox)) {
                    readingsToReturn[readingsToReturn.length] = this.arrayOfReadings[index];
                }
            }
            if (this.topLeft !== null) {
                readingsToReturn.push.apply(readingsToReturn, this.topLeft.retrieve(boundingBox));
                readingsToReturn.push.apply(readingsToReturn, this.topRight.retrieve(boundingBox));
                readingsToReturn.push.apply(readingsToReturn, this.bottomLeft.retrieve(boundingBox));
                readingsToReturn.push.apply(readingsToReturn, this.bottomRight.retrieve(boundingBox));   
            }
        
            return readingsToReturn;
        },
        rectContainsPoint: function(rect, point) {
            return (point.x >= rect.x1) && (point.x <= rect.x2) 
                && (point.y >= rect.y1) && (point.y <= rect.y2);
        },
        rectIntersectsRect: function(rect, rect2) {
            return (rect.x1 <= rect2.x2) && (rect2.x1 <= rect.x2)
                && (rect.y1 <= rect2.y2) && (rect2.y1 <= rect.y2);
        },
        rectContainsEntireRect: function(rect, rect2) {
            return (rect2.x1 >= rect.x1) && (rect2.x2 <= rect.x2) 
                && (rect2.y1 >= rect.y1) && (rect2.y2 <= rect.y2);
        }
    });
});