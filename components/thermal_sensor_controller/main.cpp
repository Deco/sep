#include <iostream>
#include <opencv2/core/core.hpp>
#include "sensor_controller.h"
#include "application_core.h"
#include "serial_port.h"

int main()
{
    std::cout << "hello world!\n";
    std::shared_ptr<Application_core> core = Application_core::instantiate();
    ThermalSensorController sc(core, "/dev/ttyACM0", 115200);

    SerialPort sp();
    

    //Application_core asd;
    //asd.getCorePtr();
    //std::shared_ptr<Application_core> corePtr = std::make_shared<Application_core>(asd);
    //ThermalSensorController sc(corePtr, "/dev/ttyACM0");


    cv::Mat_<float> img;
    time_t timeOfRead;
/*    sc.init();

    while(1) {
        bool wasDataRead = sc.popThermopileReading(img, timeOfRead);
        if(wasDataRead) {
            for (int i=0;i<4;i++) {
                for (int j=0;j<16;j++) {
                    printf("%f ", img(i, j));
                }
                printf("\n");
            }
        }
    }*/
    
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
