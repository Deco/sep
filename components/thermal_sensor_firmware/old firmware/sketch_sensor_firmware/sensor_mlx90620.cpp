
#include "sensor_mlx90620.h"

#include "MLX90620_registers.h"

#include "i2cmaster.h"

float alpha_ij[64] = {
  1.53768E-8, 1.60753E-8, 1.70066E-8, 1.46783E-8, 1.84036E-8, 1.93932E-8, 1.93349E-8, 1.67738E-8,
  1.95678E-8, 2.14886E-8, 2.10230E-8, 1.88693E-8, 2.14304E-8, 2.32931E-8, 2.25364E-8, 2.00916E-8,
  2.35841E-8, 2.46901E-8, 2.46901E-8, 2.14886E-8, 2.47483E-8, 2.66109E-8, 2.63199E-8, 2.30602E-8,
  2.56214E-8, 2.82407E-8, 2.75422E-8, 2.39916E-8, 2.63199E-8, 2.81825E-8, 2.80079E-8, 2.42244E-8,
  2.60870E-8, 2.84736E-8, 2.77169E-8, 2.43990E-8, 2.63199E-8, 2.84153E-8, 2.79497E-8, 2.43990E-8,
  2.64363E-8, 2.82989E-8, 2.81825E-8, 2.43990E-8, 2.60870E-8, 2.81825E-8, 2.70184E-8, 2.41662E-8,
  2.56214E-8, 2.72512E-8, 2.64945E-8, 2.34677E-8, 2.43990E-8, 2.57960E-8, 2.53303E-8, 2.25364E-8,
  2.32349E-8, 2.70184E-8, 2.43990E-8, 2.18379E-8, 2.16633E-8, 2.33513E-8, 2.37005E-8, 2.06737E-8,
};

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
  
  setConfiguration(16);
}

void MLX90620::varInitialization(byte calibration_data[])
{
  v_th = 256 * calibration_data[VTH_H] + calibration_data[VTH_L];
  k_t1 = (256 * calibration_data[KT1_H] + calibration_data[KT1_L]) / 1024.0; //2^10 = 1024
  k_t2 = (256 * calibration_data[KT2_H] + calibration_data[KT2_L]) / 1048576.0; //2^20 = 1,048,576
  k_t2_wtf = 4 * k_t2;
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

unsigned int MLX90620::calculateAmbient()
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

  return ( (unsigned int)(ptatHigh << 8) | ptatLow); //Combine bytes and return
}

void MLX90620::updateAmbientNode()
{
  unsigned int ptat = calculateAmbient();
  
  float k_t1_square = square(k_t1);
//  Serial.print("ad: ");
//  Serial.print(ambientData[0]);
//  Serial.print("\n");
  ambientData[0] = (-k_t1 + sqrt(square(k_t1) - (k_t2_wtf * (v_th - (float)ptat)))) / (2*k_t2) + 25; //it's much more simple now, isn't it? :)
  
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
  float v_cp_off_comp = (float)cpix - (a_cp + (b_cp/pow(2, b_i_scale)) * (ambientData[0] - 25)); 

  for (int i = 0 ; i < 64 ; i++) {
    v_ir_off_comp = irData[i] - (a_ij[i] + (float)(b_ij[i]/pow(2, b_i_scale)) * (ambientData[0] - 25)); //#1: Calculate Offset Compensation 

    v_ir_tgc_comp = v_ir_off_comp - ( ((float)tgc/32) * v_cp_off_comp); //#2: Calculate Thermal Gradien Compensation (TGC)

    v_ir_comp = v_ir_tgc_comp / emissivity; //#3: Calculate Emissivity Compensation

    pixelData[i] = sqrt( sqrt( (v_ir_comp/alpha_ij[i]) + pow(ambientData[0] + 273.15, 4) )) - 273.15;
  }
  isPixelDataNew = true;
}
int MLX90620::readCPIX_MLX90620()
{
  i2c_start_wait(MLX90620_WRITE);
  i2c_write(CMD_READ_REGISTER); //Command = read register
  i2c_write(0x91);
  i2c_write(0x00);
  i2c_write(0x01);
  i2c_rep_start(MLX90620_READ);

  byte cpixLow = i2c_readAck(); //Grab the two bytes
  byte cpixHigh = i2c_readAck();
  i2c_stop();

  return ( (int)(cpixHigh << 8) | cpixLow);
}

void MLX90620::loop(unsigned int time)
{
  //if(time > nextAmbientUpdateTime) {
  //nextAmbientUpdateTime = time+ambientUpdateInterval;
  //Serial.print("ticker: ");
//  Serial.print(ticker, DEC);
//  Serial.print("\n");
  if(ticker == 16) {
    ticker = 0;
    updateAmbientNode();
    if(checkConfig_MLX90620()) {
      //Serial.println("POR Detected!");
      setConfiguration(16); //Re-write the configuration bytes to the MLX
    }
  }
  ticker++;
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
  case MLX90620_NODE_AMBIENT: 
    {
      sizeRef = sizeof(float);
      dataPtrRef = (byte*)ambientData;

      isNewData = isAmbientDataNew;
      isAmbientDataNew = false;
      break;
    }
  case MLX90620_NODE_THERMOPHILE_ARRAY: 
    {
      sizeRef = 64*sizeof(float);
      dataPtrRef = (byte*)pixelData;

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

void MLX90620::setConfiguration(int irRefreshRateHZ)
{
  byte Hz_LSB;

  switch(irRefreshRateHZ)
  {
  case 0:
    Hz_LSB = 0b00001111;
    break;
  case 1:
    Hz_LSB = 0b00001110;
    break;
  case 2:
    Hz_LSB = 0b00001101;
    break;
  case 4:
    Hz_LSB = 0b00001100;
    break;
  case 8:
    Hz_LSB = 0b00001011;
    break;
  case 16:
    Hz_LSB = 0b00001010;
    break;
  case 32:
    Hz_LSB = 0b00001001;
    break;
  default:
    Hz_LSB = 0b00001110;
  }

//------------------------------------------------------
  byte defaultConfig_H = 0b01110100; // x111.01xx, Assumes NA = 0, ADC low reference enabled, Ta Refresh rate of 2Hz

  i2c_start_wait(MLX90620_WRITE);
  i2c_write(0x03); //Command = configuration value
  i2c_write((byte)Hz_LSB - 0x55);
  i2c_write(Hz_LSB);
  i2c_write(defaultConfig_H - 0x55); //Assumes NA = 0, ADC low reference enabled, Ta Refresh rate of 2Hz
  i2c_write(defaultConfig_H);
  i2c_stop();
}

unsigned int MLX90620::readConfig_MLX90620()
{
  i2c_start_wait(MLX90620_WRITE); //The MLX configuration is in the MLX, not EEPROM
  i2c_write(CMD_READ_REGISTER); //Command = read configuration register
  i2c_write(0x92); //Start address
  i2c_write(0x00); //Address step of zero
  i2c_write(0x01); //Number of reads is 1

    i2c_rep_start(MLX90620_READ);

  byte configLow = i2c_readAck(); //Grab the two bytes
  byte configHigh = i2c_readAck();

  i2c_stop();

  return( (unsigned int)(configHigh << 8) | configLow); //Combine the configuration bytes and return as one unsigned int
}

boolean MLX90620::checkConfig_MLX90620()
{
  if ( (readConfig_MLX90620() & (unsigned int)1<<POR_TEST) == 0)
    return true;
  else
    return false;
}

