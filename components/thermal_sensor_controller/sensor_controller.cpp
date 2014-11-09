#include "sensor_controller.h"
#include "serial_port.h"
#include "application_core.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <exception>
#include <atomic>

#include <boost/bind.hpp>
#include <boost/asio.hpp>


#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>


// Global variables to manage serial data
enum class SerialReadState {
    HEADER,
    DATA
};

SerialReadState readState = SerialReadState::HEADER;
int DataLength = 0;

// buffer of data read from device
unsigned char buff[100];
unsigned char data[100];

bool deviceSynced = false;

//bool thermoReadRequested = false;
//bool gyroReadRequested = false;

//std::atomic<GyroReading> gRead();
//std::atomic<ThermoReading> tRead();

//std::atomic<bool> gAvailable (false);// = false;
std::atomic<bool> gRequested (false);
std::atomic<bool> tRequested (false);
//gAvailable.store(false);
//std::atomic<bool> tAvailable (false);// = false;
//tAvailable.store(false);

//GyroReading g;
//ThermoReading t;


/* Created by: Chris Webb
 * Date Created: 7/6/14
 *
 * Changelog:
 * 21/10/2014: Modified to import a pointer to an Application_core object used 
 *             to start io_service work. CW
 * 2/11/2014: Started implementing io_service hook but not fully implemented.
 */
ThermalSensorController::ThermalSensorController(
    std::shared_ptr<ApplicationCore> coreIn,
    const std::string _deviceName, 
    unsigned int _baudRate
) : app_core(coreIn), 
    ios(coreIn->getIOService()),
    deviceName(_deviceName), 
    deviceBaudRate(_baudRate), 
    thermoReadingQueue(),
    gyroReadingQueue(),
    sport()
{
    /*
    hookOnSerialDataReady = sport.registerOnSerialDataReadyCallback(
            std::bind(
            &ThermalSensorController::onSerialDataReady,
            this, std::placeholders::_1
    );
    */
}


/* Work in progress:
   Function to post data to core when data is ready on sensor. 

void ThermalSensorController::onSerialDataReady()
{
    boost::asio::io_service::strand strand(*ios);
    strand.post(
        std::bind(&ThermalSensorController::handleSerialData, this)
    );
}*/



/* Created by: Chris Webb
 * Date Created: 2/11/14
 * Last Modified: 2/11/14
 * Description:
 * Flip between HEADER and DATA, reading data from the sensor if it's available.
 */
void ThermalSensorController::handleSerialData()
{
    while(true) {
        if(readState == SerialReadState::HEADER) {
            int availableBytes = sport.getAvailable();
            if(availableBytes < 4) {
                //printf("not 4 bytes available(only %d), returning.\n", availableBytes);
                return;
            }
            
            int readCount;
            readCount = sport.readDevice((char*)&buff, 4);
            assert(readCount == 4);

            assert(buff[0] == 255); 
            unsigned char id = buff[1];
            unsigned short length;

            memcpy(&length,&buff[2],2);

            // check id. USING HARDCODED NUMBERS because hexidecimal
            unsigned char correctIDs[3] = {17, 18, 19};
            if (memcmp(&id, &correctIDs[0], sizeof(id)) != 0 &&
                memcmp(&id, &correctIDs[1], sizeof(id)) != 0 &&
                memcmp(&id, &correctIDs[2], sizeof(id)) != 0) 
            {
                printf("%u is NOT equal to %u.\n", id, correctIDs[1]);
                throw std::runtime_error("id of reading is wrong");
            }

            // check length.
            unsigned short correctLengths[3] = {4, 256, 12};
            if (memcmp(&length, &correctLengths[0], sizeof(length)) != 0 &&
                memcmp(&length, &correctLengths[1], sizeof(length)) != 0 &&
                memcmp(&length, &correctLengths[2], sizeof(length)) != 0) 
            {
                throw std::runtime_error("length is unexpected");
            }
            
            readState = SerialReadState::DATA;
            DataLength = length;
            //printf("CORRECT AND FINAL HEADER: id:%x, length: %hu.\n", id, length);
        } 
        

        if(readState == SerialReadState::DATA) {
            int availableBytes = sport.getAvailable();
            if(availableBytes < DataLength) {
                //printf("not %d bytes available(only %d), returning.\n", DataLength, availableBytes);
                return;
            }
            unsigned char dataBuffer[300];
            //printf("Reading data now.\n");
            int readCount = sport.readDevice((char*)&dataBuffer, DataLength);
            assert(readCount == DataLength);
                

            unsigned char id = buff[1];


            //Reading newReading;
            //newReading.time = time(0);     // set time = to current time

            // What we do with the recently read sensor data is defined by the
            // id of the reading
            switch(id) {
                case AMBIENT_TEMP_DATA: {
                    //std::cout << "Reading new Ambient temp data!" << std::endl;
                 //   assert(DataLength == sizeof(float));
                //    float ambient = *((float*)dataBuffer);
                    //     std::cout << "ambient temp: " << ambientTemp << std::endl;

                 //   if(1) { //modified for presentation optimisation
                    //if (readRequested) {
                  //      std::lock_guard<std::mutex> readingQueueLock(readingQueueMutex);
                  //      newReading.id = AMBIENT_TEMP_DATA;
                  //      newReading.ambientTemp = ambient;
                        //printf("2Ambient temperature is %f.\n", newReading.ambientTemp);
                  //      readingQueue.push(newReading);
                 //   }

                    break;
                }
                case SENSOR_DATA: {
                    //std::cout << "Reading new MLX data!" << std::endl;
                    assert(DataLength == 64*sizeof(float));
                    ThermoReading newReading2;
                    newReading2.time = time(0);
                    // create pointer to the new images data
                    newReading2.img.create(4, 16);  // 4 rows 16 cols
                    float *imgDataPtr = (float*)newReading2.img.data; 
                    if(1){ //{ modified for presentation optimisation
                    //if (thermoReadRequested.load()) {
                        std::lock_guard<std::mutex> thermoReadingQueueLock(thermoReadingQueueMutex);
                        newReading2.id = SENSOR_DATA;
                        memcpy(imgDataPtr, dataBuffer, sizeof(float)*64);
                        if (tRequested.load())
                        thermoReadingQueue.push(newReading2);
                    }
                    break;
                }
                case IMU_DATA: {
                    //std::cout << "Reading new IMU data!" << std::endl;
                    assert(DataLength == 3*sizeof(float));
                    float *imu = (float*)dataBuffer;
                    //if(gyroReadRequested.load()) { 
                    if(1){
                        GyroReading newReading2;
                        newReading2.time = time(0);
                        std::lock_guard<std::mutex> readingQueueLock(gyroReadingQueueMutex);
                        newReading2.id = IMU_DATA;
                        newReading2.roll = imu[0];
                        newReading2.pitch = imu[0];
                        newReading2.yaw = imu[0];
                        //printf("Roll: %f, Pitch: %f, Yaw: %f.\n", newReading.orientation.data()[0], newReading.orientation.data()[1], newReading.orientation.data()[2]);
                        if (gRequested.load())
                        gyroReadingQueue.push(newReading2);
                    }
                    break;
                }
                default: {
                    throw std::runtime_error("should never execute this error, caught earlier when ID is wrong.");
                }
            }

            readState = SerialReadState::HEADER;
        }
    }
}




/* Created by: Chris Webb
 * Date Created: 7/6/14
 * Last Modified: 2/11/14
 * Description:
 * While running is true, continuously read data from sensor and store Readings in queue.
 */
void ThermalSensorController::sensorThreadFunc(){
    while (running) {
        handleSerialData();
    }
}

/* Created by: Chris Webb
 * Date Created: 7/6/14

 * Last Modified: 2/11/14
 * Description:
 * Set the bool(running) representing the sensors state to true then create a thread
 * that reads data from sensor.
 */
void ThermalSensorController::init()
{
    running = true;
    sport.openDevice(deviceName.c_str(), deviceBaudRate);
    if (!deviceSynced) {
        syncDevice();
        printf("Device sync'd.\n");
    }

    sensorThread = std::thread(&ThermalSensorController::sensorThreadFunc, this);
    
}

/* Created by: Chris Webb
 * Date Created: 2/11/14
 * Last Modified: 2/11/14
 * Description:
 * Achieves scynchronisation with physical sensor device.
 *
 * Changelog:
 * 2/11/14: Created. CW
 */
void ThermalSensorController::syncDevice()
{   
    int readCount;
    // Start synchronisation process by sending a 255 byte
    data[0] = 255;
    int sent = sport.writeDevice((char*)&data, 1);
    assert(sent == 1);

    // Read bytes until 50 continuous 255 bytes have been read.
    int count = 0;
    do {
        readCount = sport.readDevice((char*)&buff, 1);
        if(readCount > 0) {
            if(buff[0] == 255) {
                count++;
            } else {
                count = 0;
            }
        }
    } while(count < 50);

    // write 254 to sensor
    data[0] = 254;
    sent = sport.writeDevice((char*)&data, 1);
    assert(sent == 1);

    // if 254 is returned by sensor, synchronisation is complete
    do {
        readCount = sport.readDevice((char*)&buff, 1);
        if(readCount > 0 && buff[0] == 254) {
            break;
        }
    } while(true);

    // Sync complete and successful.
}










bool ThermalSensorController::isThermoReadingAvailable(){
    bool isReadingAvailable = false;
    std::lock_guard<std::mutex> thermoReadingQueueLock(thermoReadingQueueMutex);
    if(thermoReadingQueue.size() > 0) {
        isReadingAvailable = true;
    }
    return isReadingAvailable;
    //return tAvailable.load();
}

bool ThermalSensorController::isGyroReadingAvailable(){
    bool isReadingAvailable = false;
    std::lock_guard<std::mutex> gyroReadingQueueLock(gyroReadingQueueMutex);
    if(gyroReadingQueue.size() > 0) {
        isReadingAvailable = true;
    }
    return isReadingAvailable;
    //return gAvailable.load();
}

/* Created by: Chris Webb
 * Date: 7/6/14
 * Function: popThermopileReading
 * Import: 
 * Export: A Reading struct containing the reading at the front of the queue.
 * Description:
 * Removes a Reading from the readingQueue and returns it.
 *
 * Changelog:
 * 7/6/14: Created. CW
 * 2/11/14: Modified to allow for IMU sensor.
 */
bool ThermalSensorController::popReading(ThermoReading &r)
{

    tRequested.store(true);
    bool wasReadingPopped = false;
   // while (!isThermoReadingAvailable()) {
        
  //  }
    if(isThermoReadingAvailable()) {
        std::lock_guard<std::mutex> readingQueueLock(thermoReadingQueueMutex);
        if(thermoReadingQueue.size() > 0) {
            wasReadingPopped = true;
            r = thermoReadingQueue.front();
            thermoReadingQueue.pop();
                //tRequested = false;
                tRequested.store(false);
        }
    }
    //thermoReadRequested = false;

    return wasReadingPopped;
}





bool ThermalSensorController::popReading(GyroReading &r)
{
      gRequested.store(true);// = true;
    bool wasReadingPopped = false;
   // while (!isGyroReadingAvailable()) {
        
  //  }
    if(isThermoReadingAvailable()) {
        std::lock_guard<std::mutex> readingQueueLock(thermoReadingQueueMutex);
        if(thermoReadingQueue.size() > 0) {
            wasReadingPopped = true;
            r = gyroReadingQueue.front();
            gyroReadingQueue.pop();
      gRequested.store(false);
        }
    }

    //thermoReadRequested = false;
    return wasReadingPopped;
}



/* Created by: Chris Webb
 * Date: 7/6/14
 * Function: shutdown()
 * Description:
 * Set the bool(running) representing the sensors state to false.
 *
 * Changelog:
 * 7/6/14: Created. CW
 */
void ThermalSensorController::shutdown() 
{
    running = false;
}





