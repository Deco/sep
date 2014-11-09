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

int main(int argc, char **argv)
{

    std::cout << "blah" << std::endl;

    std::shared_ptr<ApplicationCore> core = ApplicationCore::instantiate();

    auto ac = std::make_shared<ActuatorController>("/dev/tty.usbserial-A9S3VTXD");
    auto ns = std::make_shared<NetService>(core);

    ac->init();
    ns->init(9004);

    ns->registerCallback("move_actuator", [&](const rapidjson::Document &doc) {
        ac->stop();
        ActuatorMoveOrder order;
        order.posDeg = cv::Vec2d(
            std::max(-150.0, std::min(150.0, -doc["yaw"  ].GetDouble()/M_PI*180)),
            std::max(- 90.0, std::min( 90.0, -doc["pitch"].GetDouble()/M_PI*180))
        );
        order.duration = 0.8;
        ac->queueMove(order);
    });
    
    std::cout << "run" << std::endl;
    core->run();
    
}
