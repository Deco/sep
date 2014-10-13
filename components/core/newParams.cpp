/* FILE: Params.cpp
 * AUTHOR: Declan White
 * CREATED: 02/09/2014
 * CHANGELOG:
 * 04/09/2014: Lots of minor fixes to syntax to get it to compile without ParamManager and added 
 * main for testing.
 */

#include "newParams.h"
#include <iostream>
#include <boost/variant.hpp> 

using namespace std;


Param::Param(
    //ParamManager &managerRefIn,
    ParamAddress &fullAddressIn,
    ParamType &typeIn,
    ParamValue &defaultValueIn
)
    :// managerRef(managerRefIn)
     fullAddress(fullAddressIn)
    , type(typeIn)
    , defaultValue(defaultValueIn)
    , currentValueAtom(defaultValueIn)
{
    // 
}


Param::Param(
    //ParamManager &managerRefIn,
    ParamAddress &fullAddressIn,
    ParamType &typeIn
)
    :// managerRef(managerRefIn)
     fullAddress(fullAddressIn)
    , type(typeIn)
{
    // 
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


const ParamAddress &Param::getAddress() const
{
    return fullAddress;
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






ParamAddress::ParamAddress(
std::vector<std::string> keyListIn
)
    : keyList(keyListIn)
{
    // 
}


ParamAddress::ParamAddress(
    std::initializer_list<std::string> keyListIn
)
    : keyList(keyListIn)
{
    // 
}



ParamManager::ParamManager(
  int &inParam
)
    : root(inParam)
{
      /*std::vector<std::string> adr;
      adr.push_back("root");
      ParamAddress pAdr(adr);
      
      Param::ParamType type = Param::ParamType::OBJ;
      Param asd(pAdr, type);*/
      //root(pAdr, type);
        
      // maybe import address and type and use initialiser

}



