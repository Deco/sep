
#include <boost/signals2/signal.hpp>
#include "threads.h"

#ifndef EVENTS_H
#define EVENTS_H

typedef boost::signals2::signal Event;
typedef boost::signals2::scoped_connection Hook;

template<typename T>
class Observed
{
public:
    Event<void(const T &newValue)> evChanged;

public:
    Observed() : value() { }
    Observed(T initialValue) : value(initialValue) { }
    
    void set(const T &newValue)
    {
        value.set(newValue);
        eventValueChanged(newValue);
    }
    T get() const
    {
        return value.get();
    }

private:
    Atom<T> value;

}

#endif//EVENTS_H
