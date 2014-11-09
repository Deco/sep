#include <iostream>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>
#include <algorithm>
#include <assert.h>
#include <sstream>
#include "sensor_controller.h"
#include "actuator_controller.h"
#include "application_core.h"
#include "net_service.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

double interval = 1.0;

int main(int argc, char **argv)
{
    if(argc != 4) {
        std::cout << "wrong args" << std::endl;
        return 1;
    }
    const char *sensorDeviceName = argv[1];
    const char *actuatorDeviceName = argv[2];
    const int port = atoi(argv[3]);


    std::cout << "blah" << std::endl;

    std::shared_ptr<ApplicationCore> core = ApplicationCore::instantiate();

    auto sc = std::make_shared<ThermalSensorController>(core, sensorDeviceName, 115200);
    auto ac = std::make_shared<ActuatorController>("/dev/tty.usbserial-A9S3VTXD");
    auto ns = std::make_shared<NetService>(core);

    sc->init();
    ac->init();
    ns->init(9004);

    boost::asio::deadline_timer timer(*core->getIOService(), boost::posix_time::seconds(interval));
    std::function<void(const boost::system::error_code&)> captureStuff;
    GyroReading gyroReading;
    ThermoReading thermoReading;
    captureStuff = [&](const boost::system::error_code& /*e*/) { 

        /*std::cout << "uwat" << std::endl;

        if(sc->popGyroReading(gyroReading)) {
            printf("Roll: %f, Pitch: %f, Yaw: %f.\n",
                gyroReading.roll, gyroReading.pitch, gyroReading.yaw
            );
        }
        if(sc->popThermoReading(thermoReading)){
            for (int i=0;i<4;i++) {
                for (int j = 0; j < 16; j++) {
                    printf("%f ", thermoReading.img(i, j));
                }
                printf("\n");
            }
        }*/


        rapidjson::Document doc;
        doc.SetObject();

        d["type"].SetString("thermo_data");

        d["yaw"].SetDouble(100.0);
        d["pitch"].SetDouble(10.0);
        d["data"].Set

        timer.async_wait(captureStuff);
        timer.expires_at(timer.expires_at() + boost::posix_time::seconds(interval));
    };
    timer.async_wait(captureStuff);
    timer.expires_at(timer.expires_at() + boost::posix_time::seconds(interval));

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

