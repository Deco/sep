/*
Originally written by Declan with modifications by Chris to include
support for the gyro sensors.

IMU code sourced from Pololu (https://github.com/pololu/minimu-9-ahrs-arduino)
Accessed: 1/09/2014
Licence is GNU GENERAL PUBLIC LICENSE

*/


#include "sensor.h"
#include "sensor_mlx90620.h"
#include <L3G.h>
#include <LSM303.h>
#include "firmware_registers.h"
#include "i2cmaster.h"
#include <Wire.h>


// ************* Unchanged from MinIMU9AHRS.ino **************
// Uncomment the below line to use this axis definition: 
   // X axis pointing forward
   // Y axis pointing to the right 
   // and Z axis pointing down.
// Positive pitch : nose up
// Positive roll : right wing down
// Positive yaw : clockwise
int SENSOR_SIGN[9] = {1,1,1,-1,-1,-1,1,1,1}; //Correct directions x,y,z - gyro, accelerometer, magnetometer
// Uncomment the below line to use this axis definition: 
   // X axis pointing forward
   // Y axis pointing to the left 
   // and Z axis pointing up.
// Positive pitch : nose down
// Positive roll : right wing down
// Positive yaw : counterclockwise
//int SENSOR_SIGN[9] = {1,-1,-1,-1,1,1,1,-1,-1}; //Correct directions x,y,z - gyro, accelerometer, magnetometer

// tested with Arduino Uno with ATmega328 and Arduino Duemilanove with ATMega168



// LSM303 accelerometer: 8 g sensitivity
// 3.9 mg/digit; 1 g = 256
#define GRAVITY 256  //this equivalent to 1G in the raw data coming from the accelerometer 

#define ToRad(x) ((x)*0.01745329252)  // *pi/180
#define ToDeg(x) ((x)*57.2957795131)  // *180/pi

// L3G4200D gyro: 2000 dps full scale
// 70 mdps/digit; 1 dps = 0.07
#define Gyro_Gain_X 0.07 //X axis Gyro gain
#define Gyro_Gain_Y 0.07 //Y axis Gyro gain
#define Gyro_Gain_Z 0.07 //Z axis Gyro gain
#define Gyro_Scaled_X(x) ((x)*ToRad(Gyro_Gain_X)) //Return the scaled ADC raw data of the gyro in radians for second
#define Gyro_Scaled_Y(x) ((x)*ToRad(Gyro_Gain_Y)) //Return the scaled ADC raw data of the gyro in radians for second
#define Gyro_Scaled_Z(x) ((x)*ToRad(Gyro_Gain_Z)) //Return the scaled ADC raw data of the gyro in radians for second

// LSM303 magnetometer calibration constants; use the Calibrate example from
// the Pololu LSM303 library to find the right values for your board
#define M_X_MIN -672//-656//-421 
#define M_Y_MIN -721//-750//-639
#define M_Z_MIN -645//-659//-238
#define M_X_MAX 718//743//424
#define M_Y_MAX 668//678//295
#define M_Z_MAX 539//545//472

#define Kp_ROLLPITCH 0.02
#define Ki_ROLLPITCH 0.00002
#define Kp_YAW 1.2
#define Ki_YAW 0.00002

/*For debugging purposes*/
//OUTPUTMODE=1 will print the corrected data, 
//OUTPUTMODE=0 will print uncorrected data of the gyros (with drift)
#define OUTPUTMODE 1

//#define PRINT_DCM 0     //Will print the whole direction cosine matrix
#define PRINT_ANALOGS 0 //Will print the analog raw data
#define PRINT_EULER 1   //Will print the Euler angles Roll, Pitch and Yaw

#define STATUS_LED 13 

float G_Dt=0.02;    // Integration time (DCM algorithm)  We will run the integration loop at 50Hz if possible

long timer=0;   //general purpuse timer
long timer_old;
long timer24=0; //Second timer used to print values 
int AN[6]; //array that stores the gyro and accelerometer data
int AN_OFFSET[6]={0,0,0,0,0,0}; //Array that stores the Offset of the sensors

int gyro_x;
int gyro_y;
int gyro_z;
int accel_x;
int accel_y;
int accel_z;
int magnetom_x;
int magnetom_y;
int magnetom_z;
float c_magnetom_x;
float c_magnetom_y;
float c_magnetom_z;
float MAG_Heading;

float Accel_Vector[3]= {0,0,0}; //Store the acceleration in a vector
float Gyro_Vector[3]= {0,0,0};//Store the gyros turn rate in a vector
float Omega_Vector[3]= {0,0,0}; //Corrected Gyro_Vector data
float Omega_P[3]= {0,0,0};//Omega Proportional correction
float Omega_I[3]= {0,0,0};//Omega Integrator
float Omega[3]= {0,0,0};

// Euler angles
float roll;
float pitch;
float yaw;

float errorRollPitch[3]= {0,0,0}; 
float errorYaw[3]= {0,0,0};

unsigned int counter=0;
byte gyro_sat=0;

float DCM_Matrix[3][3]= {
  {
    1,0,0  }
  ,{
    0,1,0  }
  ,{
    0,0,1  }
}; 
float Update_Matrix[3][3]={{0,1,2},{3,4,5},{6,7,8}}; //Gyros here


float Temporary_Matrix[3][3]={
  {
    0,0,0  }
  ,{
    0,0,0  }
  ,{
    0,0,0  }
};
// ************* Unchanged from MinIMU9AHRS.ino **************




// Sensors on the arduino
MLX90620 mlx;
// TODO: make gyro and compass inherit from our Sensor class to make code easier to manage in the future


void outputMLXSensorNodeData(byte code, Sensor &sensorRef, int nodeId);
void outputIMUdata();
void imuSetup();
void imuLoop();


void setup()
{
    Serial.begin(115200);
    
	// Setup i2c connection for MLX using twimaster.cpp file
	// 		note: MLX sensor can't be connected using the wire.h library (as far as I know searching the internet)
    i2c_init();
    delay(5);
    mlx.init();
    mlx.calibrate();
	
	// Setup i2c connection for gyro and compass using wire.h and initilise sensors.
    imuSetup();
}

// Extended setup function to encapsulate the setup of the IMU. Written by Pololu
void imuSetup()
{
    pinMode (STATUS_LED,OUTPUT);  // Status LED
  
    I2C_Init();

    digitalWrite(STATUS_LED,LOW);
    delay(1500);
 
	// Initilise all sensors
    Accel_Init();
    Compass_Init();
    Gyro_Init();
  
    delay(20);
   
    // Calibration ****   
    for(int i=0;i<32;i++)    // We take some readings...
      {
      Read_Gyro();
      Read_Accel();
      for(int y=0; y<6; y++)   // Cumulate values
        AN_OFFSET[y] += AN[y];
      delay(20);
      }
    
    for(int y=0; y<6; y++)
      AN_OFFSET[y] = AN_OFFSET[y]/32;
    
    AN_OFFSET[5]-=GRAVITY*SENSOR_SIGN[5];
    // Calibration ****  
	
	/* Enable this to print offset values
    Serial.println("Offset:");
    for(int y=0; y<6; y++)
    Serial.println(AN_OFFSET[y]);
	*/
  
    delay(2000);
    digitalWrite(STATUS_LED,HIGH);
    
    timer=millis();
    delay(20);
    counter=0;

}


void loop()
{
	// Synchronisation of Arduino written by Declan
    while(!Serial) { /* spin */ };
    
    if(Serial.available() > 0) {
      byte cmd = Serial.read();
      if(cmd == 255) {
        byte in = 255;
        do {
          while(!Serial) { /* spin */ }
          Serial.write(255);
          if(Serial.available() > 0) {
            in = Serial.read();
          }
        } while(in != 254);
        Serial.write(254);
      }
    }
    
    unsigned long timeMs = millis(); // TODO: Handle overflow? (50 days)

	
	
    mlx.loop(timeMs);
	
    outputMLXSensorNodeData(COMM_ID_AMBIENT          , mlx, MLX90620_NODE_AMBIENT          );
    outputMLXSensorNodeData(COMM_ID_THERMOPHILE_ARRAY, mlx, MLX90620_NODE_THERMOPHILE_ARRAY);
	// hardcoded output(MLX)SensorNodeData as L3M and LGD do not inherit from our Sensor class
	
	imuLoop();
	
}

// Outputs sensor data to Serial. Written by Declan
// TODO: Use this function to output IMU data, requires modification of L3G and LSM303 files
void outputMLXSensorNodeData(byte code, Sensor &sensorRef, int nodeId)
{
    unsigned short dataSize; byte *dataPtr;
    if(sensorRef.obtainNodeData(nodeId, dataSize, dataPtr)) {
        Serial.write(255);
        Serial.write(code);
        Serial.write((byte*)&dataSize, sizeof(unsigned short));
        Serial.write(dataPtr, dataSize);
	//Serial.print("printing some MLX data\n");
        //Serial.write(dataPtr, dataSize);
    }
}

// Used to write IMU data to serial for now. Written by Chris
void outputIMUdata(){
  
  float data[3];
  data[0] = (float)ToDeg(roll);
  data[1] = (float)ToDeg(pitch);
  data[2] = (float)ToDeg(yaw);
  
  // Output order of data for Sensor Controller:
  // Sentinal value -> ID of sensor -> size of data to send -> data
  // example: 255 -> 0x13 -> (length,1) -> (data, length)
  
  Serial.write(255);
  Serial.write(COMM_ID_GYROSCOPE);
  
  unsigned short dataSize; byte *dataPtr;
  dataSize = 3 * sizeof(float);
  dataPtr = (byte*)&data;
  
  Serial.write((byte*)&dataSize, sizeof(unsigned short));
  Serial.write(dataPtr, dataSize);
  
}

// Loop to get sensor data and calculate roll, pitch and yaw. 
// Extended main loop to seperate IMU interaction from MLX. 
// Written by Pololu.
void imuLoop(){
  if((millis()-timer)>=20)  // Main loop runs at 50Hz
  {
    counter++;
    timer_old = timer;
    timer=millis();
    if (timer>timer_old)
      G_Dt = (timer-timer_old)/1000.0;    // Real time of loop run. We use this on the DCM algorithm (gyro integration time)
    else
      G_Dt = 0;
    
    // *** DCM algorithm
    // Data adquisition
    Read_Gyro();   // This read gyro data
    Read_Accel();     // Read I2C accelerometer
    
    if (counter > 5)  // Read compass data at 10Hz... (5 loop runs)
      {
      counter=0;
      Read_Compass();    // Read I2C magnetometer
      Compass_Heading(); // Calculate magnetic heading  
      }
    
    // Calculations...
    Matrix_update(); 
    Normalize();
    Drift_correction();
    Euler_angles();
    // ***
   
  // Serial.print("printing some IMU data\n");
  outputIMUdata();
 //   printdata();
  }

}

