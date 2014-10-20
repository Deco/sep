#ifndef APP_CORE_H
#define APP_CORE_H

#include <memory>


class Application_core{

public:
Application_core();


//void getCorePtr();

static std::shared_ptr<Application_core> instantiate();

private:
	static std::weak_ptr<Application_core> singletonInstanceWeakPtr;
	//Application_core *core;
};




#endif