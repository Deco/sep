
void Param::Param(
    ParamManager &managerRefIn,
    ParamAddress &&fullAddressIn,
    ParamType &&typeIn,
    ParamValue &&defaultValueIn
)
    : managerRef(managerRefIn)
    , fullAddress(fullAddressIn)
    , type(typeIn)
    , defaultValue(defaultValueIn)
    , currentValueAtom(defaultValueIn)
{
    // 
}

bool Param::isTerminal() const
{
    return (
            type == STRING
        ||  type == INT64
        ||  type == FLOAT64
        ||  type == BOOL
    );
}

const ParamAddress &Param::getAddress() const
{
    return fullAddress;
}

ParamValue Param::getValue() const
{
    return valueAtom.get();
}

void Param::setValue(ParamValue value)
{
    value.set(value);
}

const ParamValue& Param::getDefault()
{
    return defaultValue;
}

ParamValue Param::getMinValue()
{
    if(type != Type::INT64 && type != Type::FLOAT64) {
        throw new std::runtime_error(
            "Attempt to get `min` from a param that is not an int64 or float64."
        );
    }
    return minimumValueAtom.get();
}

ParamValue Param::getMaxValue()
{
    if(type != Type::INT64 && type != Type::FLOAT64) {
        throw new std::runtime_error(
            "Attempt to get `max` from a param that is not an int64 or float64."
        );
    }
    return maximumValueAtom.get();
}

int Param::getMaxLength()
{
    if(type != Type::STRING) {
        throw new std::runtime_error(
            "Attempt to get `maxlen` from a param that is not a string."
        );
    }
    return maximumValueAtom.get();
}


bool Param::isLocked() const
{
    
}
void Param::setIsLocked(bool value)
{
    

ParamAddress::ParamAddress(
std::vector<std::string> keyListIn
)
    : keyList(keyListIn)
{
    // 
}

ParamAddress(
    std::initializer_list<std::string> keyListIn
)
    : keyList(keyListIn)
{
    // 
}



