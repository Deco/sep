#include <iostream>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>
#include <algorithm>
#include <assert.h>
#include <sstream>
#include "actuator_controller.h"
#include "application_core.h"
#include "net_service.h"

int main()
{

    std::cout << "blah" << std::endl;

    std::shared_ptr<ApplicationCore> core = ApplicationCore::instantiate();

    auto ns = std::make_shared<NetService>(core);

    ns->init();
    
    std::cout << "run" << std::endl;
    core->run();


    // Initialises Actuator Controller
    //ActuatorController ac("/dev/tty.usbserial-A9S3VTXD");

    // Initialises Actuator Thread
    //ac.init();






}
