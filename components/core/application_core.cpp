
#include "application_core.h"
#include <csignal>
#include <functional>
#include <cstddef>
#include <sstream>

std::weak_ptr<ApplicationCore> ApplicationCore::singletonInstanceWeakPtr = std::weak_ptr<ApplicationCore>();

/* ApplicationCore::instantiate
    Author: Declan White
    Changelog:
        [2014-09-26 DWW] Created.
*/
std::shared_ptr<ApplicationCore> ApplicationCore::instantiate(
    //
) {
    
    if(!singletonInstanceWeakPtr.expired()) { // If there is already a singleton instance..
        // ..throw an error.
        throw new std::runtime_error(
            "Attempt to create two instances of ApplicationCore!"
        );
    }
    
    // Otherwise construct an instance..
    auto appPtr = std::make_shared<ApplicationCore>();
    // ..store a weak_ptr to it.
    singletonInstanceWeakPtr = appPtr;
    
    // We need to associate the appropriate signal handlers to ensure the
    // applicaiton terminates when Ctrl+C or `kill` is used.
    std::signal(SIGTERM, ApplicationCore::handleRawSignal);
    std::signal(SIGINT , ApplicationCore::handleRawSignal);
    
    return appPtr;
}


/* ApplicationCore::(primary constructor)
    Author: Declan White
    Changelog:
        [2014-09-26 DWW] Created.
*/
ApplicationCore::ApplicationCore(
    //
)
    : iosPtr(std::make_shared<boost::asio::io_service>()) // Construct the IO service
    , workerThreadGroup() // Construct the worker thread group
    , logStrand(*iosPtr) // Construct the logging strand so that it works on ios
{
    // 
}


/* ApplicationCore::run
    Author: Declan White
    Changelog:
        [2014-09-26 DWW] Created.
*/
void ApplicationCore::run(
    //
) {
    
    // Give the IO service some fake work to do so that `io_service.run()`
    // doesn't terminate when there is nothing to do.
    boost::shared_ptr<boost::asio::io_service::work> fakeWork(
        new boost::asio::io_service::work(*iosPtr)
    );
    
    std::cout << "Spawning worker threads..." << std::endl;
    
    // Create as many threads as there are cores, so fully maximise our usage
    // of the host platform's processor.
    int coreCount = boost::thread::hardware_concurrency();
    for(int threadI = 0; threadI < coreCount; threadI++) {
        workerThreadGroup.create_thread([this,threadI]() {
            workerThreadFunc(threadI);
        });
    }
    
    // Block the main thread until the worker threads exit (when the application
    // has exited).
    // Theoretically, this call will sleep the main thread!
    workerThreadGroup.join_all();
    
    std::cout << "Application exiting..." << std::endl;
    
}


/* ApplicationCore::stop
    Author: Declan White
    Changelog:
        [2014-09-26 DWW] Created.
*/
void ApplicationCore::stop(
    //
) {
    // Tell the IO service to stop executing work.
    //ios.stop();
    // We post this through the log strand to ensure all log messages are
    // handled before stopping the IO service
    
    logStrand.post(std::bind(&boost::asio::io_service::stop, iosPtr));
}

/* ApplicationCore::workerThreadFunc
    Author: Declan White
    Changelog:
        [2014-09-26 DWW] Created.
*/
void ApplicationCore::workerThreadFunc(
    int threadNum
) {
    // Run any work the IO service has queued.
    log(LogLevel::INFO, [&](std::stringstream &ss) {
        ss << "Worker thread #" << threadNum << " running. ";
    });
    try {
        iosPtr->run();
    } catch (const std::exception& ex) {
        log(LogLevel::FATAL, [&](std::stringstream &ss) {
            ss << "Exception in IO service handler: " << ex.what();
        });
        stop();
    } catch (const std::string& ex) {
        log(LogLevel::FATAL, [&](std::stringstream &ss) {
            ss << "Exception in IO service handler: " << ex;
        });
        stop();
    } catch (...) {
        log(LogLevel::FATAL, [&](std::stringstream &ss) {
            ss << "Exception in IO service handler: UNKNOWN";
        });
        stop();
    }
}

/* ApplicationCore::log
    Author: Declan White
    Changelog:
        [2014-09-26 DWW] Created.
*/
void ApplicationCore::log(LogLevel level, std::string msg)
{
    logStrand.post(std::bind(&ApplicationCore::logHandler, this, level, msg));
}
void ApplicationCore::log(LogLevel level, std::stringstream msgStream)
{
    log(level, msgStream.str());
}
void ApplicationCore::log(LogLevel level, std::function<void(std::stringstream&)> msgStreamBuilderFunc)
{
    std::stringstream ss;
    msgStreamBuilderFunc(ss);
    log(level, ss.str());
}

/* ApplicationCore::logHandler
    Author: Declan White
    Changelog:
        [2014-09-26 DWW] Created.
*/
void ApplicationCore::logHandler(LogLevel level, std::string msg)
{
    std::cout
        << "["
        << (
                level == LogLevel::FATAL ? "FATAL"
            :   level == LogLevel::ERROR ? "ERROR"
            :   level == LogLevel::WARN  ? "WARN "
            :   level == LogLevel::INFO  ? "INFO "
            :   level == LogLevel::DEBUG ? "DEBUG"
            :   level == LogLevel::TRACE ? "TRACE"
            :   "UNKNOWN"
        )
        <<"] "
        << msg
        << std::endl;
    ;
}


/* ApplicationCore::handleRawSignal
    Author: Declan White
    Changelog:
        [2014-09-26 DWW] Created.
*/
void ApplicationCore::handleRawSignal(
    int signum
) {
    auto appPtr = singletonInstanceWeakPtr.lock();
    
    bool shouldQuit = false;
    if(signum == SIGTERM) {
        appPtr->log(LogLevel::FATAL, "Received SIGTERM; Quitting...");
        shouldQuit = true;
    } else if(signum == SIGINT) {
        appPtr->log(LogLevel::FATAL, "Received SIGINT; Quitting...");
        shouldQuit = true;
    } else {
        appPtr->log(LogLevel::WARN, [&](std::stringstream &ss) {
            ss << "Received unknown signal: " << signum;
        });
    }
    if(shouldQuit) {
        auto appPtr = singletonInstanceWeakPtr.lock();
        appPtr->stop();
    }
}

const std::shared_ptr<boost::asio::io_service> ApplicationCore::getIOService()
{
    return iosPtr;
}




