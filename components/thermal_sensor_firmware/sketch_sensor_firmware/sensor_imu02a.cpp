
#include "sensor_IMU02A.h"

#include "IMU02A_registers.h"

#include <Wire.h>
#define I2C_BUS Wire

IMU02A::IMU02A()
{
    gyro_i2caddr = ITG3200_ADDR_AD0_LOW;
    setGyroOffsets(0,0,0);
}

void IMU02A::init()
{
    initGyroNode();
}

void IMU02A::shutdown()
{
    // 
}

void IMU02A::calibrate()
{
    // 
}

void IMU02A::loop(unsigned int time)
{
    updateGyroNode();
}

int IMU02A::getNodeCount()
{
    return 1;
}
bool IMU02A::obtainNodeData(
    int nodeId,
    unsigned short &sizeRef, byte *&dataPtrRef
) {
    bool isNewData = false;
    switch(nodeId) {
        case IMU02A_NODE_GYROSCOPE: {
            sizeRef = 3*sizeof(double);
            dataPtrRef = (byte*)gyroData;

            isNewData = isGyroDataNew;
            isGyroDataNew = false;
            break;
        }
    }
    return isNewData;
}


void IMU02A::initGyroNode() 
{
    byte _SRateDiv = NOSRDIVIDER;
    byte _Range = RANGE2000;
    byte _filterBW = BW256_SR8;
    byte _ClockSrc = PLL_ZGYRO_REF;
    bool _ITGReady = true;
    bool _INTRawDataReady = true;
    
    setGyroSampleRateDiv(_SRateDiv);
    setGyroFSRange(_Range);
    setGyroFilterBW(_filterBW);
    setGyroClockSource(_ClockSrc);
    setGyroITGReady(_ITGReady);
    setGyroRawDataReady(_INTRawDataReady);
    delay(GYROSTART_UP_DELAY);
}

void IMU02A::updateGyroNode()
{
    int rawGyroData[3];
    readGyroRaw(rawGyroData);

    for(int i = 0; i < 3; i++) {
        gyroData[i] = (double)rawGyroData[i];
        Serial.print("x:");
        Serial.print(rawGyroData[i]);
        Serial.print("; y:");
        Serial.print(rawGyroData[i]);
        Serial.print("; z:");
        Serial.print(rawGyroData[i]);
        Serial.print("\n");
    }

    isGyroDataNew = true;
}

byte IMU02A::getGyroSampleRateDiv() 
{
    return read_reg(SMPLRT_DIV);
}

void IMU02A::setGyroSampleRateDiv(byte _SampleRate) 
{
    write_reg(SMPLRT_DIV, _SampleRate);
}

byte IMU02A::getGyroFSRange() 
{
    return ((read_reg(DLPF_FS) & DLPFFS_FS_SEL) >> 3);
}

void IMU02A::setGyroFSRange(byte _Range) 
{
    write_reg(DLPF_FS, ((read_reg(DLPF_FS) & ~DLPFFS_FS_SEL) | (_Range << 3)) );
}

byte IMU02A::getGyroFilterBW() 
{
    return (read_reg(DLPF_FS) & DLPFFS_DLPF_CFG);
}

void IMU02A::setGyroFilterBW(byte _BW)
{
    write_reg(DLPF_FS, ((read_reg(DLPF_FS) & ~DLPFFS_DLPF_CFG) | _BW));
}

bool IMU02A::isGyroINTActiveOnLow() 
{
    return ((read_reg(INT_CFG) & INTCFG_ACTL) >> 7);
}

void IMU02A::setGyroINTLogiclvl(bool _State)
{
    write_reg(INT_CFG, ((read_reg(INT_CFG) & ~INTCFG_ACTL) | (_State << 7)));
}

bool IMU02A::isGyroINTOpenDrain()
{
    return ((read_reg(INT_CFG) & INTCFG_OPEN) >> 6);
}

void IMU02A::setGyroINTDriveType(bool _State)
{
    write_reg(INT_CFG, ((read_reg(INT_CFG) & ~INTCFG_OPEN) | _State << 6));
}

bool IMU02A::isGyroLatchUntilCleared() 
{
    return ((read_reg(INT_CFG) & INTCFG_LATCH_INT_EN) >> 5);
}

void IMU02A::setGyroLatchMode(bool _State)
{
    write_reg(INT_CFG, ((read_reg(INT_CFG) & ~INTCFG_LATCH_INT_EN) | _State << 5));
}

bool IMU02A::isGyroAnyRegClrMode() 
{
    return ((read_reg(INT_CFG) & INTCFG_INT_ANYRD_2CLEAR) >> 4);
}

void IMU02A::setGyroLatchClearMode(bool _State) 
{
    write_reg(INT_CFG, ((read_reg(INT_CFG) & ~INTCFG_INT_ANYRD_2CLEAR) | _State << 4));
}

bool IMU02A::isGyroITGReadyOn()
{
    return ((read_reg(INT_CFG) & INTCFG_ITG_RDY_EN) >> 2);
}

void IMU02A::setGyroITGReady(bool _State)
{
    write_reg(INT_CFG, ((read_reg(INT_CFG) & ~INTCFG_ITG_RDY_EN) | _State << 2));
}

bool IMU02A::isGyroRawDataReadyOn() 
{
    uint8_t r = read_reg(INT_CFG);
    return (r & INTCFG_RAW_RDY_EN);
}

void IMU02A::setGyroRawDataReady(bool _State) 
{
    write_reg(INT_CFG, ((read_reg(INT_CFG) & ~INTCFG_RAW_RDY_EN) | _State));
}

bool IMU02A::isGyroITGReady() 
{
    return ((read_reg(INT_STATUS) & INTSTATUS_ITG_RDY) >> 2);
}

bool IMU02A::isGyroRawDataReady() 
{
    return (read_reg(INT_STATUS) & INTSTATUS_RAW_DATA_RDY);
}

void IMU02A::readGyroTemp(double *_Temp) 
{
    uint8_t buf[2];
    read_bytes(TEMP_OUT, buf, 2);
    *_Temp = 35 + ((buf[0] << 8 | buf[1]) + 13200) / 280.0;    // F=C*9/5+32
}

void IMU02A::readGyroRaw(int *_GyroX, int *_GyroY, int *_GyroZ) 
{
    int8_t _buff[6];
    read_bytes(GYRO_XOUT, (uint8_t*)_buff, 6);

    int16_t x, y, z;
    x = _buff[0] << 8 | _buff[1];
    y = _buff[2] << 8 | _buff[3];
    z = _buff[4] << 8 | _buff[5];

    *_GyroX = x;
    *_GyroY = y;
    *_GyroZ = z;
}

void IMU02A::readGyroRaw( int *_GyroXYZ) 
{
    readGyroRaw(_GyroXYZ, _GyroXYZ+1, _GyroXYZ+2);
}

void IMU02A::setGyroOffsets(int _Xoffset, int _Yoffset, int _Zoffset) 
{
    gyroOffsets[0] = _Xoffset;
    gyroOffsets[1] = _Yoffset;
    gyroOffsets[2] = _Zoffset;
}

void IMU02A::gyroZeroCalibrate(unsigned int totSamples, unsigned int sampleDelayMS) 
{
    double tmpOffsets[] = {0,0,0};
    int xyz[3];

    for (unsigned int i = 0; i < totSamples; i++) 
    {
        delay(sampleDelayMS);
        readGyroRaw(xyz);
        tmpOffsets[0] += xyz[0];
        tmpOffsets[1] += xyz[1];
        tmpOffsets[2] += xyz[2];
    }
    setGyroOffsets(
        -tmpOffsets[0] / totSamples + 0.5,
        -tmpOffsets[1] / totSamples + 0.5,
        -tmpOffsets[2] / totSamples + 0.5
    );
}

void IMU02A::readGyroRawCal(int *_GyroX, int *_GyroY, int *_GyroZ) 
{
    readGyroRaw(_GyroX, _GyroY, _GyroZ);
    *_GyroX += gyroOffsets[0];
    *_GyroY += gyroOffsets[1];
    *_GyroZ += gyroOffsets[2];
}

void IMU02A::readGyroRawCal(int *_GyroXYZ) 
{
    readGyroRawCal(_GyroXYZ, _GyroXYZ+1, _GyroXYZ+2);
}

void IMU02A::readGyro(double *outputPtr)
{
    int x, y, z;
    readGyroRawCal(&x, &y, &z);
    outputPtr[0] = x / 14.375;
    outputPtr[1] = y / 14.375;
    outputPtr[2] = z / 14.375;
}

void IMU02A::resetGyro() 
{
    write_reg(PWR_MGM, PWRMGM_HRESET);
    delay(GYROSTART_UP_DELAY); //gyro startup
}

bool IMU02A::isGyroLowPower() 
{
    return (read_reg(PWR_MGM) & PWRMGM_SLEEP) >> 6;
}

void IMU02A::setGyroPowerMode(bool _State) 
{
    write_reg(PWR_MGM, ((read_reg(PWR_MGM) & ~PWRMGM_SLEEP) | _State << 6));
}

bool IMU02A::isGyroXStandby() 
{
    return (read_reg(PWR_MGM) & PWRMGM_STBY_XG) >> 5;
}

bool IMU02A::isGyroYStandby()
{
    return (read_reg(PWR_MGM) & PWRMGM_STBY_YG) >> 4;
}

bool IMU02A::isGyroZStandby()
{
    return (read_reg(PWR_MGM) & PWRMGM_STBY_ZG) >> 3;
}

void IMU02A::setGyroXStandby(bool _Status)
{
    write_reg(PWR_MGM, ((read_reg(PWR_MGM) & PWRMGM_STBY_XG) | _Status << 5));
}

void IMU02A::setGyroYStandby(bool _Status)
{
    write_reg(PWR_MGM, ((read_reg(PWR_MGM) & PWRMGM_STBY_YG) | _Status << 4));
}

void IMU02A::setGyroZStandby(bool _Status)
{
    write_reg(PWR_MGM, ((read_reg(PWR_MGM) & PWRMGM_STBY_ZG) | _Status << 3));
}

byte IMU02A::getGyroClockSource()
{
    return (read_reg(PWR_MGM) & PWRMGM_CLK_SEL);
}

void IMU02A::setGyroClockSource(byte _CLKsource)
{
    write_reg(PWR_MGM, ((read_reg(PWR_MGM) & ~PWRMGM_CLK_SEL) | _CLKsource));
}


void IMU02A::write_reg(uint8_t reg, uint8_t value)
{
    I2C_BUS.beginTransmission(gyro_i2caddr);
    I2C_BUS.write(reg);
    I2C_BUS.write(value);
    I2C_BUS.endTransmission();
}

uint8_t IMU02A::read_reg(uint8_t reg)
{
    uint8_t value;

    I2C_BUS.beginTransmission(gyro_i2caddr);
    I2C_BUS.write(reg);
    I2C_BUS.endTransmission();
    I2C_BUS.requestFrom(gyro_i2caddr, (uint8_t)1);
    value = I2C_BUS.read();
    I2C_BUS.endTransmission();

    return value;
}

uint8_t IMU02A::read_bytes(uint8_t reg, uint8_t* buf, uint8_t num)
{
    I2C_BUS.beginTransmission(gyro_i2caddr);
    I2C_BUS.write(reg);
    I2C_BUS.endTransmission();
    I2C_BUS.requestFrom(gyro_i2caddr, (uint8_t)num);

    for(int i = 0; i < num; i++)
        buf[i] = I2C_BUS.read();
}


