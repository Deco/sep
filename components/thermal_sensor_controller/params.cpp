/* FILE: Params.cpp
 * AUTHOR: Declan White, Chris Webb, Thomas Smallridge
 * CREATED: 02/09/2014
 * CHANGELOG:
 * 04/09/2014: Lots of minor fixes to syntax to get it to compile without ParamManager and added 
 * main for testing.
 * 13/10/2014: Added name field for parameters. Implemented adding new parameters to tree. Tested 
 * structure
 * 14/10/2014: Added createParam/getParam for objects. Added appendParam/getParam for lists.
 * Refactored constructors for non-terminals/terminals.
 */

#include "params.h"
#include <iostream>
#include <sstream>
#include <boost/variant.hpp> 

using namespace std;


//Constructor for terminals (STRING, INT64, FLOAT64, BOOL)
Param::Param(
    ParamType typeIn,
    ParamValue defaultValueIn,
    std::string nameIn
)
    : type(typeIn)
    , defaultValue(defaultValueIn)
    , currentValueAtom(defaultValueIn)
    , name(nameIn)
{
    if (    type != ParamType::STRING
        &&  type != ParamType::INT64
        &&  type != ParamType::FLOAT64
        &&  type != ParamType::BOOL)
    {
        throw new std::runtime_error(
            "Invalid Param constructor call. Non-terminal type passed to terminal constructor"
            );
    }
}

Param::Param(
    ParamType typeIn,
    std::string nameIn
)
    : type(typeIn)
    , name(nameIn)
{
    if (    type != ParamType::OBJ
        &&  type != ParamType::LIST)
    {
        throw new std::runtime_error(
            "Invalid Param constructor call. Terminal type passed to non-terminal constructor"
            );
    }
}

///////// This is for OBJECTS only ///////// 
std::shared_ptr<Param> Param::getParam(
    std::string &&key
) {
    //If parameter is not an object, throw an exception
    if(type != ParamType::OBJ) {
        throw new std::runtime_error(
            "Tried to get non-obj parameter in object getParam()"
            );
    }
    
    //Create pointer for param to get from map
    std::shared_ptr<Param> returnParamPtr;
    
    //Access the map (thread-safe), de-varianting it to objectValue for dereferencing
    currentValueAtom.access_read([&key, &returnParamPtr] (const ParamValue &valueRef) {
        auto& objectValue = boost::get<std::map<const std::string, const std::shared_ptr<Param>>>(
            valueRef
        );
        
        //Get the parameter's pointer
        returnParamPtr = objectValue.at(key);
    });

    //Return the pointer to the parameter
    return returnParamPtr;
}


///////// This is for OBJECTS only ///////// 
//Creating a object parameter with a terminal child
std::shared_ptr<Param> Param::createParam(
    std::string &&key,
    ParamType childType,
    ParamValue &&childDefaultValue
) {
    if(type != ParamType::OBJ) {
        throw new std::runtime_error(
            "Tried to create non-obj parameter in object createParam()"
            );
    }

    //Create pointer for new parameter
    std::shared_ptr<Param> newParamPtr;

    //Access to get the map (stored in objectValue)
    currentValueAtom.access([&key, &newParamPtr, &childType, &childDefaultValue] (ParamValue &valueRef) {
        auto &objectValue = boost::get<std::map<const std::string, const std::shared_ptr<Param>>>(
            valueRef
        );
        //Create parameter and place it in map. Sets pointer to it's value for returning
        const std::string &keyc = key;
        newParamPtr = std::make_shared<Param>(childType, childDefaultValue, key);
        const auto &childPtr = newParamPtr;
        objectValue.insert(std::make_pair(keyc, childPtr));
    });

    //Create the pointer to the new parameter
    return newParamPtr;
}

///////// This is for OBJECTS only /////////
//Creating a object parameter with a non-terminal child
std::shared_ptr<Param> Param::createParam( 
    std::string &&key,
    ParamType childType
) {
    if(type != ParamType::OBJ) {
        throw new std::runtime_error(
            "Tried to create non-obj parameter in object createParam()"
            );
    }

    //Create pointer for new parameter
    std::shared_ptr<Param> newParamPtr;

    //Access to get the map (stored in objectValue)
    currentValueAtom.access([&key, &newParamPtr, &childType] (ParamValue &valueRef) {
        auto &objectValue = boost::get<std::map<const std::string, const std::shared_ptr<Param>>>(
            valueRef
        );
        //Create parameter and place it in map. Sets pointer to it's value for returning
        const std::string &keyc = key;
        newParamPtr = std::make_shared<Param>(childType, key);
        const auto &childPtr = newParamPtr;
        objectValue.insert(std::make_pair(keyc, childPtr));
    });

    //Create the pointer to the new parameter
    return newParamPtr;
}

///////// This is for LISTS only ///////// 
std::shared_ptr<Param> Param::getParam(
    int key
) {
    //If this parameter isn't a list, throw an exception
    if(type != ParamType::LIST) {
        throw new std::runtime_error(
            "Tried to get non-list parameter in list getParam()"
            );
    }

    //Create pointer for the return parameter
    std::shared_ptr<Param> returnParamPtr;

    //Get the vector list to access
    currentValueAtom.access_read([&key, &returnParamPtr] (const ParamValue &valueRef) {
        auto& objectValue = boost::get<std::vector<std::shared_ptr<Param>>>(
            valueRef
        );

        //Index the vector for the parameter to return
        const int &keyc = key;
        objectValue.at(keyc);
        
    });
    
    //return the pointer to the accessed parameter
    return returnParamPtr;
}


///////// This is for LISTS only ///////// 
//Creating a list parameter with a terminal child
std::shared_ptr<Param> Param::appendParam( // NEED ANOTHER VERSION OF THIS WITHOUT DEFAULT VALUE!!!!
    int key,
    ParamType childType,
    ParamValue &&childDefaultValue
) {
    if(type != ParamType::LIST) {
        throw new std::runtime_error(
            "Tried to create non-list parameter in list appendParam()"
            );
    }
    //Create pointer for new parameter
    std::shared_ptr<Param> newParamPtr;

    //Access to get the map (stored in objectValue)
    currentValueAtom.access([&key, &newParamPtr, &childType, &childDefaultValue] (ParamValue &valueRef) {
        auto& objectValue = boost::get<std::vector<std::shared_ptr<Param>>>(
            valueRef
        );
        //Create parameter and place at the back of the list. Sets pointer for returning
        std::stringstream nameStr;
        nameStr << '#' << key;
        newParamPtr = std::make_shared<Param>(childType, childDefaultValue, nameStr.str());
        const auto &childPtr = newParamPtr;
        objectValue.push_back(childPtr);
    });

    //Create the pointer to the new parameter
    return newParamPtr;
}

///////// This is for LISTS only ///////// 
//Creating a list parameter with a non-terminal child
std::shared_ptr<Param> Param::appendParam( 
    int key,
    ParamType childType
) {
    if(type != ParamType::LIST) {
        throw new std::runtime_error(
            "Tried to create non-list parameter in list appendParam()"
            );
    }
    //Create pointer for new parameter
    std::shared_ptr<Param> newParamPtr;

    //Access to get the map (stored in objectValue)
    currentValueAtom.access([&key, &newParamPtr, &childType] (ParamValue &valueRef) {
        auto& objectValue = boost::get<std::vector<std::shared_ptr<Param>>>(
            valueRef
        );
        //Create parameter and place at the back of the list. Sets pointer for returning
        std::stringstream nameStr;
        nameStr << '#' << key;
        newParamPtr = std::make_shared<Param>(childType, nameStr.str());
        const auto &childPtr = newParamPtr;
        objectValue.push_back(childPtr);
    });

    //Create the pointer to the new parameter
    return newParamPtr;
}


//Returns true if the parameter is a terminal, false if it is an object or list
bool Param::isTerminal() const
{
    return (
            type == ParamType::STRING
        ||  type == ParamType::INT64
        ||  type == ParamType::FLOAT64
        ||  type == ParamType::BOOL
    );
}

//Value getter
ParamValue Param::getValue() 
{
    return currentValueAtom.get();
}

//Value setter
void Param::setValue(ParamValue valueIn)
{
    if (isLockedBool.get()) {
        throw new std::runtime_error (
            "Attempted to set value of a locked parameter"
            );
    }
    else {
        currentValueAtom.set(valueIn);
    }
}

//Returns the default value for this parameter
const ParamValue& Param::getDefault()
{
    return defaultValue;
}

//Returns the minimum value for a parameter (assuming its a float64/int64)
ParamValue Param::getMinValue()
{
    if(type != ParamType::INT64 && type != ParamType::FLOAT64) {
        throw new std::runtime_error(
            "Attempt to get `min` from a param that is not an int64 or float64."
        );
    }
    return minimumValueAtom.get();
}

//Returns the maximum value for a parameter (assuming its a float64/int64)
ParamValue Param::getMaxValue()
{
    if(type != ParamType::INT64 && type != ParamType::FLOAT64) {
        throw new std::runtime_error(
            "Attempt to get `max` from a param that is not an int64 or float64."
        );
    }
    return maximumValueAtom.get();
}

//Returns the length of a parameter (assuming its a string)
int Param::getMaxLength()
{
    if(type != ParamType::STRING) {
        throw new std::runtime_error(
            "Attempt to get `maxlen` from a param that is not a string."
        );
    }
    return boost::get<uint64_t>(maximumValueAtom.get());
}

//Checks to see if the parameter is locked
bool Param::isLocked() 
{
    return isLockedBool.get();
}

//Locks or unlocks the parameter
void Param::setIsLocked(bool value)
{
    isLockedBool.set(value);
}

//Returns the name/key of the parameter
std::string Param::getName() {
    return name;
}


/********* PARAM MANAGER *********/
ParamManager::ParamManager(
    // 
)
{
    std::string rootName = "root";

    rootParamPtr = std::make_shared<Param>(
        ParamType::OBJ,
        rootName
    );
}

const std::shared_ptr<Param>& ParamManager::getRootParam()
{
    return rootParamPtr;
}
