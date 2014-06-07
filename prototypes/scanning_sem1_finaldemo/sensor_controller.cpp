#include "sensor_controller.h"
#include "serialconn.h"
#include <mutex>

/* Definitions for the ID byte read from sensor telling us 
 * what kind of data the sensor is sending */
#define AMBIENT_TEMP 0x11
#define SENSOR_DATA 0x12    


ThermalSensorController::ThermalSensorController(
    const std::string _deviceName, unsigned int _baudRate
)   : deviceName(_deviceName), deviceBaudRate(_baudRate), readingQueue()
{
    //
}

/* Start a thread that reads data from sensor pushes to queue */
void ThermalSensorController::init()
{
    running = true;
    sensorThread = std::thread(&ThermalSensorController::sensorThreadFunc, this);
}

/* This func is ran by a thread to read data and put in queue */
void ThermalSensorController::sensorThreadFunc()
{
    std::cout << "Opening sensor device: " << deviceName << std::endl;

    unsigned char data[255];
    unsigned char buff[255];

    Reading newReading;
    newReading.img.create(4, 16);  // 4 rows 16 cols
    newReading.time = time(0);
    
    SerialConn sc(deviceName, 115200);
    int readCount;

    data[0] = 255;
    int sent = sc.write((char*)&data, 1);
    assert(sent == 1);

    int count = 0;
    do {
        readCount = sc.read((char*)&buff, 1);
        if(readCount > 0) {
            if(buff[0] == 255) {
                count++;
            } else {
                count = 0;
            }
        }
    } while(count < 50);

    data[0] = 254;
    sent = sc.write((char*)&data, 1);
    assert(sent == 1);


    do {
        readCount = sc.read((char*)&buff, 1);
        if(readCount > 0 && buff[0] == 254) {
            break;
        }
    } while(true);
    printf("synced!\n");

    while (running){
        sc.read((char*)&buff, 1);
        assert(buff[0] == 255); // sentinal byte

        sc.read((char*)&buff, 1);
        unsigned char id = buff[0];

        sc.read((char*)&buff, 2);
        unsigned short len = *((unsigned short*)buff);

        sc.read((char*)&buff, len);


        float ambientTemp = 0;

        switch(id) {
            case AMBIENT_TEMP: {
                std::cout << "Reading new Ambient temp data!" << std::endl;
                assert(len == sizeof(float));
                float ambient = *((float*)buff);
                ambientTemp = ambient;
                std::cout << "ambient temp: " << ambientTemp << std::endl;
                break;
            };
            case SENSOR_DATA: {
                std::cout << "Reading new sensor data!" << std::endl;
                assert(len == 64*sizeof(float));
                // create pointer to the new images data
                float *imgDataPtr = (float*)newReading.img.data; 
                if(1) {
                    std::lock_guard<std::mutex> readingQueueLock(readingQueueMutex);
                    memcpy(imgDataPtr, buff, sizeof(float)*64);
                    readingQueue.push(newReading);
                }
                break;
            };
            default: {
                assert(1 == 0);
            }
        }
    }
}




bool ThermalSensorController::popThermopileReading(cv::Mat &matRef, time_t &timeRef)
{
    bool isReadingAvailable = false;
    Reading r;
    if(1) {
        std::lock_guard<std::mutex> readingQueueLock(readingQueueMutex);

      //  readingQueueMutex.lock();
        if(readingQueue.size() > 0) {
            r = readingQueue.front();
            readingQueue.pop();
            isReadingAvailable = true;
        }
    }
    if(isReadingAvailable) {
        matRef = r.img;
        timeRef = r.time;
    }

    return isReadingAvailable;
}

void ThermalSensorController::turnOff() {

    running = false;

}
