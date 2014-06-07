#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>

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
    
    bool popThermopileReading(cv::Mat &matRef, double &timeRef);
    
private:
    struct Reading {
        cv::Mat_<float> img;
        double time;
    };
private:
    std::thread sensorThread;
    
    const std::string deviceName;
    const unsigned int deviceBaudRate;
    
    std::mutex readingQueueMutex;
    std::queue<Reading> readingQueue;
    
    void sensorThreadFunc();
};


