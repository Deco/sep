#include <iostream>
#include <cmath>
#include <string>
#include <algorithm>
#include <assert.h>
#include <sstream>
#include "serialisation.h"
#include "sensor_controller.h"
#include "actuator_controller.h"
#include "application_core.h"
#include "net_service.h"
#include "rgb_controller.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

cv::Vec4b hsv(int hue, float sat, float val);

double interval = 100;

int main(int argc, char **argv)
{
    if(argc != 6) {
        std::cout << "wrong args" << std::endl;
        return 1;
    }
    const char *sensorDeviceName = argv[1];
    const char *actuatorDeviceName = argv[2];
    const int cameraDeviceNum = atoi(argv[3]);
    const int port = atoi(argv[4]);
    const char *mode = argv[5];

    bool showThermal = false;
    bool showRGB = false;
    if(mode[0] == 't') {
        showThermal = true;
    } else if(mode[0] == 'r') {
        showRGB = true;
    } else {
        throw "wtf";
    }

    std::cout << "blah" << std::endl;

    std::shared_ptr<ApplicationCore> core = ApplicationCore::instantiate();

    auto sc = std::make_shared<ThermalSensorController>(core, sensorDeviceName, 115200);
    auto rc = std::make_shared<RgbController>(core, cameraDeviceNum);
    auto ac = std::make_shared<ActuatorController>("/dev/tty.usbserial-A9S3VTXD");
    auto ns = std::make_shared<NetService>(core);

    sc->init();
    rc->init();
    ac->init();
    ns->init(port);

    boost::asio::deadline_timer timer(*core->getIOService());
    std::function<void(const boost::system::error_code&)> captureStuff;
    GyroReading gyroReading;
    ThermoReading thermoReading;
    captureStuff = [&](const boost::system::error_code& /*e*/) { 
        //
        cv::Vec2d pos = ac->getCurrentPosition();

        rc->captureFrame();
        auto rgbFrame = rc->popFrame();

        if(showRGB && rgbFrame->rows > 0) {
            rapidjson::Document doc;
            auto &aloc = doc.GetAllocator();
            doc.SetObject();

            cv::Mat imgMat(rgbFrame->rows, rgbFrame->cols, CV_8UC4, cv::Scalar::all(0.0));
            cv::cvtColor(*rgbFrame, imgMat, CV_BGR2RGBA, 4); 

            cv::Size size(rgbFrame->cols*0.2, rgbFrame->rows*0.2);
            cv::resize(imgMat, imgMat, size);

            std::string imgDataB64 = tobase64(imgMat.data, imgMat.total()*4*sizeof(byte));
            rapidjson::Value val;
            val.SetString(imgDataB64.c_str(), doc.GetAllocator());
            doc.AddMember("data", val, aloc);

            doc.AddMember("type", "rgb_data", aloc);
            doc.AddMember("yaw", -pos[0], aloc);
            doc.AddMember("pitch", -pos[1], aloc);
            doc.AddMember("dataWidth", imgMat.cols, aloc);
            doc.AddMember("dataHeight", imgMat.rows, aloc);
            doc.AddMember("yawSize", 63.625, aloc);
            doc.AddMember("pitchSize", 35.789, aloc);

            ns->sendWSDoc(doc);
        }


        /*if(sc->popGyroReading(gyroReading)) {
            printf("Roll: %f, Pitch: %f, Yaw: %f.\n",
                gyroReading.roll, gyroReading.pitch, gyroReading.yaw
            );
        }*/
        sc->requestThermoReading();

        std::cout << "tick: " << timer.expires_at() << std::endl;

        if(showThermal && sc->popThermoReading(thermoReading)){
            rapidjson::Document doc;
            auto &aloc = doc.GetAllocator();
            doc.SetObject();
            doc.AddMember("type", "thermo_data", aloc);
            doc.AddMember("yaw", -pos[0], aloc);
            doc.AddMember("pitch", -pos[1], aloc);

            cv::Mat imgMat(4, 16, CV_8UC4, cv::Scalar::all(0.0));
            cv::Mat mat = thermoReading.img;

            for(int i = 0; i < mat.total(); i++) {
                int y = 3-(i%4);
                int x = i/4;
                double temp = mat.at<float>(0, i);

                if(
                        (x == 11 && y == 2)
                    ||  (x == 11 && y == 3)
                    ||  (x == 12 && y == 2)
                ) {
                    temp += 10.0;
                }

                //std::cout << (int)temp << " ";

                cv::Vec4b col = hsv(
                    300-300.0*(std::max(temp, 14.0)-14.0)/(40.0-14.0),
                    1, 1
                );
                if(temp <= 11.0) {
                    col = cv::Vec4b(30, 30, 50, 255);
                } else if(temp > 40.0) {
                    col = cv::Vec4b(255, 255, 255, 255);
                }
                imgMat.at<cv::Vec4b>(y, x) = col;
                //std::cout << std::endl;
            }

            std::string imgDataB64 = tobase64(imgMat.data, imgMat.total()*4*sizeof(byte));
            rapidjson::Value val;
            val.SetString(imgDataB64.c_str(), doc.GetAllocator());
            doc.AddMember("data", val, aloc);

            ns->sendWSDoc(doc);
        }

        timer.expires_from_now(boost::posix_time::milliseconds(interval));
        timer.async_wait(captureStuff);
    };
    timer.expires_from_now(boost::posix_time::milliseconds(interval));
    timer.async_wait(captureStuff);

    ns->registerCallback("move_actuator", [&](const rapidjson::Document &doc) {
        ac->stop();
        ActuatorMoveOrder order;
        order.posDeg = cv::Vec2d(
            std::max(-150.0, std::min(150.0, -doc["yaw"  ].GetDouble()/M_PI*180)),
            std::max(- 90.0, std::min( 90.0, -doc["pitch"].GetDouble()/M_PI*180))
        );
        order.duration = 3.5;
        ac->queueMove(order);
    });
    
    std::cout << "run" << std::endl;
    core->run();
}



// hue: 0-360°; sat: 0.
cv::Vec4b hsv(int hue, float sat, float val)
{
    hue %= 360;
    while(hue<0) hue += 360;

    if(sat<0.f) sat = 0.f;
    if(sat>1.f) sat = 1.f;

    if(val<0.f) val = 0.f;
    if(val>1.f) val = 1.f;

    int h = hue/60;
    float f = float(hue)/60-h;
    float p = val*(1.f-sat);
    float q = val*(1.f-sat*f);
    float t = val*(1.f-sat*(1-f));

    switch(h)
    {
        default:
        case 0:
        case 6: return cv::Vec4b(val*255, t*255, p*255, 255);
        case 1: return cv::Vec4b(q*255, val*255, p*255, 255);
        case 2: return cv::Vec4b(p*255, val*255, t*255, 255);
        case 3: return cv::Vec4b(p*255, q*255, val*255, 255);
        case 4: return cv::Vec4b(t*255, p*255, val*255, 255);
        case 5: return cv::Vec4b(val*255, p*255, q*255, 255);
    }
}

