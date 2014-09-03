
#include <mutex>

#ifndef THREADS_H
#define THREADS_H

/* class Atom
    Author: Declan White
    Description:
        A wrapper for primitive types that allows the value to accessed in a
        thread-safe manner.
    Template parameters:
        typename T: the type of the contained value.
    Changelog:
        [2014-09-02 DWW] Created.
*/
template<typename T>
class Atom
{
public:
    /* Atom::(default constructor)
        Author: Declan White
        Description: TODO
        Changelog:
            [2014-09-02 DWW] Created.
    */
    Atom() : value() { }
    
    /* Atom::(value copy constructor)
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Changelog:
            [2014-09-02 DWW] Created.
    */
    Atom(T initialValue) : value(initialValue) { }

public:
    /* Atom::get
        Author: Declan White
        Description:
            Atomically ("thread-safely") loads the value of the contained value.
        Returns: TODO
        Changelog:
            [2014-09-02 DWW] Created.
    */
    T get()
    {
        std::shared_lock readLock(valueMutex);
        return value;
    }
    
    /* Atom::set
        Author: Declan White
        Description:
            Atomically ("thread-safely") stores a new value to the contained
            value.
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-02 DWW] Created.
    */
    void set(T newValue)
    {
        std::unique_lock writeLock(valueMutex);
        value = newValue;
    }
    
    /* Atom::access
        Author: Declan White
        Description:
            Calls the method specified by the parameter `func` with the
            guarantee that it has exclusive thread-safe access to the contained
            value, which is provided using a reference.
        Parameters: TODO
        Changelog:
            [2014-09-02 DWW] Created.
    */
    void access(std::function<void(T &valueRef)> func)
    {
        std::unique_lock writeLock(valueMutex);
        T &valueRef = value;
        func(valueRef);
    }

private:
    std::mutex valueMutex;
    T value;

}

#endif//THREADS_H
