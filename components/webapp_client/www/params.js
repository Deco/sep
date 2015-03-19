
window.paramManager = { // MAKE THIS A CLASS
    rootNode: {
        type: "root",
        nodes: {
            actuator_controller: {
                type: "node",
            }
        }
    },
    initialise: function() {
        // todoooo!
    },
    getRootNodeList: function() {
        return ["actuator_controller", "actuator_comm", "sensor_controller", "sensor_comm", "test"];
    },
    get: function(keyList) {
        if(key[0] == "actuator_controller") {
            // 
        } else if(key[0] == "actuator_comm") {
            if(key[1] == "sample_rate_sec") {
                return
            }
        } else if(key[0] == "sensor_controller") {
            
        } else if(key[0] == "sensor_comm") {
            
        }
        throw new Error("NYI");
    },
};
