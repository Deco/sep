


#ifndef THREADS_H
#define THREADS_H

template<typename T>
class Atom
{
public:
    Atom() : value() { }
    Atom(T initialValue) : value(initialValue) { }
    
    T get()
    {
        std::shared_lock readLock(valueMutex);
        return value;
    }
    void set(T newValue)
    {
        std::unique_lock writeLock(valueMutex);
        value = newValue;
    }
    void access(std::function<void(T &value)> func)
    {
        std::unique_lock writeLock(valueMutex);
        func(value);
    }

private:
    std::mutex valueMutex;
    T value;

}

#endif//THREADS_H
