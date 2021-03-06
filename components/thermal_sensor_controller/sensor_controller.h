#ifndef SENSOR_CONTROLLER_H
#define SENSOR_CONTROLLER_H

#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>
#include <queue>
#include <ctime>
#include <memory>
#include <vector>

#include "serial_port.h"
#include "application_core.h"

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

// Different types of data from sensor
#define AMBIENT_TEMP_DATA 0x11
#define SENSOR_DATA 0x12    
#define IMU_DATA 0x13

typedef unsigned char byte;

// Struct to hold data from sensor
struct ThermoReading {
    unsigned char id;
    time_t time;
    cv::Mat_<float> img;
};
struct GyroReading {
    unsigned char id;
    time_t time;
    float roll, pitch, yaw;
};

class ThermalSensorController
{
public:
    // Constructor that imports a device name and baud rate of the sensor
    ThermalSensorController(
        std::shared_ptr<ApplicationCore> coreIn,
        const std::string _deviceName,
        unsigned int _baudRate = 115200
    );
    
    // Start reading data from the sensor within a new thread.
    void init();
    // Stop reading data from the sensor.
    void shutdown();
    // not implemented
    void update();
    // Return the result of queue pop
    bool popThermoReading(ThermoReading &r);
    bool popGyroReading(GyroReading &r);

    void requestThermoReading();
    void requestGyroReading();
    
    // If there is a Reading available, return true
    bool isThermoReadingAvailable();
    bool isGyroReadingAvailable();
    // Handle reading serial data from sensor
    void handleSerialData();
    // Serial Port class to handle reading and writing over serial port.    
    SerialPort sport;

    //void onSerialDataReady(); not yet implemented.

    /* not yet implemented.
    hook registerOnSerialDataReadyCallback(
        std::function<void ()> callbackPtr
      //std::function<void (const ActuatorInfo&, ActuatorState)> callbackPtr
    );

    virtual hook registerActuatorStateChangeCallback(
        std::function<void (const ActuatorInfo&, ActuatorState)> callbackPtr
    ) = 0;

    event<
        void(const ActuatorInfo&, double, double, bool)
    > eventActuatorMovementUpdate;

    event<void(ActuatorMoveOrder)> eventCurrentOrderChanged;
*/

private:
    // Seperate thread to continuously read from sensor.
    std::thread sensorThread;
    // Serial port of the device
    const std::string deviceName;
    // Baud rate of the device
    const unsigned int deviceBaudRate;
    // Mutex to stop threading errors (race conditions)
    std::mutex thermoReadingQueueMutex;
    std::mutex gyroReadingQueueMutex;
    // Queue containing all of the Readings from the sensor
    std::queue<ThermoReading> thermoReadingQueue;
    std::queue<GyroReading> gyroReadingQueue;
    // Boolean representing state of a device and if it's being read.
    bool running;
    // function to be ran in parallel to read from device
    void sensorThreadFunc();
    // core application pointer to use io_service
    std::shared_ptr<ApplicationCore> app_core;
    // pointer to io_service to be taken from app_core
    const std::shared_ptr<boost::asio::io_service> ios;
    // sync device on serial
    void syncDevice();

    
    /* not implemented
    hook hookOnSerialDataReady;
    hook hookEventProc;
    */

};


#endif
