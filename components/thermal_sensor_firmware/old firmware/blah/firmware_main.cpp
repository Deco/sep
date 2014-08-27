
#include <Wire.h>

#include "sensor.h"
#include "sensor_mlx90620.h"

#include "firmware_registers.h"

#include "i2cmaster.h"

MLX90620 mlx;

void outputSensorNodeData(byte code, Sensor &sensorRef, int nodeId);

void setup()
{
    Serial.begin(115200);
    
    i2c_init();

    delay(5);

    mlx.init();
    mlx.calibrate();
}

void loop()
{
    unsigned long timeMs = millis(); // TODO: Handle overflow? (50 days)

    mlx.loop(timeMs);

    outputSensorNodeData(COMM_ID_AMBIENT          , mlx, MLX90620_NODE_AMBIENT          );
    outputSensorNodeData(COMM_ID_THERMOPHILE_ARRAY, mlx, MLX90620_NODE_THERMOPHILE_ARRAY);
    
}

void outputSensorNodeData(byte code, Sensor &sensorRef, int nodeId)
{
    unsigned short dataSize; byte *dataPtr;
    if(sensorRef.obtainNodeData(nodeId, dataSize, dataPtr)) {
        Serial.write(255);
        Serial.write(code);
        Serial.write(&dataSize, sizeof(unsigned short));
        Serial.write(dataPtr, dataSize);
    }
}
