#include <iostream>
#include <opencv2/core/core.hpp>
#include "sensor_controller.h"
#include "application_core.h"
#include "serial_port.h"

int main()
{
    std::shared_ptr<ApplicationCore> core = ApplicationCore::instantiate();
    ThermalSensorController sc(core, "/dev/ttyACM0", 115200);


    std::cout << "ThermalSensorController successfully created.\n";
    

    cv::Mat_<float> img;
    time_t timeOfRead;


    sc.init();

  /*  struct Reading {
        unsigned char id;
        time_t time;
        cv::Mat_<float> img;
        //cv::Vec3f orientation;
        std::vector<float> orientation;
        float ambientTemp;
    };*/

        Reading r;

    while(1) {
        if (sc.isReadingAvailable()) {
            sc.popThermopileReading(r);

            if (r.id == SENSOR_DATA) {
                //printf("print out MLX data here.\n");
                for (int i=0;i<4;i++) {
                    for (int j=0;j<16;j++) {
                        printf("%f ", r.img(i, j));
                    }
                    printf("\n");
                }
            } else if (r.id == IMU_DATA) {
                printf("Roll: %f, Pitch: %f, Yaw: %f.\n", r.orientation.data()[0], r.orientation.data()[1], r.orientation.data()[2]);
            } else if (r.id == AMBIENT_TEMP_DATA) {
                printf("Ambient temperature is %f.\n", r.ambientTemp);
            }
        }
    }

    
    return 0;
}
/*
int main()
{
    SerialConn sc("/dev/ttyACM0", 115200);
    int count = 0;
    while(count<20) {
        byte buff;
        int readCount = sc.read(&buff, 1);
        if(readCount > 0) {
            printf("byte: %x\n", (int)buff);
            count++;
        }
    }
    sc.close();
}*/

/*
int main()
{
    boost::asio::io_service io;
    boost::asio::serial_port sport(io, "/dev/ttyACM0");
    sport.set_option(boost::asio::serial_port_base::baud_rate(115200));
    
    while(true) {
        byte c;
        boost::asio::read(sport, boost::asio::buffer(&c, 1));
        
        printf("byte: %x\n", (int)c);
    }
}
*/    
