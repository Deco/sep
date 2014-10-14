
#include "sensor_controller.h"
#include "serial_port_bsd.hpp"

int main()
{
    std::shared_ptr<ApplicationCore> core = ApplicationCore::instantiate();
    ThermalSensorController sc(&core, "/dev/ttyACM0", 115200);
    
    cv::Mat_<float> img;
    time_t timeOfRead;
    sc.init();

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
