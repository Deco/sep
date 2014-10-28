#include "sensor_controller.h"
#include "serial_port.h"
#include "application_core.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

#include <boost/bind.hpp>
#include <boost/asio.hpp>


#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

/* Definitions for the ID byte read from sensor telling us 
 * what kind of data the sensor is sending */
#define AMBIENT_TEMP_DATA 0x11
#define SENSOR_DATA 0x12    
#define IMU_DATA 0x13

// TODO: Ambient temp data reading not being added to queue.

/* Created by: Chris Webb
 * Date Created: 7/6/14
 * Function: ThermalSensorController Constructor
 *
 * Changelog:
 * 21/10/2014: Modified to import a pointer to an Application_core object used 
 *             to start io_service work. CW
 */
ThermalSensorController::ThermalSensorController(
    std::shared_ptr<Application_core> core, const std::string _deviceName, unsigned int _baudRate
)   
    : app_core(core), deviceName(_deviceName), deviceBaudRate(_baudRate), readingQueue()
{
    
    boost::shared_ptr< boost::asio::io_service > io_service(
        new boost::asio::io_service
    );

    boost::shared_ptr< boost::asio::io_service::work > work(
        new boost::asio::io_service::work( *io_service )
    );

    ios = io_service;

}




/* Created by: Chris Webb
 * Date Created: 7/6/14
 * Function: init()
 * Description:
 * Set the bool(running) representing the sensors state to true then create a thread
 * that calls sensorThreadFunc().
 */
void ThermalSensorController::init()
{
    running = true;
    boost::asio::io_service::strand strand( *ios );
    //sensorThread = std::thread(&ThermalSensorController::sensorThreadFunc, this);

    // buffer of data to be written to device
    std::vector<byte> data;

    // buffer of data read from device
    std::vector<byte> buff;

    // sync device
    //ios->dispatch(boost::bind(&ThermalSensorController::sync, this, data, buff));
    strand.post( boost::bind( &ThermalSensorController::sync, this, data, buff ) );

    while (running){
        ios->post(boost::bind(&ThermalSensorController::takeReading, this, data, buff));        
    }

}


bool ThermalSensorController::sync(std::vector<byte> &data, std::vector<byte> &buff)
{
 std::cout << "Opening sensor device: " << deviceName << std::endl;

    // buffer of data to be written to device
    //unsigned char data[255]; 
    //std::vector<byte> data;

    // buffer of data read from device
    //unsigned char buff[255];
    //std::vector<byte> buff;
    
    
    sport.openDevice(deviceName.c_str(), deviceBaudRate);
    size_t readCount;

    // Start synchronisation process by sending a 255 byte
    data[0] = 255;
    //int sent = sport.write((char*)&data, 1);
    size_t sent = sport.writeDevice(data);
    assert(sent == 1);

    // Read bytes until 50 continuous 255 bytes have been read.
    int count = 0;
    do {
        readCount = sport.readDevice(buff, 1);
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
    sent = sport.writeDevice(data);
    assert(sent == 1);

    // if 254 is returned by sensor, synchronisation is complete
    do {
        readCount = sport.readDevice(buff, 1);
        if(readCount > 0 && buff[0] == 254) {
            break;
        }
    } while(true);
    printf("synced!\n");
}


bool ThermalSensorController::takeReading(std::vector<byte> &data, std::vector<byte> &buff)
{
    bool waitingForOrientation = false;

    // Create a new Reading object to store the sensor data
    Reading newReading;
    newReading.img.create(4, 16);  // 4 rows 16 cols
    newReading.time = time(0);     // set time = to current time
        
    // read sentinal byte
    sport.readDevice(buff, 1);
    //std::cout << "This byte should be 255!" << buff[0] << std::endl;
    assert(buff[0] == 255); 

    // read id byte representing what sensor is sending
    sport.readDevice(buff, 1);
    unsigned char id = buff[0];

    // read length of data that sensor is going to send
    sport.readDevice(buff, 2);
    unsigned short len = *((unsigned short*)buff.data());
    //byte buff2[sizeof(unsigned short)];
    //memcpy(&buff2,&buff,sizeof(unsigned short));
    //unsigned short len = buff2[0];

    // read sensor data into buffer
    sport.readDevice(buff, len);


    float ambientTemp = 0;        
    // What we do with the recently read sensor data is defined by the
    // id of the reading
    switch(id) {
        case AMBIENT_TEMP_DATA: {
            std::cout << "Reading new Ambient temp data!" << std::endl;
            assert(len == sizeof(float));
            float ambient = *((float*)buff.data());
            
            //memcpy(convert.b, &buff, sizeof(float));   

            ambientTemp = ambient;
            std::cout << "ambient temp: " << ambientTemp << std::endl;
            break;
        };
        case SENSOR_DATA: {
            std::cout << "Reading new MLX data!" << std::endl;
            assert(len == 64*sizeof(float));
            // create pointer to the new images data
            float *imgDataPtr = (float*)newReading.img.data; 
            if(1) {
                std::lock_guard<std::mutex> readingQueueLock(readingQueueMutex);
                memcpy(imgDataPtr, buff.data(), sizeof(float)*64);
                waitingForOrientation = true; // not used
                readingQueue.push(newReading);
            }
            break;
        };
    case IMU_DATA: {
    std::cout << "Reading new IMU data!" << std::endl;
    assert(len == 3*sizeof(float));
            
    //float roll = *((float*)buff);
    float *imu = (float*)buff.data();
    // add orientation to newReading should be around here
    std::cout << "roll: " << imu[0];
    std::cout << ", pitch:: " << imu[1];
    std::cout << ", yaw: " << imu[2] <<  std::endl;
    break;
    }
        default: {
            assert(1 == 0);
        }
    }
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
 */
/*bool ThermalSensorController::sensorThreadFunc()
{
   
     std::cout << "Opening sensor device: " << deviceName << std::endl;

    // buffer of data to be written to device
    //unsigned char data[255]; 
    std::vector<byte> data;

    // buffer of data read from device
    //unsigned char buff[255];
    std::vector<byte> buff;

    // Create a new Reading object to store the sensor data
    Reading newReading;
    newReading.img.create(4, 16);  // 4 rows 16 cols
    newReading.time = time(0);     // set time = to current time
    
    
    // Create a new serial connection to device
    SerialPort sc;
    sc.openDevice(deviceName.c_str(), 115200);
    size_t readCount;

    // Start synchronisation process by sending a 255 byte
    data[0] = 255;
    //int sent = sc.write((char*)&data, 1);
    size_t sent = sc.writeDevice(data);
    assert(sent == 1);

    // Read bytes until 50 continuous 255 bytes have been read.
    int count = 0;
    do {
        readCount = sc.readDevice(buff, 1);
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
    sent = sc.writeDevice(data);
    assert(sent == 1);

    // if 254 is returned by sensor, synchronisation is complete
    do {
        readCount = sc.readDevice(buff, 1);
        if(readCount > 0 && buff[0] == 254) {
            break;
        }
    } while(true);
    printf("synced!\n");


    // loop until controller has requested to stop
    while (running){
	bool waitingForOrientation = false;
        
        // read sentinal byte
        sc.readDevice(buff, 1);
	    //std::cout << "This byte should be 255!" << buff[0] << std::endl;
        assert(buff[0] == 255); 
    
        // read id byte representing what sensor is sending
        sc.readDevice(buff, 1);
        unsigned char id = buff[0];

        // read length of data that sensor is going to send
        sc.readDevice(buff, 2);
        unsigned short len = *((unsigned short*)buff.data());
        //byte buff2[sizeof(unsigned short)];
        //memcpy(&buff2,&buff,sizeof(unsigned short));
        //unsigned short len = buff2[0];

        // read sensor data into buffer
        sc.readDevice(buff, len);


        float ambientTemp = 0;        
        // What we do with the recently read sensor data is defined by the
        // id of the reading
        switch(id) {
            case AMBIENT_TEMP_DATA: {
                std::cout << "Reading new Ambient temp data!" << std::endl;
                assert(len == sizeof(float));
                float ambient = *((float*)buff.data());
                
                //memcpy(convert.b, &buff, sizeof(float));   

                ambientTemp = ambient;
                std::cout << "ambient temp: " << ambientTemp << std::endl;
                break;
            };
            case SENSOR_DATA: {
                std::cout << "Reading new MLX data!" << std::endl;
                assert(len == 64*sizeof(float));
                // create pointer to the new images data
                float *imgDataPtr = (float*)newReading.img.data; 
                if(1) {
                    std::lock_guard<std::mutex> readingQueueLock(readingQueueMutex);
                    memcpy(imgDataPtr, buff.data(), sizeof(float)*64);
		            waitingForOrientation = true; // not used
                    readingQueue.push(newReading);
                }
                break;
            };
	    case IMU_DATA: {
		std::cout << "Reading new IMU data!" << std::endl;
		assert(len == 3*sizeof(float));
                
		//float roll = *((float*)buff);
		float *imu = (float*)buff.data();
		// add orientation to newReading should be around here
		std::cout << "roll: " << imu[0];
		std::cout << ", pitch:: " << imu[1];
		std::cout << ", yaw: " << imu[2] <<  std::endl;
		break;
	    }
            default: {
                assert(1 == 0);
            }
        }
        
    }// end while
}*/



/* Created by: Chris Webb
 * Date: 7/6/14
 * Function: popThermopileReading
 * Import: 
 *      matRef: cv::Mat passed by reference to return the Reading at the front of
 *              the queue. 
 *      timeRef: time_t passed by reference to return the time the reading at the
 *              front of the queue was created.
 * Export:
 *      boolean containing true if queue has been popped otherwise false
 * Description:
 * Removes a Reading from the readingQueue atomically using mutexes and returns the 
 * values.
 *
 * Changelog:
 * 7/6/14: Created. CW
 */
bool ThermalSensorController::popThermopileReading(cv::Mat &matRef, time_t &timeRef)
{
    bool isReadingAvailable = false;
    Reading r;
    if(1) {
        std::lock_guard<std::mutex> readingQueueLock(readingQueueMutex);

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





