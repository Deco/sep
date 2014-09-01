#include "sensor_controller.h"
#include "serialconn.h"
#include <mutex>

/* Definitions for the ID byte read from sensor telling us 
 * what kind of data the sensor is sending */
#define AMBIENT_TEMP_DATA 0x11
#define SENSOR_DATA 0x12    
#define IMU_DATA 0x13

// TODO: create a way to stop the thread: set running = false and stop thread



/* Created by: Chris Webb
 * Date: 7/6/14
 * Function: ThermalSensorController Constructor
 */
ThermalSensorController::ThermalSensorController(
    const std::string _deviceName, unsigned int _baudRate
)   : deviceName(_deviceName), deviceBaudRate(_baudRate), readingQueue()
{
    //
}



/* Created by: Chris Webb
 * Date: 7/6/14
 * Function: init()
 * Description:
 * Set the bool(running) representing the sensors state to true then create a thread
 * that calls sensorThreadFunc().
 */
void ThermalSensorController::init()
{
    running = true;
    sensorThread = std::thread(&ThermalSensorController::sensorThreadFunc, this);
}



/* Created by: Chris Webb
 * Date: 7/6/14
 * Function: sensorThreadFunc()
 * Description:
 * Function is designed to be ran continuously within its own thread. Function creates
 * a SerialConn object to communicate to a device over a Serial Port then
 * synchronises device by sending a 255 byte and waiting for the device to return
 * 50 bytes back containing 255 and then sending 254 and if it receives 254 in return,
 * the device has been successfully synchronised. New sensor readings are atomically 
 * placed within a queue (readingQueue).
 */
void ThermalSensorController::sensorThreadFunc()
{
    std::cout << "Opening sensor device: " << deviceName << std::endl;

    // buffer of data to be written to device
    unsigned char data[255]; 
    // buffer of data read from device
    unsigned char buff[255];

    // Create a new Reading object to store the sensor data
    Reading newReading;
    newReading.img.create(4, 16);  // 4 rows 16 cols
    newReading.time = time(0);     // set time = to current time
    
    
    // Create a new serial connection to device
    SerialConn sc(deviceName, 115200);
    int readCount;

    // Start synchronisation process by sending a 255 byte
    data[0] = 255;
    int sent = sc.write((char*)&data, 1);
    assert(sent == 1);

    // Read bytes until 50 continuous 255 bytes have been read.
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

    // write 254 to sensor
    data[0] = 254;
    sent = sc.write((char*)&data, 1);
    assert(sent == 1);

    // if 254 is returned by sensor, synchronisation is complete
    do {
        readCount = sc.read((char*)&buff, 1);
        if(readCount > 0 && buff[0] == 254) {
            break;
        }
    } while(true);
    printf("synced!\n");

    // loop until controller has requested to stop
    while (running){
	bool waitingForOrientation = false;
        
        // read sentinal byte
        sc.read((char*)&buff, 1);
	//std::cout << "This byte should be 255!" << buff[0] << std::endl;
        assert(buff[0] == 255); 

        // read id byte representing what sensor is sending
        sc.read((char*)&buff, 1);
        unsigned char id = buff[0];

        // read length of data that sensor is going to send
        sc.read((char*)&buff, 2);
        unsigned short len = *((unsigned short*)buff);

        // read sensor data into buffer
        sc.read((char*)&buff, len);


        float ambientTemp = 0;
        
        // What we do with the recently read sensor data is defined by the
        // id of the reading
        switch(id) {
            case AMBIENT_TEMP_DATA: {
                std::cout << "Reading new Ambient temp data!" << std::endl;
                assert(len == sizeof(float));
                float ambient = *((float*)buff);
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
                    memcpy(imgDataPtr, buff, sizeof(float)*64);
		    waitingForOrientation = true; // not used
                    readingQueue.push(newReading);
                }
                break;
            };
	    case IMU_DATA: {
		std::cout << "Reading new IMU data!" << std::endl;
		assert(len == 3*sizeof(float));
                
		//float roll = *((float*)buff);
		float *imu = (float*)buff;
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
}



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
 */
void ThermalSensorController::shutdown() 
{
    running = false;
}





