/*
  Author: Thomas Smallridge
  Created: 27/9/14
  Task: BS26
  Class responsible for controlling the client side of the parameter system. Holds the hierarchy
  of parameters and relevant functions to access and update the hierarchy. Will be worked on
  further in the coming weeks to interact with the server-side of the parameter system.
*/
  

//Example address
//['root', 'numbers', 'dummy', 'minValue']
    
ParameterController = new Class({ 
    //Test parameter hierarchy list
    //ex root.fields.numbers.fields.dummy.minValue = 2;
    root: {
        type: "obj",
        fields: {
            
            numbers: {
                type: "obj",
                fields: {
                    dummy: {
                        type: "int",
                        minValue: "5",
                        maxValue: "10,"
                    },
                    floaty: {
                        type: "float",
                        value: "3.5",
                    },
                }
            },
            
            list: {
                type: "list",
                children: [
                    {   type: "boolean",
                        value: "true",
                    },
                    {   type: "boolean",
                        value: "false",
                    },
                ]
            },
            
            stringy: {
                type: "string",
                value: "words",
            },
        }, 
    },
    
    sendParam: function() {
        //stub while server parameter system is under development
    },
    
    //Given an address, returns the type of the parameter at the end of that address
    getParamType: function(addr) {
        var param = getParam(addr);
        return param.type;
    },
    
    //Given an address and key for a data value, returns relevant data
    getParamData: function(addr, dataKey) {
        var param = getParam(addr);
        
        var data = param[dataKey];
        if (data === null) {
            throw ("Attempted to retrieve null data from parameter");
        }
        return data;
    },
    
    //Traverses the parameter hierarchy, returning the param at the given address
    getParam: function(addr) {
        
        //Set param to the root of the parameter hierarhy
        var param = this.root;
        
        //Traverse hierarchy to parameter addressed
        var keyI = 0;
        for(keyI = 0; keyI < addr.length; keyI++) {
            var key = addr[keyI];
            var keyType = typeof key;
            switch (param.type) {
                case "obj":
                    if (keyType != 'string') {
                        throw ("Expected key of type 'string'; Found: " + keyType);
                    }
                    param = param.fields[key];
                    break;
                case "list":
                    if ((keyType != 'number') || (key^0 != key)) {
                        throw ("Expected key of type 'int'; Found: " + keyType);
                    }
                    param = param[key];
                    break;
                default:
                    throw ("Expected non-terminal param or end of address; Found: " + param.type);
                }
            }
        return param;
    },
    
    setParam: function(addr, dataKey, value) {
       
       //Get the addressed parameter
        var param = getParam(addr);
        
        //Send proposed change to server
        //sendParam()
        
        //Wait for server reply
        
        //Update tree accordingly
        //param[dataKey] = value;
        
    },
    
    //Returns the fields of an object within the parameter hierarchy given its address
    getParamObjFields: function(addr) {
        
        //Get parameter addressed
        var param = getParam(addr);
        
        //Throw exception if addressed element not an object
        if (param.type != "obj") {
            throw "Given address not of type 'obj'";
        }
        
        //Build list of object fields and return that
        var returnFields = [];
        param.fields.forEach(function(info) {
            returnFields.push(info.toString());
        });
        
        return returnFields;
    },
    
});
