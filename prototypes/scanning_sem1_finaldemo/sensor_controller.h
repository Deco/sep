#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>
#include <queue>
#include <ctime>

class ThermalSensorController
{
public:
    /*ThermalSensorController(std::shared_ptr<ParamSet> pset);*/
    ThermalSensorController(
        const std::string _deviceName,
        unsigned int _baudRate=115200
    );
    
    void init();
    void shutdown();
    
    void update();
    void turnOff();
    
    bool popThermopileReading(cv::Mat &matRef, time_t &timeRef);
    
private:
    struct Reading {
        cv::Mat_<float> img;
        time_t time;
    };
private:
    std::thread sensorThread;
    
    const std::string deviceName;
    const unsigned int deviceBaudRate;
    
    std::mutex readingQueueMutex;
    std::queue<Reading> readingQueue;
    bool running;
    
    void sensorThreadFunc();
};


