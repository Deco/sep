
#include "sensor_mlx90620.h"

#include "MLX90620_registers.h"

#include "i2cmaster.h"

MLX90620::MLX90620()
{
    // 
}

void MLX90620::init()
{
    // 
}
void MLX90620::shutdown()
{
    // 
}

void MLX90620::calibrate()
{
    i2c_start_wait(MLX90620_EEPROM_WRITE);
    i2c_write(0x00); //EEPROM info starts at location 0x00
    i2c_rep_start(MLX90620_EEPROM_READ);

    //Read all 256 bytes from the sensor's EEPROM
    for(int i = 0 ; i <= 255 ; i++) {
        eepromData[i] = i2c_readAck();
    }

    i2c_stop(); //We're done talking

    varInitialization(eepromData); //Calculate a bunch of constants from the EEPROM data

    writeTrimmingValue(eepromData[OSC_TRIM_VALUE]);
}

void MLX90620::varInitialization()
{
    v_th = 256 * calibration_data[VTH_H] + calibration_data[VTH_L];
    k_t1 = (256 * calibration_data[KT1_H] + calibration_data[KT1_L]) / 1024.0; //2^10 = 1024
    k_t2 = (256 * calibration_data[KT2_H] + calibration_data[KT2_L]) / 1048576.0; //2^20 = 1,048,576
    emissivity = ((unsigned int)256 * calibration_data[CAL_EMIS_H] + calibration_data[CAL_EMIS_L]) / 32768.0;

    a_cp = calibration_data[CAL_ACP];
    if(a_cp > 127) a_cp -= 256; //These values are stored as 2's compliment. This coverts it if necessary.

    b_cp = calibration_data[CAL_BCP];
    if(b_cp > 127) b_cp -= 256;

    tgc = calibration_data[CAL_TGC];
    if(tgc > 127) tgc -= 256;

    b_i_scale = calibration_data[CAL_BI_SCALE];

    for(int i = 0 ; i < 64 ; i++) {
        //Read the individual pixel offsets
        a_ij[i] = calibration_data[i]; 
        if(a_ij[i] > 127) a_ij[i] -= 256; //These values are stored as 2's compliment. This coverts it if necessary.

        //Read the individual pixel offset slope coefficients
        b_ij[i] = calibration_data[0x40 + i]; //Bi(i,j) begins 64 bytes into EEPROM at 0x40
        if(b_ij[i] > 127) b_ij[i] -= 256;
    }
}

void MLX90620::writeTrimmingValue(byte val)
{
    i2c_start_wait(MLX90620_WRITE); //Write to the sensor
    i2c_write(0x04); //Command = write oscillator trimming value
    i2c_write((byte)val - 0xAA);
    i2c_write(val);
    i2c_write(0x56); //Always 0x56
    i2c_write(0x00); //Always 0x00
}

void MLX90620::updateAmbientNode()
{
    i2c_start_wait(MLX90620_WRITE);
    i2c_write(CMD_READ_REGISTER); //Command = read PTAT
    i2c_write(0x90); //Start address is 0x90
    i2c_write(0x00); //Address step is 0
    i2c_write(0x01); //Number of reads is 1
    i2c_rep_start(MLX90620_READ);

    byte ptatLow = i2c_readAck(); //Grab the lower and higher PTAT bytes
    byte ptatHigh = i2c_readAck();

    i2c_stop();

    unsigned int ptat = ( (unsigned int)(ptatHigh << 8) | ptatLow); //Combine bytes and return
    ambientData[0] = (-k_t1 + sqrt(square(k_t1) - (4 * k_t2 * (v_th - (float)ptat)))) / (2*k_t2) + 25; //it's much more simple now, isn't it? :)
    
    isAmbientDataNew = true;
}

void MLX90620::updatePixelNode()
{
    i2c_start_wait(MLX90620_WRITE);
    i2c_write(CMD_READ_REGISTER); //Command = read a register
    i2c_write(0x00); //Start address = 0x00
    i2c_write(0x01); //Address step = 1
    i2c_write(0x40); //Number of reads is 64
    i2c_rep_start(MLX90620_READ);

    for(int i = 0 ; i < 64 ; i++) {
        byte pixelDataLow = i2c_readAck();
        byte pixelDataHigh = i2c_readAck();
        irData[i] = (int)(pixelDataHigh << 8) | pixelDataLow;
    }

    i2c_stop();
    
    float v_ir_off_comp;
    float v_ir_tgc_comp;
    float v_ir_comp;

    //Calculate the offset compensation for the one compensation pixel
    //This is a constant in the TO calculation, so calculate it here.
    int cpix = readCPIX_MLX90620(); //Go get the raw data of the compensation pixel
    float v_cp_off_comp = (float)cpix - (a_cp + (b_cp/pow(2, b_i_scale)) * (Tambient - 25)); 

    for (int i = 0 ; i < 64 ; i++) {
        v_ir_off_comp = irData[i] - (a_ij[i] + (float)(b_ij[i]/pow(2, b_i_scale)) * (Tambient - 25)); //#1: Calculate Offset Compensation 

        v_ir_tgc_comp = v_ir_off_comp - ( ((float)tgc/32) * v_cp_off_comp); //#2: Calculate Thermal Gradien Compensation (TGC)

        v_ir_comp = v_ir_tgc_comp / emissivity; //#3: Calculate Emissivity Compensation

        temperatures[i] = sqrt( sqrt( (v_ir_comp/alpha_ij[i]) + pow(Tambient + 273.15, 4) )) - 273.15;
    }
    isPixelDataNew = true;
}

void MLX90620::loop(unsigned int time)
{
    if(time > nextAmbientUpdateTime) {
        nextAmbientUpdateTime = time+ambientUpdateInterval;
        updateAmbientNode();
    }
    updatePixelNode();
}

int MLX90620::getNodeCount()
{
    return 2;
}
bool MLX90620::obtainNodeData(
    int nodeId,
    unsigned short &sizeRef, byte *&dataPtrRef
) {
    bool isNewData = false;
    switch(nodeId) {
        case MLX90620_NODE_AMBIENT: {
            sizeRef = sizeof(float);
            dataPtrRef = &ambientData;

            isNewData = isAmbientDataNew;
            isAmbientDataNew = false;
            break;
        }
        case MLX90620_NODE_THERMOPHILE_ARRAY: {
            sizeRef = 64*sizeof(float);
            dataPtrRef = &pixelData;

            isNewData = isPixelDataNew;
            isPixelDataNew = false;
            break;
        }
    }
    return isNewData;
}

int MLX90620::getSampleFreq()
{
    // 
}
bool MLX90620::setSampleFreq(int f)
{
    // 
}
