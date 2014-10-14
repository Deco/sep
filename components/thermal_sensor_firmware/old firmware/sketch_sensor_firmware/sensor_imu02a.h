//

#include "sensor.h"

#include <Wire.h>
#define I2C_BUS Wire

#ifndef SENSOR_IMU02A_H
#define SENSOR_IMU02A_H

#define IMU02A_NODE_GYROSCOPE 0

class IMU02A : public Sensor {
public:
    IMU02A();
    
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
    uint8_t gyro_i2caddr;
    int gyroOffsets[3];
    double gyroData[3];
    bool isGyroDataNew;

    void initGyroNode();

    void updateGyroNode();

    byte getGyroSampleRateDiv();

    void setGyroSampleRateDiv(byte _SampleRate);

    byte getGyroFSRange();

    void setGyroFSRange(byte _Range);

    byte getGyroFilterBW();

    void setGyroFilterBW(byte _BW);

    bool isGyroINTActiveOnLow();

    void setGyroINTLogiclvl(bool _State);

    bool isGyroINTOpenDrain();

    void setGyroINTDriveType(bool _State);

    bool isGyroLatchUntilCleared();

    void setGyroLatchMode(bool _State);

    bool isGyroAnyRegClrMode();

    void setGyroLatchClearMode(bool _State);

    bool isGyroITGReadyOn();

    void setGyroITGReady(bool _State);

    bool isGyroRawDataReadyOn();

    void setGyroRawDataReady(bool _State);

    bool isGyroITGReady();

    bool isGyroRawDataReady();

    void readGyroTemp(double *_Temp);

    void readGyroRaw(int *_GyroX, int *_GyroY, int *_GyroZ);

    void readGyroRaw( int *_GyroXYZ);

    void setGyroOffsets(int _Xoffset, int _Yoffset, int _Zoffset);

    void gyroZeroCalibrate(unsigned int totSamples, unsigned int sampleDelayMS);

    void readGyroRawCal(int *_GyroX, int *_GyroY, int *_GyroZ);

    void readGyroRawCal(int *_GyroXYZ);

    void readGyro(double *outputPtr);

    void resetGyro();

    bool isGyroLowPower();

    void setGyroPowerMode(bool _State);

    bool isGyroXStandby();

    bool isGyroYStandby();

    bool isGyroZStandby();

    void setGyroXStandby(bool _Status);

    void setGyroYStandby(bool _Status);

    void setGyroZStandby(bool _Status);

    byte getGyroClockSource();

    void setGyroClockSource(byte _CLKsource);


    void write_reg(uint8_t reg, uint8_t value);

    uint8_t read_reg(uint8_t reg);

    uint8_t read_bytes(uint8_t reg, uint8_t* buf, uint8_t num);


};

#endif
