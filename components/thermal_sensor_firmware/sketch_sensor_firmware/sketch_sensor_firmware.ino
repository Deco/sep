
#include "sensor.h"
#include "sensor_mlx90620.h"
#include "sensor_imu02a.h"

#include <Wire.h>

#include "firmware_registers.h"

#include "i2cmaster.h"

MLX90620 mlx;
IMU02A imu;

void outputSensorNodeData(byte code, Sensor &sensorRef, int nodeId);

void setup()
{
    Serial.begin(115200);
    
    //i2c_init();
    //delay(5);
    I2C_BUS.begin();
    //delay(5);

    //mlx.init();
    //delay(5);
    //mlx.calibrate();
    delay(5);

    imu.init();
    delay(5);
    imu.calibrate();
    delay(5);

}

void loop()
{
    while(!Serial) { /* spin */ };

    if(Serial.available() > 0) {
        byte cmd = Serial.read();
        if(cmd == 255) { // sync
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
    imu.loop(timeMs);

    outputSensorNodeData(COMM_ID_AMBIENT          , mlx, MLX90620_NODE_AMBIENT          );
    outputSensorNodeData(COMM_ID_THERMOPHILE_ARRAY, mlx, MLX90620_NODE_THERMOPHILE_ARRAY);
    outputSensorNodeData(COMM_ID_GYROSCOPE        , imu, IMU02A_NODE_GYROSCOPE          );
        
}

void outputSensorNodeData(byte code, Sensor &sensorRef, int nodeId)
{
    unsigned short dataSize; byte *dataPtr;
    /*if(sensorRef.obtainNodeData(nodeId, dataSize, dataPtr)) {
        Serial.write(255);
        Serial.write(code);
        Serial.write((byte*)&dataSize, sizeof(unsigned short));
        Serial.write(dataPtr, dataSize);
    }*/
}

