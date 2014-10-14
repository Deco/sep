/* FILE: Params.cpp
 * AUTHOR: Declan White, Chris Webb, Thomas Smallridge
 * CREATED: 02/09/2014
 * CHANGELOG:
 * 04/09/2014: Lots of minor fixes to syntax to get it to compile without ParamManager and added 
 * main for testing.
 * 13/10/2014: Added name field for parameters. Implemented adding new parameters to tree. Tested 
 * structure
 * 14/10/2014: Added createParam/getParam for objects
 */

#include "params.h"
#include <iostream>
#include <boost/variant.hpp> 

using namespace std;


Param::Param(
    ParamType typeIn,
    ParamValue &&defaultValueIn,
    std::string &&nameIn
)
    : type(typeIn)
    , defaultValue(defaultValueIn)
    , currentValueAtom(defaultValueIn)
    , name(nameIn)
{
    // 
}


///////// This is for OBJECTS only ///////// 
const std::shared_ptr<Param>& getParam(
    std::string &&key
) {
    //If parameter is not an object, throw an exception
    if(type != ParamType::OBJ) {
        try {
            throw "Tried to get non-obj parameter in object getParam()";
        }
        catch (std::string e) {
             std::cout << "An exception occurred. " << e << std::endl;
        }
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
    
    /* Ok, so to summarise:
        currentValueAtom.access_read blocks until the "boxed" value is ready to read safely
        It then calls the lambda function
        with `valueRef` being a reference to the "boxed" value
        We know this param is an object, so:
        we use boost::get to "de-variant-ise" out the std::map
        and we store a reference to it in objectValue (no copying, yay!)
        we then use the key passed in to access that map
        and we set childParamPtr to point to obtained value
    */
    
}


///////// This is for OBJECTS only ///////// 
const std::shared_ptr<Param>& createParam(
    std::string &&key,
    ParamType type,
    ParamValue &&defaultValue
) {
    if(type != ParamType::OBJ) {
        try {
            throw "Tried to create non-obj parameter in object createParam()";
        }
        catch (std::string e) {
             std::cout << "An exception occurred. " << e << std::endl;
        }
    }

    //Create pointer for new parameter
    std::shared_ptr<Param> newParamPtr;

    //Access to get the map (stored in objectValue)
    currentValueAtom.access([&key, &newParamPtr] (const ParamValue &valueRef) {
        auto& objectValue = boost::get<std::map<const std::string, const std::shared_ptr<Param>>>(
            valueRef
        );
        //Create parameter and place it in map. Sets pointer to it's value for returning
        newParamPtr = objectValue.emplace(key, type, defaultValue, key);
    });

    //Create the pointer to the new parameter
    return newParamPtr;
}


///////// This is for LISTS only ///////// 
const std::shared_ptr<Param>& getParam(
    int key
) {
    //If this parameter isn't a list, throw an exception
    if(type != ParamType::LIST) {
        try {
            throw "Tried to get non-list parameter in list getParam()";
        }
        catch (std::string e) {
             std::cout << "An exception occurred. " << e << std::endl;
        }
    }

    //Create pointer for the return parameter
    std::shared_ptr<Param> returnParamPtr;

    //Get the vector list to access
    currentValueAtom.access_read([&key, &returnParamPtr] (const ParamValue &valueRef) {
        auto& objectValue = boost::get<std::vector<const std::shared_ptr<Param>>>(
            valueRef
        );

        //Index the vector for the parameter to return
        returnParamPtr = objectValue.at(key);
        
    });
    
    //return the pointer to the accessed parameter
    return returnParamPtr;
}


///////// This is for LISTS only ///////// 
const std::shared_ptr<Param>& appendParam(
    int key,
    ParamType type,
    ParamValue &&defaultValue
) {
    if(type != ParamType::LIST) {
        try {
            throw "Tried to create non-list parameter in list appendParam()";
        }
        catch (std::string e) {
             std::cout << "An exception occurred. " << e << std::endl;
        }
    }
    //Create pointer for new parameter
    std::shared_ptr<Param> newParamPtr;

    //Access to get the map (stored in objectValue)
    currentValueAtom.access([&key, &newParamPtr] (const ParamValue &valueRef) {
        auto& objectValue = boost::get<std::vector<const std::shared_ptr<Param>>>(
            valueRef
        );
        //Create parameter and place at the back of the list. Sets pointer for returning
        newParamPtr = objectValue.emplace_back(type, defaultValue, key);
    });

    //Create the pointer to the new parameter
    return newParamPtr;
}



bool Param::isTerminal() const
{
    return (
            type == ParamType::STRING
        ||  type == ParamType::INT64
        ||  type == ParamType::FLOAT64
        ||  type == ParamType::BOOL
    );
}


ParamValue Param::getValue() 
{
    return currentValueAtom.get();
}


void Param::setValue(ParamValue valueIn)
{
    currentValueAtom.set(valueIn);
}


const ParamValue& Param::getDefault()
{
    return defaultValue;
}


ParamValue Param::getMinValue()
{
    if(type != ParamType::INT64 && type != ParamType::FLOAT64) {
        throw new std::runtime_error(
            "Attempt to get `min` from a param that is not an int64 or float64."
        );
    }
    return minimumValueAtom.get();
}


ParamValue Param::getMaxValue()
{
    if(type != ParamType::INT64 && type != ParamType::FLOAT64) {
        throw new std::runtime_error(
            "Attempt to get `max` from a param that is not an int64 or float64."
        );
    }
    return maximumValueAtom.get();
}


int Param::getMaxLength()
{
    if(type != ParamType::STRING) {
        throw new std::runtime_error(
            "Attempt to get `maxlen` from a param that is not a string."
        );
    }
    return boost::get<uint64_t>(maximumValueAtom.get());
}


bool Param::isLocked() 
{
    return isLockedBool.get();
}


void Param::setIsLocked(bool value)
{
    isLockedBool.set(value);
}

//Author: Tom || Date: 13/10/14
std::string Param::getName() {
    return name;
}



/********* PARAM MANAGER *********/
ParamManager::ParamManager(
    // 
)
{
    rootParamPtr = std::make_shared(
        // todo
    );
}

const std::shared_ptr<Param>& ParamManager::getRootParam()
{
    return rootParamPtr;
}

