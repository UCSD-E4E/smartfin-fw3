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
 * 
 * Modified by Emily Thorpe - Auguest 2023
 ***************************************************************/

#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "product.hpp"
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
#include "ICM_20948.h"

#define SERIAL_PORT Serial

#define WIRE_PORT Wire
#define AD0_VAL 1

ICM_20948_I2C myICM;
#endif

float getAccMG( int16_t raw, uint8_t fss );
float getGyrDPS( int16_t raw, uint8_t fss );
float getMagUT( int16_t raw );
float getTmpC( int16_t raw );

void setupICM(void)
{
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    WIRE_PORT.begin();
    WIRE_PORT.setClock(400000);

    myICM.begin(WIRE_PORT, AD0_VAL);

    SF_OSAL_printf("Initialization of the sensor returned: ");
    SF_OSAL_printf(myICM.statusString());
    if (myICM.status != ICM_20948_Stat_Ok)
    {
        SF_OSAL_printf("ICM fail");
        FLOG_AddError(FLOG_ICM_FAIL, 0);
    }
#endif
}


bool getAccelerometer(float *acc_x, float *acc_y, float *acc_z)
{
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    ICM_20948_AGMT_t agmt = myICM.getAGMT();

    if (myICM.status != ICM_20948_Stat_Ok)
    {
        FLOG_AddError(FLOG_ICM_FAIL, 0);
        return false;
    }

    *acc_x = getAccMG(agmt.acc.axes.x, agmt.fss.a);
    *acc_y = getAccMG(agmt.acc.axes.y, agmt.fss.a);
    *acc_z = getAccMG(agmt.acc.axes.z, agmt.fss.a);
#endif
    return true;
}

bool getGyroscope(float *gyr_x, float *gyr_y, float *gyr_z)
{
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    ICM_20948_AGMT_t agmt = myICM.getAGMT();

    if (myICM.status != ICM_20948_Stat_Ok)
    {
        FLOG_AddError(FLOG_ICM_FAIL, 0);
        return false;
    }

    *gyr_x = getGyrDPS(agmt.gyr.axes.x, agmt.fss.g);
    *gyr_y = getGyrDPS(agmt.gyr.axes.y, agmt.fss.g);
    *gyr_z = getGyrDPS(agmt.gyr.axes.z, agmt.fss.g);
#endif
    return true;
}

bool getMagnetometer(float *mag_x, float *mag_y, float *mag_z)
{
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    ICM_20948_AGMT_t agmt = myICM.getAGMT();

    if (myICM.status != ICM_20948_Stat_Ok)
    {
        FLOG_AddError(FLOG_ICM_FAIL, 0);
        return false;
    }

    *mag_x = getMagUT(agmt.mag.axes.x);
    *mag_y = getMagUT(agmt.mag.axes.y);
    *mag_z = getMagUT(agmt.mag.axes.z);
#endif
    return true;
}

bool getTemperature(float *temperature)
{
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    ICM_20948_AGMT_t agmt = myICM.getAGMT();

    *temperature = getTmpC(agmt.tmp.val);
#endif
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
