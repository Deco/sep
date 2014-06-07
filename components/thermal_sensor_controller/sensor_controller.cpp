#include "sensor_controller.h"
#include "serialconn.h"
#include <mutex>




ThermalSensorController::ThermalSensorController(
    const std::string _deviceName, unsigned int _baudRate
)   : deviceName(_deviceName), deviceBaudRate(_baudRate), readingQueue()
{
    //
}

/* Start a thread that reads data from sensor pushes to queue */
void ThermalSensorController::init()
{
    sensorThread = std::thread(&ThermalSensorController::sensorThreadFunc, this);
}

/* This func is ran by a thread to read data and put in queue */
void ThermalSensorController::sensorThreadFunc()
{

union float2bytes { float f; char b[sizeof(float)]; };

float2bytes f2b;
unsigned char buff[255];
    /*
    float a[] = { 1,  2,  3,  4,
               5,  6,  7,  8,
               9, 10, 11, 12 };
    CvMat mat = cvMat( 3, 4, CV_32FC1, a ); // 32FC1 for float
*/
  //  img.create(20, 20);
  int count = 10;
  while(count >= 0){
    Reading newReading;
    newReading.img.create(4, 16);  // 4 rows 16 cols
    newReading.time = 50;
    
    
  //  cv::Mat M = cvCreateMat( 4, 16, CV_32FC1 );
   // M->data.fl[ 0 * M->cols + 0 ] = (float)3.0;
    cv::Mat newMat;
    newMat.create(4,16, CV_32FC1);
    
    float data[4][16];

    SerialConn sc("/dev/ttyACM0", 115200);

    int count=0;
    for (int i=0;i<4;i++) {
        for (int j=0;j<16;j++) {
          for (int k=0;k<sizeof(float);k++) {
                int readCount = sc.read((char*)&buff, 1);
                f2b.b[k] = buff[0];
             //   printf("\treading byte from sensor: %u\n",buff[0] );
                data[i][j] = f2b.f;
            }
            count++;
          //  printf("Just read float num %d: %f\n",count , data[i][j]);
        }
    }




newReading.img = cv::Mat( 4, 16, CV_32FC1, data );
//cv::Mat mat = cv::Mat( 4, 16, CV_32FC1, data );


        /* DEC CODE BOX EXAMPLE
        switch(id) {
    case 16: {
        
        if(len != 4*64) {
            // problems!
        }
        
        float data[64];
        sport.read((char*)&data, sizeof(float)*64);
        
        img.create(4, 16);
        float *imgDataPtr = (float*)img.data;
        
        memcpy(imgDataPtr, data, sizeof(float)*64);
        
        break;
    }
        
        */
        
        
        
        
 //       if(readCount > 0) {
       // newReading.img.at<unsigned char>(0, 0) = buff[0];

        
        
    //    printf("%d:byte: %x\n", (int)buff[0]);
        
     //   newReading.img.push_back(buff);
     
        readingQueue.push(newReading);
           
 //           count++;
 //       }
 //   }
     
    sc.close();
    count--;

}

    /*
     while true do
        try
            open serial port using deviceName and deviceBaudRate  
            "synchronise"
            
            while true do
                read bytes
                handle data
                if(1) {
                    std::lock readingQueueLock(readingQueueMutex);
                    push to readingQueue
                }
            end
        catch errors
            handle errors
        end
    end*/
   // while(1) {
  //      std::cout << "thread" << std::endl;
  //  }
}

bool ThermalSensorController::popThermopileReading(cv::Mat &matRef, double &timeRef)
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