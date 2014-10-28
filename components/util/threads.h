/* FILE: threads.h
 * AUTHOR: Declan White
 * CREATED: 02/09/2014
 * CHANGELOG:
 * 04/09/2014: Lots of minor fixes to syntax to get it to compile.
 */
#include <mutex>
#include <condition_variable>
#include <thread>

#ifndef THREADS_H
#define THREADS_H






/* class atom
    Author: Declan White
    Description:
        A wrapper for primitive types that allows the value to accessed in a
        thread-safe manner.
    Template parameters:
        typename T: the type of the contained value.
    Changelog:
        [2014-09-02 DWW] Created.
        [2014-09-04 DWW] Modified to include a condition variable.
*/
template<typename T>
class atom
{
public:
    /* atom::(default constructor)
        Author: Declan White
        Description: TODO
        Changelog:
            [2014-09-02 DWW] Created.
    */
    atom() : value() {
        //
    }
    
    /* atom::(value copy constructor)
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Changelog:
            [2014-09-02 DWW] Created.
    */
    atom(T initialValue) : value(initialValue) { }

public:
    /* atom::get
        Author: Declan White
        Description:
            atomically ("thread-safely") loads the value of the contained value.
        Returns: TODO
        Changelog:
            [2014-09-02 DWW] Created.
    */
    T get()
    {
        std::lock_guard<std::mutex> lock(valueMutex);
        //std::shared_lock readLock(valueMutex);
        return value;
    }

    
    /* atom::set
        Author: Declan White
        Description:
            atomically ("thread-safely") stores a new value to the contained
            value.
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-02 DWW] Created.
    */ 
    void set(T newValue)
    {
        if(1) {
            std::unique_lock<std::mutex> lock(valueMutex, std::try_to_lock);
            //std::unique_lock writeLock(valueMutex);
            value = newValue;
        }
        valueConditionVariable.notify_all();
    }

    /* atom::access
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
        if(1) {
            //std::unique_lock writeLock(valueMutex);
            std::unique_lock<std::mutex> lock(valueMutex,std::try_to_lock);
            T &valueRef = value;
            func(valueRef);
        }
        valueConditionVariable.notify_all();
    }
    
    /* atom::access_read
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */ 
    void access_read(std::function<void(const T &valueRef)> func)
    {
        if(1) {
            //std::shared_lock readLock(valueMutex);
            std::lock_guard<std::mutex> lock(valueMutex);
            const T &valueRef = value;
            func(valueRef);
        }
        //valueConditionVariable.notify_all();
    } 
    
    /* atom::conditional_wait
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
            
    void conditional_wait(std::function<bool(const T &valueRef)> predicate)
    {
        //std::unique_lock conditionLock(valueMutex); original
        //std::unique_lock(valueMutex);
        std::unique_lock <std::mutex> conditionLock(valueMutex); // maybe this


        valueConditionVariable.wait(conditionLock, [&predicate,this]{ 
            return predicate(this->value);
        });
    }
   
private:
    std::mutex valueMutex;
    std::condition_variable valueConditionVariable;
    T value;
};


#endif//THREADS_H