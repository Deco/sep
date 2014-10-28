#include <boost/signals2/signal.hpp>
#include "threads.h"

#ifndef EVENTS_H
#define EVENTS_H

/* alias event
    Author: Declan White
    Description:
        An alias for the signal class provided by boost::signals2.
        This should be used by classes providing events to be subscribed to by
        other classes.
    Changelog:
        [2014-09-02 DWW] Created.
*/
template<typename T>
using event = boost::signals2::signal<T>;

/* alias hook
    Author: Declan White
    Description:
        An alias for the scoped_connection class provided by boost::signals2.
        This should be used by classes subscribing to the events of other
        classes.
    Changelog:
        [2014-09-02 DWW] Created.
*/
using hook = boost::signals2::scoped_connection;


/* class observed
    Author: Declan White
    Description:
        A convenience wrapper for primitive types which allows listeners to be
        notified when the contained value changes.
        The contained value is accessed in a thread-safe manner.
    Changelog:
        [2014-09-02 DWW] Created.
*/
template<typename T>
class observed
{
public:
    event<void(const T &newValue)> evChanged;

public:
    observed() : value() { }
    observed(T initialValue) : value(initialValue) { }
    
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
    atom<T> value;

};

/* struct Cancellableevent
    Author: Nathan Monteloene
            http://stackoverflow.com/a/8438180/837856
    Description:
        Used to with an event (boost::signals2::signal) to make it possible for
        listeners to cancel the propragation of the event to other listeners.
        A return value of `true` indicates propragation of the event should
        halt.
    Changelog:
        [2014-09-02 DWW] Sourced from StackOverflow (with syntax modifications).
*/
struct cancellable {
    typedef bool result_type;
    
    template<typename InputIterator>
    result_type operator()(
        InputIterator aFirstObserver,
        InputIterator aLastObserver
    ) const
    {
        result_type val = false;
        for (; aFirstObserver != aLastObserver && !val; ++aFirstObserver) {
            val = *aFirstObserver;
        }
        return val;
    }
    
};

#endif//EVENTS_H