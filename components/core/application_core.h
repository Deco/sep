
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#ifndef CORE_H
#define CORE_H


/* class ApplicationCore
    Author: Declan White
    Description:
        Provides a singleton manager for centralised resources.
    Notes:
        This is a singleton class. Attempting to create two instances of this
        class will raise an exception.
    Changelog:
        [2014-09-26 DWW] Created.
*/
class ApplicationCore
{
public:
    enum class LogLevel {
        FATAL,
        ERROR,
        WARN,
        INFO,
        DEBUG,
        TRACE
    };    

public:
    
    /* ApplicationCore::instantiate
        Author: Declan White
        Description:
            This method will creates the singleton application instance and
            returns a pointer to it.
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-26 DWW] Created.
    */
    static std::shared_ptr<ApplicationCore> instantiate();
    

public:
    
    /* ApplicationCore::run
        Author: Declan White
        Description:
            This method executes the application and blocks until `stop` is
            called (typically by `handleRawSignal` when `SIGTERM` or `SIGINT`
            is received).
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-26 DWW] Created.
    */
    void run();
    
    /* ApplicationCore::stop
        Author: Declan White
        Description:
            This method signals the application to stop execution in a graceful
            manner.
        Note:
            This method does not block.
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-26 DWW] Created.
    */
    void stop();
    
    /* ApplicationCore::findParam
        Author: Declan White
        Description:
            Returns a reference to a param specified by the given address.
        Note:
            Throws an exception if a parameter could not be found at the given
            address.
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-26 DWW] Created.
    */
    std::weak_ptr<Param> &findParam(ParamAddress &&addr) const;

    
    /* ApplicationCore::log
        Author: Declan White
        Description:
            This method outputs a message to the terminal and other log
            destinatons in a thread-safe manner.
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-26 DWW] Created.
    */
    void log(LogLevel level, std::string msg);
    void log(LogLevel level, std::stringstream msgStream);
    void log(LogLevel level, std::function<void(std::stringstream)> msgStreamBuilderFunc);
    

private:
    
    /* ApplicationCore::(primary constructor)
        Author: Declan White
        Description:
            Constructs the singleton instance.
        Note:
            Must be private to prevent multipl constructions.
        Parameters: TODO
        Changelog:
            [2014-09-26 DWW] Created.
    */
    ApplicationCore();
    
    /* ApplicationCore::workerThreadFunc
        Author: Declan White
        Description:
            This method is executed by each worker thread and consumes work
            provided by the IO service.
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-26 DWW] Created.
    */
    void workerThreadFunc(int threadNum);
    
    /* ApplicationCore::logHandler
        Author: Declan White
        Description:
            This method handles the actual concatenations of a log message to
            the terminal and other log destinations.
        Notes:
            Dispatched serially by logStrand.
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-26 DWW] Created.
    */
    void logHandler(LogLevel level, std::string msg);
    

private:
    
    // IO service used to execute everything in the application
    boost::asio::io_service ios;
    
    // A group of threads for the IO service to execute on
    boost::thread_group workerThreadGroup;
    
    // A strand used to ensure logging is atomic
    boost::asio::io_service::strand logStrand;

/*#### Static ####*/
private:
    /* ApplicationCore::handleRawSignal
        Author: Declan White
        Description:
            This method handles quiting the application via Ctrl+C or the "kill"
            command.
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-26 DWW] Created.
    */
    static void handleRawSignal(int signum);

private:
    
    static std::weak_ptr<ApplicationCore> singletonInstanceWeakPtr;

};




#endif//CORE_H
