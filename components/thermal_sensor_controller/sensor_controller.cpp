#include "sensor_controller.h"
#include "serial_port.h"
#include "application_core.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <exception>

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
    readingQueue(),
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


            Reading newReading;
            newReading.time = time(0);     // set time = to current time

            // What we do with the recently read sensor data is defined by the
            // id of the reading
            switch(id) {
                case AMBIENT_TEMP_DATA: {
                    //std::cout << "Reading new Ambient temp data!" << std::endl;
                    assert(DataLength == sizeof(float));
                    float ambient = *((float*)dataBuffer);
                    //     std::cout << "ambient temp: " << ambientTemp << std::endl;

                    if(1) {
                        std::lock_guard<std::mutex> readingQueueLock(readingQueueMutex);
                        newReading.id = AMBIENT_TEMP_DATA;
                        newReading.ambientTemp = ambient;
                        //printf("2Ambient temperature is %f.\n", newReading.ambientTemp);
                        readingQueue.push(newReading);
                    }

                    break;
                }
                case SENSOR_DATA: {
                    //std::cout << "Reading new MLX data!" << std::endl;
                    assert(DataLength == 64*sizeof(float));
                    // create pointer to the new images data
                    newReading.img.create(4, 16);  // 4 rows 16 cols
                    float *imgDataPtr = (float*)newReading.img.data; 
                    if(1) {
                        std::lock_guard<std::mutex> readingQueueLock(readingQueueMutex);
                        newReading.id = SENSOR_DATA;
                        memcpy(imgDataPtr, dataBuffer, sizeof(float)*64);
                        readingQueue.push(newReading);
                    }
                    break;
                }
                case IMU_DATA: {
                    //std::cout << "Reading new IMU data!" << std::endl;
                    assert(DataLength == 3*sizeof(float));
                    float *imu = (float*)dataBuffer;
                    if(1) {
                        std::lock_guard<std::mutex> readingQueueLock(readingQueueMutex);
                        newReading.id = IMU_DATA;
                        newReading.orientation.push_back(imu[0]);
                        newReading.orientation.push_back(imu[1]);
                        newReading.orientation.push_back(imu[2]);
                        //printf("Roll: %f, Pitch: %f, Yaw: %f.\n", newReading.orientation.data()[0], newReading.orientation.data()[1], newReading.orientation.data()[2]);
                        readingQueue.push(newReading);
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






/* Created by: Chris Webb
 * Date Created: 7/6/14
 * Last Modified: 22/10/2014
 * Function: sensorThreadFunc()
 * Description:
 * Function is designed to be ran continuously within its own thread. Function creates
 * a SerialConn object to communicate to a device over a Serial Port then
 * synchronises device by sending a 255 byte and waiting for the device to return
 * 50 bytes back containing 255 and then sending 254 and if it receives 254 in return,
 * the device has been successfully synchronised. New sensor readings are atomically 
 * placed within a queue (readingQueue).
 *
 * Changelog:
 * 7/6/14: Created. CW
 * 25/9/2014: Now supports IMU data. CW
 * 21/10/2014: Updated to use vectors instead of arrays and now using Declans SerialPort class. CW
 * 2/11/2014: Split function into multiple functions and modified design to check for bytes.
 
void ThermalSensorController::sensorThreadFunc()
{
  std::cout << "Opening sensor device: " << deviceName << std::endl;

    // buffer of data to be written to device
    unsigned char data[300]; 
    // buffer of data read from device
    unsigned char buff[300];

    // Create a new Reading object to store the sensor data
   // Reading newReading;
   // newReading.img.create(4, 16);  // 4 rows 16 cols
  //  newReading.time = time(0);     // set time = to current time
    
    
    // Create a new serial connection to device
    SerialPort sc;
    sc.openDevice(deviceName.c_str(), deviceBaudRate);
    int readCount;

    // Start synchronisation process by sending a 255 byte
    data[0] = 255;
    int sent = sc.writeDevice((char*)&data, 1);
    assert(sent == 1);

    // Read bytes until 50 continuous 255 bytes have been read.
    int count = 0;
    do {
        readCount = sc.readDevice((char*)&buff, 1);
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
    sent = sc.writeDevice((char*)&data, 1);
    assert(sent == 1);

    // if 254 is returned by sensor, synchronisation is complete
    do {
        readCount = sc.readDevice((char*)&buff, 1);
        if(readCount > 0 && buff[0] == 254) {
            break;
        }
    } while(true);

    // loop until controller has requested to stop
    while (running){

        // read sentinal byte
    while (sc.getAvailable() < 1) {
        // wait until data is available
    }
    readCount = sc.readDevice((char*)&buff, 1);


        assert(buff[0] == 255); 
        // read id byte representing what sensor is sending
         while (sc.getAvailable() < 1) {
            // wait for more data
        }
        sc.readDevice((char*)&buff, 1);
        unsigned char id = buff[0];
        //printf("ID: %x.\n", id);

        // read length of data that sensor is going to send
         while (sc.getAvailable() < 2) {
            // wait for more data
        }
        sc.readDevice((char*)&buff, 2);
        unsigned short len = *((unsigned short*)buff);



        while (sc.getAvailable() < len) {
            // wait for more data
        }
        // read sensor data into buffer
        sc.readDevice((char*)&buff, len);

        float ambientTemp = 0;
        
        // What we do with the recently read sensor data is defined by the
        // id of the reading

        Reading newReading;
        newReading.time = time(0);     // set time = to current time

        switch(id) {
            case AMBIENT_TEMP_DATA: {
         //       std::cout << "Reading new Ambient temp data!" << std::endl;
                assert(len == sizeof(float));
                float ambient = *((float*)buff);
                ambientTemp = ambient;
           //     std::cout << "ambient temp: " << ambientTemp << std::endl;

                if(1) {
                    std::lock_guard<std::mutex> readingQueueLock(readingQueueMutex);
                    newReading.id = AMBIENT_TEMP_DATA;
                    newReading.ambientTemp = ambient;
                    readingQueue.push(newReading);
                }

                break;
            };
            case SENSOR_DATA: {
             //   std::cout << "Reading new MLX data!" << std::endl;
                assert(len == 64*sizeof(float));
                // create pointer to the new images data
                newReading.img.create(4, 16);  // 4 rows 16 cols
                float *imgDataPtr = (float*)newReading.img.data; 
                if(1) {
                    std::lock_guard<std::mutex> readingQueueLock(readingQueueMutex);
                    newReading.id = SENSOR_DATA;
                    memcpy(imgDataPtr, buff, sizeof(float)*64);
                    readingQueue.push(newReading);
                }
                break;
            };
        case IMU_DATA: {
       //     std::cout << "Reading new IMU data!" << std::endl;
            assert(len == 3*sizeof(float));
                    
            //float roll = *((float*)buff);
            float *imu = (float*)buff;
            // add orientation to newReading should be around here

            if(1) {
                std::lock_guard<std::mutex> readingQueueLock(readingQueueMutex);
                newReading.id = IMU_DATA;
                newReading.orientation.push_back(imu[0]);
                newReading.orientation.push_back(imu[1]);
                newReading.orientation.push_back(imu[2]);
                readingQueue.push(newReading);
            }

            break;
           }
           default: {
                assert(1 == 0);
            }
        }
        
    }// end while
}*/







bool ThermalSensorController::isReadingAvailable(){
    bool isReadingAvailable = false;
    std::lock_guard<std::mutex> readingQueueLock(readingQueueMutex);
    if(readingQueue.size() > 0) {
        isReadingAvailable = true;
    }
    return isReadingAvailable;
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
Reading ThermalSensorController::popThermopileReading()
{
    Reading r;
    if(isReadingAvailable()) {
        std::lock_guard<std::mutex> readingQueueLock(readingQueueMutex);
        if(readingQueue.size() > 0) {
            r = readingQueue.front();
            readingQueue.pop();
        }
    }

    return r;
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





