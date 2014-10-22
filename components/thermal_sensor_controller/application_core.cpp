#include "application_core.h"
#include <memory>
#include <iostream>
#include <exception>




std::weak_ptr<Application_core> Application_core::singletonInstanceWeakPtr;

Application_core::Application_core()
    : ios()
{
	//
}

boost::asio::io_service& Application_core::getIOS()
{
    return ios;
}



std::shared_ptr<Application_core> Application_core::instantiate() {
	 
    if(auto test = singletonInstanceWeakPtr.lock()) { // If there is already a singleton instance..
        // ..throw an error.
        throw std::runtime_error("Attempt to create two instances of ApplicationCore!");
    }
    
    // Otherwise construct an instance..
    auto appPtr = std::make_shared<Application_core>();
    // ..store a weak_ptr to it.
    singletonInstanceWeakPtr = appPtr;
    
    // We need to associate the appropriate signal handlers to ensure the
    // applicaiton terminates when Ctrl+C or `kill` is used.
    //std::signal(SIGTERM, ApplicationCore::handleRawSignal);
    //std::signal(SIGINT , ApplicationCore::handleRawSignal);
    
    return appPtr;
}


//void Application_core::getCorePtr(){
//	std::cout << "return pointer to application core now.\n";
	//ptr = std::make_shared<Application_core>(core);
//}