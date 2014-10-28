#ifndef APP_CORE_H
#define APP_CORE_H

#include <memory>
#include <boost/asio.hpp>


class Application_core{

public:
Application_core();


//void getCorePtr();

static std::shared_ptr<Application_core> instantiate();

std::shared_ptr<boost::asio::io_service>& getIOS();

private:
	static std::weak_ptr<Application_core> singletonInstanceWeakPtr;
	boost::asio::io_service ios;
};




#endif