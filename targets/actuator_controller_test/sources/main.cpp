
#include <boost/asio.hpp>
#include <iostream>
#include "application_core.h"

int main()
{
    std::shared_ptr<ApplicationCore> core = ApplicationCore::instantiate();
    
    
    
    core->run();

    std::cout << "meow" << std::endl;
    
    return 0;
}
