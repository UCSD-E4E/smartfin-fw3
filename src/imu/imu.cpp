/****************************************************************
* Example1_Basics.ino, Example8_DMP_RawAccel.ino
* ICM 20948 Arduino Library Demo
* Use the default configuration to stream 9-axis IMU data
* Owen Lyke @ SparkFun Electronics
* Original Creation Date: April 17 2019
*
* Please see License.md for the license information.
*
* Distributed as-is; no warranty is given.
*
* Modified by Emily Thorpe - August 2023
***************************************************************/
#include "ICM_20948.h"
#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "consts.hpp"
#include "i2c/i2c.h"
#include <cmath>
#include <map>
#include <vector>
#define SERIAL_PORT Serial


#define WIRE_PORT Wire
#define AD0_VAL 1  //should be set to 0, currently for dev board need to change to 1

#define GIB 1073741824

ICM_20948_I2C myICM;


float getAccMG( int16_t raw, uint8_t fss );
float getGyrDPS( int16_t raw, uint8_t fss );
float getMagUT( int16_t raw );
float getTmpC( int16_t raw );


void setupICM(void)
{
   WIRE_PORT.begin();
   WIRE_PORT.setClock(400000);


   myICM.begin(WIRE_PORT, AD0_VAL);
  
   SF_OSAL_printf("Initialization of the sensor returned: ");
   SF_OSAL_printf(myICM.statusString());
   if (myICM.status != ICM_20948_Stat_Ok)
   {
       SF_OSAL_printf("ICM fail!");
       FLOG_AddError(FLOG_ICM_FAIL, myICM.status);
   }
   
  // DMP sensor options are defined in ICM_20948_DMP.h
  //    INV_ICM20948_SENSOR_ACCELEROMETER               (16-bit accel)
  //    INV_ICM20948_SENSOR_GYROSCOPE                   (16-bit gyro + 32-bit calibrated gyro)
  //    INV_ICM20948_SENSOR_RAW_ACCELEROMETER           (16-bit accel)
  //    INV_ICM20948_SENSOR_RAW_GYROSCOPE               (16-bit gyro + 32-bit calibrated gyro)
  //    INV_ICM20948_SENSOR_MAGNETIC_FIELD_UNCALIBRATED (16-bit compass)
  //    INV_ICM20948_SENSOR_GYROSCOPE_UNCALIBRATED      (16-bit gyro)
  //    INV_ICM20948_SENSOR_STEP_DETECTOR               (Pedometer Step Detector)
  //    INV_ICM20948_SENSOR_STEP_COUNTER                (Pedometer Step Detector)
  //    INV_ICM20948_SENSOR_GAME_ROTATION_VECTOR        (32-bit 6-axis quaternion)
  //    INV_ICM20948_SENSOR_ROTATION_VECTOR             (32-bit 9-axis quaternion + heading accuracy)
  //    INV_ICM20948_SENSOR_GEOMAGNETIC_ROTATION_VECTOR (32-bit Geomag RV + heading accuracy)
  //    INV_ICM20948_SENSOR_GEOMAGNETIC_FIELD           (32-bit calibrated compass)
  //    INV_ICM20948_SENSOR_GRAVITY                     (32-bit 6-axis quaternion)
  //    INV_ICM20948_SENSOR_LINEAR_ACCELERATION         (16-bit accel + 32-bit 6-axis quaternion)
  //    INV_ICM20948_SENSOR_ORIENTATION                 (32-bit 9-axis quaternion + heading accuracy)
   
   bool success = true;
   success &= (myICM.initializeDMP() == ICM_20948_Stat_Ok);
    // Enable the DMP sensors you want
    // Configuring DMP to output data at multiple ODRs:
    // DMP is capable of outputting multiple sensor data at different rates to FIFO.
    // Setting value can be calculated as follows:
    // Value = (DMP running rate / ODR ) - 1
    // E.g. For a 5Hz ODR rate when DMP is running at 55Hz, value = (55/5) - 1 = 10.
   success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_ORIENTATION) == ICM_20948_Stat_Ok);
   success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_RAW_ACCELEROMETER) == ICM_20948_Stat_Ok);
   success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_ACCELEROMETER) == ICM_20948_Stat_Ok);
   success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Quat9, 0) == ICM_20948_Stat_Ok);        
   success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Accel, 0) == ICM_20948_Stat_Ok);       

   //Enable the FIFO
   success &= (myICM.enableFIFO() == ICM_20948_Stat_Ok);


   // Enable the DMP
   success &= (myICM.enableDMP() == ICM_20948_Stat_Ok);


   // Reset DMP
   success &= (myICM.resetDMP() == ICM_20948_Stat_Ok);


   // Reset FIFO
   success &= (myICM.resetFIFO() == ICM_20948_Stat_Ok);
   if (success == false)
   {
       SF_OSAL_printf("DMP fail!" __NL__);
       FLOG_AddError(FLOG_ICM_FAIL, 1);
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

bool getDMPAccelerometer(float* acc_x, float* acc_y, float* acc_z)
{
   icm_20948_DMP_data_t data;
   myICM.readDMPdataFromFIFO(&data);
   if ((myICM.status == ICM_20948_Stat_Ok) || (myICM.status == ICM_20948_Stat_FIFOMoreDataAvail))
   {
      if ((data.header & DMP_header_bitmap_Accel) != 0)
      {
         *acc_x = (float)data.Raw_Accel.Data.X;
         *acc_y = (float)data.Raw_Accel.Data.Y;
         *acc_z = (float)data.Raw_Accel.Data.Z;
         return true;
      }
   }
   return false;

}

//TODO: DMP Packet needs to include acceleration accuracy (header2)
bool getDMPAccelerometerAcc(float* acc_acc)
{
   icm_20948_DMP_data_t data;
   myICM.readDMPdataFromFIFO(&data);
   if ((myICM.status == ICM_20948_Stat_Ok) || (myICM.status == ICM_20948_Stat_FIFOMoreDataAvail))
   {
      
      if (((data.header & DMP_header_bitmap_Header2) != 0) && ((data.header2 & DMP_header2_bitmap_Accel_Accuracy) != 0))
      {
         *acc_acc = data.Accel_Accuracy;
         return true;
      } 
   }
   
   FLOG_AddError(FLOG_ICM_FAIL, myICM.status);
   return false;
}

bool getDMPQuaternion(double *q1, double *q2, double *q3, double *q0, double *acc)
{
   icm_20948_DMP_data_t data;
   myICM.readDMPdataFromFIFO(&data);
   if ((data.header & DMP_header_bitmap_Quat9) != 0)
      {
         *q1 = ((double)data.Quat9.Data.Q1) / GIB;
         *q2 = ((double)data.Quat9.Data.Q2) / GIB;
         *q3 = ((double)data.Quat9.Data.Q3) / GIB;
         //*acc = (double)data.Quat9.Data.Accuracy;
         *q0 = sqrt(1.0 - ((*q1 * *q1) + (*q2 * *q2) + (*q3 * *q3)));
         return true;
      }
   
   return false;
}

bool getDMPGyroscope(float *g_x, float *g_y, float *g_z)
{
   icm_20948_DMP_data_t data;
   myICM.readDMPdataFromFIFO(&data);
   if ((data.header & DMP_header_bitmap_Gyro) != 0)
      {
         *g_x = (float)data.Gyro_Calibr.Data.X;
         *g_y = (float)data.Gyro_Calibr.Data.Y;
         *g_z = (float)data.Gyro_Calibr.Data.Z;
         return true;
      }

   return false;
}

