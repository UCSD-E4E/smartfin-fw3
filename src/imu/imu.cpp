/****************************************************************
 * Example1_Basics.ino
 * ICM 20948 Arduino Library Demo
 * Use the default configuration to stream 9-axis IMU data
 * Owen Lyke @ SparkFun Electronics
 * Original Creation Date: April 17 2019
 *
 * Please see License.md for the license information.
 *
 * Distributed as-is; no warranty is given.
 ***************************************************************/
#include "ICM_20948.h" // Click here to get the library: http://librarymanager/All#SparkFun_ICM_20948_IMU
#include "cli/conio.hpp"

#define SERIAL_PORT Serial

#define WIRE_PORT Wire
#define AD0_VAL 1

ICM_20948_I2C myICM; // Otherwise create an ICM_20948_I2C object

void printPaddedInt16b(int16_t val);
void printRawAGMT(ICM_20948_AGMT_t agmt);
void printFormattedFloat(float val, uint8_t leading, uint8_t decimals);
void printScaledAGMT(ICM_20948_I2C *sensor);
float getAccMG( int16_t raw, uint8_t fss );
float getGyrDPS( int16_t raw, uint8_t fss );
float getMagUT( int16_t raw );
float getTmpC( int16_t raw );

void setupICM(void)
{

  WIRE_PORT.begin();
  WIRE_PORT.setClock(400000);

  //myICM.enableDebugging(); // Uncomment this line to enable helpful debug messages on Serial

  bool initialized = false;
  while (!initialized)
  {
    myICM.begin(WIRE_PORT, AD0_VAL);

    SF_OSAL_printf("Initialization of the sensor returned: ");
    SF_OSAL_printf(myICM.statusString());
    if (myICM.status != ICM_20948_Stat_Ok)
    {
      SF_OSAL_printf("Trying again...");
      delay(500);
    }
    else
    {
      initialized = true;
    }
  }
}


bool getAccelerometer(float *acc_x, float *acc_y, float *acc_z)
{
    ICM_20948_AGMT_t agmt = myICM.getAGMT();
    
    *acc_x = getAccMG(agmt.acc.axes.x, agmt.fss.a);
    *acc_y = getAccMG(agmt.acc.axes.y, agmt.fss.a);
    *acc_z = getAccMG(agmt.acc.axes.z, agmt.fss.a);

    return true;
}

bool getGyroscope(float *gyr_x, float *gyr_y, float *gyr_z)
{
    ICM_20948_AGMT_t agmt = myICM.getAGMT();
    
    *gyr_x = getGyrDPS(agmt.gyr.axes.x, agmt.fss.g);
    *gyr_y = getGyrDPS(agmt.gyr.axes.y, agmt.fss.g);
    *gyr_z = getGyrDPS(agmt.gyr.axes.z, agmt.fss.g);

    return true;
}

bool getMagnetometer(float *mag_x, float *mag_y, float *mag_z)
{
    ICM_20948_AGMT_t agmt = myICM.getAGMT();    

    *mag_x = getMagUT(agmt.mag.axes.x);
    *mag_y = getMagUT(agmt.mag.axes.y);
    *mag_z = getMagUT(agmt.mag.axes.z);

    return true;
}

bool getTemperature(float *temperature)
{
    ICM_20948_AGMT_t agmt = myICM.getAGMT();

    *temperature = getTmpC(agmt.tmp.val);
    return true;
}

float getAccMG( int16_t raw, uint8_t fss ){
  switch(fss){
    case 0 : return (((float)raw)/16.384); break;
    case 1 : return (((float)raw)/8.192); break;
    case 2 : return (((float)raw)/4.096); break;
    case 3 : return (((float)raw)/2.048); break;
    default : return 0; break;
  }
}

float getGyrDPS( int16_t raw, uint8_t fss ){
  switch(fss){
    case 0 : return (((float)raw)/131); break;
    case 1 : return (((float)raw)/65.5); break;
    case 2 : return (((float)raw)/32.8); break;
    case 3 : return (((float)raw)/16.4); break;
    default : return 0; break;
  }
}

float getMagUT( int16_t raw ){
  return (((float)raw)*0.15);
}

float getTmpC( int16_t raw ){
  return (((float)raw)/333.87);
}
