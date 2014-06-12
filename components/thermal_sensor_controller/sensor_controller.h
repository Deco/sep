#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>
#include <queue>
#include <ctime>


class ThermalSensorController
{
public:
    // Constructor that imports a device name and baud rate of the sensor
    /*ThermalSensorController(std::shared_ptr<ParamSet> pset);*/
    ThermalSensorController(
        const std::string _deviceName,
        unsigned int _baudRate=115200
    );
    
    // Start reading data from the sensor within a new thread.
    void init();
    // Stop reading data from the sensor.
    void shutdown();
    // not implemented
    void update();
    // Remove a reading from the queue of Readings and return by passed in references.
    // Return the result of queue pop
    bool popThermopileReading(cv::Mat &matRef, time_t &timeRef);
    
private:
    // Represents a single 16X4 reading from the MLX sensor and the time it was read.
    struct Reading {
        cv::Mat_<float> img;
        time_t time;
    };
private:
    // Seperate thread to continuously read from sensor.
    std::thread sensorThread;
    // Serial port of the device
    const std::string deviceName;
    // Baud rate of the device
    const unsigned int deviceBaudRate;
    // Mutex to stop threading errors (race conditions)
    std::mutex readingQueueMutex;
    // Queue containing all of the Readings from the sensor
    std::queue<Reading> readingQueue;
    // Boolean representing state of a device and if it's being read.
    bool running;
    // function to be ran in parallel to read from device
    void sensorThreadFunc();
};


