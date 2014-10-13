/* FILE: Params.cpp
 * AUTHOR: Declan White, Chris Webb, Thomas Smallridge
 * CREATED: 02/09/2014
 * CHANGELOG:
 * 04/09/2014: Lots of minor fixes to syntax to get it to compile without ParamManager and added 
 * main for testing.
 * 13/10/2014: Added name field for parameters. Implemented adding new parameters to tree. Tested 
 * structure
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
    if(type != ParamType::OBJ) {
        throw exception and stuff
    }
    
    std::shared_ptr<Param> childParamPtr;
    
    currentValueAtom.access_read([&key, &childParamPtr] (const ParamValue &valueRef) {
        auto& objectValue = boost::get<std::map<const std::string, const std::shared_ptr<Param>>>(
            valueRef
        );
        
        childParamPtr = objectValue.at(key);
    });
    
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
    /* you're learning a lot :P
        const-correctness
        managed pointers (shared_ptr, weak_ptr)
        references
        std::function and lambdas
        thread-safe operations
        good idea
        no probs :D
    */ //Anyways I'm leaving uni now. Thanks for the help/education :)
    return childParamPtr;
}


///////// This is for OBJECTS only ///////// 
const std::shared_ptr<Param>& createParam(
    std::string &&key,
    ParamType type,
    ParamValue &&defaultValue
) {
    if(type != ParamType::OBJ) {
        throw exception and stuff
    }
    
    // now, for createParam
    // Same thing, but notice the const
    // access_read forces that const
    // because it's access_READ
    
    // in createParam, you'll need to use just access
    // which gives you a non-const reference
    // which lets you use std::map<?>.emplace
    // (map.emplace CANNOT be used on a const value/reference)
    // (map.at CAN because it doesn't change the map)
    
    //Creates new parameter and places it within the map
    Param &newParam = objectValue.emplace(key, type, defaultValue, key);
    
    
    
    //Create the pointer to the new parameter
    return newParam;
}


///////// This is for LISTS only ///////// 
const std::shared_ptr<Param>& getParam(
    int key
) {
    if(type != ParamType::LIST) {
        throw exception and stuff
    }
    // implement me!
}


///////// This is for LISTS only ///////// 
const std::shared_ptr<Param>& createParam(
    int key,
    ParamType type,
    ParamValue &&defaultValue
) {
    if(type != ParamType::LIST) {
        throw exception and stuff
    }
    // implement me!
    // return a pointer to the new param for convenience
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

