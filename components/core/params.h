/* FILE: Params.h
 * AUTHOR: Declan White
 * CREATED: 02/09/2014
 * CHANGELOG:
 * 04/09/2014: 
 * Lots of minor fixes to syntax to get it to compile.
 * Added setter for ParamAddress
 */

#include <memory>
#include <atomic>
#include <vector>
#include <map>
#include <initializer_list>

#include <boost/variant.hpp>
#include "threads.h"


#ifndef PARAMS_H
#define PARAMS_H 


/* [Param system design]
    Author: Declan White
    
    The parameter system allows for data to move within the system and between
    the system and the user with ease and transparency.
    
    Each parameter is either terminal or non-terminal.
    Terminal parameters are strings, integers and doubles; they contain data
    which is their value and attributes.
    Non-terminal parameters are objects and lists; they contain other paremeters
    and have the inferred attribute "length" which is the number of children
    parameters they have.
    
    Terminal types:
        - String:
            `val`    | Required | The value of the string.
            `def`    | Required | The default value of the string.
            `maxlen` |          | The maximum length of the value.
        
        - Integer:
            `val`    | Required | A 64-bit integer value.
            `def`    | Required | The default value.
            `min`    | Optional | The minimum value (inclusive).
            `max`    | Optional | The maximum value (inclusive).
        
        - Double:
            `val`    | Required | A 64-bit floating point decimal.
            `def`    | Required | The default value.
            `min`    | Optional | The minimum value (inclusive).
            `max`    | Optional | The maximum value (inclusive).
        
        - Boolean:
            `val`    | Required | `true` or `false`.
            `def`    | Required | The default value.
    
    Non-terminal types:
        - Object: A key-value map where keys are strings and values are other
            parameters of any type.
        
        - List: A list of other parameters (indices start at zero).
    
    At runtime, every parameter has the extra attribute `locked` which is set
    by the serverside. For example, when the actuator communicator is connected,
    the `serialPort` parameter cannot be modified. Another example is the
    position of an actuator: letting the client modify the parameter makes no
    sense because it would not actually cause the actuator to move; that must
    be performed through the appropriate action channels.
    
    Each terminal parameter can have the optional `choices` parameter which is
    for the purpose of displaying the possible values to the user.
    
    The entire parameter system is in a hierarchial form.
    The root parameter is an Object that contains the parameter objects for
    each module of the application.
    Example (arbitrary syntax):
        root = obj {
            actuatorComm = obj {
                baudRate = int64 {
                    val = 123;
                    def = 456;
                    min = 0;
                    max = 999;
                    locked = false;
                },
                serialPort = string {
                    val = "/dev/abc";
                    def = "/dev/123";
                    maxlen = 50;
                    locked = true;
                },
                actuatorDataList = list {
                    actuator1 = obj {
                        enabled = boolean {
                            val = true;
                            default = true;
                        },
                        axis =  string {
                            val = "yaw";
                            choices = {"yaw", "pitch"};
                        },
                        position = float64 {
                            val = 123.123;
                            def = 456.123;
                            min = 0;
                            max = 999.123;
                            locked = true;
                        },
                    },
                    actuator2 = obj {
                        axis =  string {
                            val = "pitch";
                            choices = {"yaw", "pitch"};
                        },
                        position = float64 {
                            val = 123.123;
                            def = 456.123;
                            min = 0;
                            max = 999.123;
                            locked = true;
                        },
                    },
                },
            },
            sensorController = {
                etc,
            }
        }
    
    
*/



class Param;
class ParamManager;

typedef boost::variant<
    /* STRING  */ std::string,
    /* INT64   */ uint64_t,
    /* FLOAT64 */ double,
    /* BOOL    */ bool,
    /* OBJ     */ std::map<const std::string, const std::shared_ptr<Param>>,
    /* LIST    */ std::vector<std::shared_ptr<Param>>//std::vector<const std::shared_ptr<Param>>
> ParamValue;


/* class Param
    Author: Declan White
    Description:
        TODO
    Changelog:
        [2014-09-26 DWW] Created.
*/
class Param {
public:
    
    enum class ParamType {
        STRING,
        INT64,
        FLOAT64,
        BOOL,
        OBJ,
        LIST,
    };
    


protected:
    friend class ParamManager;
    
    /* Param::(primary constructor)
        Author: Declan White
        Description:
            This constructor should only be called by the parent Param.
        Parameters: TODO
        Changelog:
            [2014-09-26 DWW] Created.
    */
    Param(
        ParamType typeIn, 
        ParamValue &&defaultValueIn,
        std::string &&name
    );


public:
    
    ///////// This is for OBJECTS only ///////// 
    const std::shared_ptr<Param>& getParam(
        std::string &&key
    );
    
    ///////// This is for OBJECTS only ///////// 
    const std::shared_ptr<Param>& createParam(
        std::string &&key,
        ParamType type,
        ParamValue &&defaultValue
    );
    
    
    ///////// This is for LISTS only ///////// 
    const std::shared_ptr<Param>& getParam(
        int key
    );
    
    
    ///////// This is for LISTS only ///////// 
    const std::shared_ptr<Param>& appendParam(
        int key,
        ParamType type,
        ParamValue &&defaultValue
    );
    

    // 
    bool isTerminal() const;
    
    // For all types
    //const ParamAddress &getAddress() const;
    
    // For terminal types
    ParamValue getValue();
    void setValue(ParamValue value);
    
    // For all types
    const ParamValue& getDefault();
    
    // For int64 and float64
    ParamValue getMinValue();
    ParamValue getMaxValue();
    
    // For string
    int getMaxLength();
    
    // 
    bool isLocked();
    void setIsLocked(bool value);
    
    // 
    int getChildCount();
    
    //returns name of the parameter
    std::string getName();

    
private:
    
    const std::string name;
    const ParamType type; 
    const ParamValue defaultValue; 
    atom<ParamValue> currentValueAtom;
    // It might be better to deal with these attributes using inheritance,
    // but that would increase complexity and reduce performance.
    atom<ParamValue> minimumValueAtom;
    atom<ParamValue> maximumValueAtom; // Used for both `max` and string's `maxlen`
    atom<bool> isLockedBool;
};



class ParamManager {

public:
    ParamManager();
    
    const std::shared_ptr<Param>& getRootParam();
    
private:
    std::shared_ptr<Param> rootParamPtr;

};



#endif//PARAMS_H