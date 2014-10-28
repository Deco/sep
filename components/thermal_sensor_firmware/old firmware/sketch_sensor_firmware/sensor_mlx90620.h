//

#include "sensor.h"

#ifndef SENSOR_MLX90620_H
#define SENSOR_MLX90620_H

#define MLX90620_NODE_AMBIENT           0
#define MLX90620_NODE_THERMOPHILE_ARRAY 1

class MLX90620 : public Sensor {
public:
    MLX90620();
    
    // ?!?!?!?!?
    
    void init();
    void shutdown();

    void loop(unsigned int time);

    int getNodeCount();
    bool obtainNodeData(
        int nodeId,
        unsigned short &sizeRef, byte *&dataPtrRef
    );
    // ?!?!?!?!?

    void calibrate();

    int getSampleFreq();
    bool setSampleFreq(int f);

private:
    void varInitialization(byte calibration_data[]);
    void writeTrimmingValue(byte val);
    int readCPIX_MLX90620();
    unsigned int calculateAmbient();
    void setConfiguration(int irRefreshRateHZ);
    unsigned int readConfig_MLX90620();
    boolean checkConfig_MLX90620();
    
    void updateAmbientNode();
    void updatePixelNode();
    
    bool isAmbientDataNew;
    float ambientData[1];
    bool isPixelDataNew;
    float pixelData[64];
  
    int ticker;
    
    int irData[64]; //Contains the raw IR data from the sensor
    float temperatures[64]; //Contains the calculated temperatures of each pixel in the array
    byte eepromData[256]; //Contains the full EEPROM reading from the MLX (Slave 0x50)

    //These are constants calculated from the calibration data stored in EEPROM
    //See varInitialize and section 7.3 for more information
    int v_th, a_cp, b_cp, tgc, b_i_scale;
    float k_t1, k_t2, emissivity;
    float k_t2_wtf;
    int a_ij[64], b_ij[64];
};

#endif

