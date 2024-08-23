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
#include "ICM_20948.h"
#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "consts.hpp"
#include "i2c/i2c.h"
#include <cmath>
#define SERIAL_PORT Serial


#define WIRE_PORT Wire
#define AD0_VAL 0


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
   // myICM.initializeDMP();
  


   SF_OSAL_printf("Initialization of the sensor returned: ");
   SF_OSAL_printf(myICM.statusString());
   if (myICM.status != ICM_20948_Stat_Ok)
   {
       SF_OSAL_printf("ICM fail!");
       FLOG_AddError(FLOG_ICM_FAIL, 0);
   }
   if (myICM.status == ICM_20948_Stat_ParamErr) {
      SF_OSAL_printf("3");
   } else if (myICM.status == ICM_20948_Stat_WrongID) {
      SF_OSAL_printf("4");
   } else if (myICM.status == ICM_20948_Stat_InvalSensor) {
      SF_OSAL_printf("5");
   } else if (myICM.status == ICM_20948_Stat_NoData) {
      SF_OSAL_printf("6");
   } else if (myICM.status == ICM_20948_Stat_SensorNotSupported) {
      SF_OSAL_printf("7");
   } else if (myICM.status == ICM_20948_Stat_DMPNotSupported) {
      SF_OSAL_printf("8");
   } else if (myICM.status == ICM_20948_Stat_DMPVerifyFail) {
      SF_OSAL_printf("9");
   } else if (myICM.status == ICM_20948_Stat_FIFONoDataAvail) {
      SF_OSAL_printf("10");
   } else if (myICM.status == ICM_20948_Stat_FIFOIncompleteData) {
      SF_OSAL_printf("10");
   } else if (myICM.status == ICM_20948_Stat_UnrecognisedDMPHeader) {
      SF_OSAL_printf("11");
   } else if (myICM.status == ICM_20948_Stat_InvalDMPRegister) {
      SF_OSAL_printf("12");
   } else if (myICM.status == ICM_20948_Stat_NUM) {
      SF_OSAL_printf("13");
   } else if (myICM.status == ICM_20948_Stat_Unknown) {
      SF_OSAL_printf("14");
   }else {
      SF_OSAL_printf("none");
   }
   bool success = true;
   success &= (myICM.initializeDMP() == ICM_20948_Stat_Ok);
   success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_ORIENTATION) == ICM_20948_Stat_Ok);
   // success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_RAW_GYROSCOPE) == ICM_20948_Stat_Ok);
    success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_RAW_ACCELEROMETER) == ICM_20948_Stat_Ok);
   // success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_GYROSCOPE) == ICM_20948_Stat_Ok);
   success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_ACCELEROMETER) == ICM_20948_Stat_Ok);
   // success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_MAGNETIC_FIELD_UNCALIBRATED) == ICM_20948_Stat_Ok);
   // success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_LINEAR_ACCELERATION) == ICM_20948_Stat_Ok);
   success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Quat9, 0) == ICM_20948_Stat_Ok);        // Set to 5Hz
   success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Accel, 0) == ICM_20948_Stat_Ok);        // Set to 1Hz
   success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Gyro, 0) == ICM_20948_Stat_Ok);         // Set to 1Hz
   success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Gyro_Calibr, 0) == ICM_20948_Stat_Ok);  // Set to 1Hz
   success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Cpass, 0) == ICM_20948_Stat_Ok);        // Set to 1Hz
   success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Cpass_Calibr, 0) == ICM_20948_Stat_Ok); // Set to 1Hz
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
       SF_OSAL_printf("DMP fail");
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

bool getDMPAccelerometer(float *acc_x, float *acc_y, float *acc_z, float *acc_acc)
{
   icm_20948_DMP_data_t data;
   myICM.readDMPdataFromFIFO(&data);
   if ((myICM.status == ICM_20948_Stat_Ok) || (myICM.status == ICM_20948_Stat_FIFOMoreDataAvail)) 
  {
  if ((data.header & DMP_header_bitmap_Accel) != 0) 
    {
   *acc_x = data.Raw_Accel.Data.X;
   *acc_y = data.Raw_Accel.Data.Y;
   *acc_z = data.Raw_Accel.Data.Z;
   }
  } else {
      FLOG_AddError(FLOG_ICM_FAIL, 3);
  }
   return true;
   
}

bool getDMPAcc(float*acc_acc)
{
   icm_20948_DMP_data_t data;
   myICM.readDMPdataFromFIFO(&data);
   if ((myICM.status == ICM_20948_Stat_Ok) || (myICM.status == ICM_20948_Stat_FIFOMoreDataAvail)) 
  {
  if ((data.header & DMP_header2_bitmap_Accel_Accuracy) != 0) 
    {
   *acc_acc = data.Accel_Accuracy;
   }
  } else {
      FLOG_AddError(FLOG_ICM_FAIL, 3);
  }
  return true;
}

bool getDMPQuaternion(double *q1, double *q2, double *q3, double *q0, double *acc)
{
   icm_20948_DMP_data_t data;
   myICM.readDMPdataFromFIFO(&data);
  
   *q1 = ((double)data.Quat9.Data.Q1) / 1073741824.0;
   *q2 = ((double)data.Quat9.Data.Q2) / 1073741824.0;
   *q3 = ((double)data.Quat9.Data.Q3) / 1073741824.0;
   *acc = (double)data.Quat9.Data.Accuracy;
   *q0 = sqrt(1.0 - ((*q1 * *q1) + (*q2 * *q2) + (*q3 * *q3)));


   return true;
}

bool getDMPGyroscope(float *g_x, float *g_y, float *g_z)
{
   icm_20948_DMP_data_t data;
   myICM.readDMPdataFromFIFO(&data);
  
   *g_x = (float)data.Gyro_Calibr.Data.X;
   *g_y = (float)data.Gyro_Calibr.Data.Y;
   *g_z = (float)data.Gyro_Calibr.Data.Z;


   return true;
}


bool getDMPQuat6(double *q1, double *q2, double *q3)
{
  icm_20948_DMP_data_t data;
  myICM.readDMPdataFromFIFO(&data);
  
  
  *q1 = ((double)data.Quat6.Data.Q1) / 1073741824.0; // Convert to double. Divide by 2^30
  *q2 = ((double)data.Quat6.Data.Q2) / 1073741824.0; // Convert to double. Divide by 2^30
  *q3 = ((double)data.Quat6.Data.Q3) / 1073741824.0; // Convert to double. Divide by 2^30   *c_x = (float)data.Compass.Data.X;
   


   return true;
}

// bool getDMPRaw(uint8_t *fb){
//    icm_20948_DMP_data_t data;
//   myICM.readDMPdataFromFIFO(&data);
//   for (size_t i = 0; i < sizeof(data.fifoByte); ++i) {
//     fb[i] = data.fifoByte[i];
//    }
// return true;
// }

void readDMP(void){

}

void whereDMP(void){
   // std::string sName(reinterpret_cast<char*>(name));
   Serial.write(myICM.getWhoAmI());
   Serial.write("\n");
}

void getDMPData(void) {
  int count = 0;
  int count1 = 0;
  double q1 = 0;
  double q2 = 0;
  double q3 = 0;
  double acc_x = 0;
  double acc_y = 0;
  double acc_z = 0;
  while(count < 50) {
    count++;
 icm_20948_DMP_data_t data;
 
 myICM.readDMPdataFromFIFO(&data);
  q1 = ((double)data.Quat6.Data.Q1) / 1073741824.0; // Convert to double. Divide by 2^30
  q2 = ((double)data.Quat6.Data.Q2) / 1073741824.0; // Convert to double. Divide by 2^30
  q3 = ((double)data.Quat6.Data.Q3) / 1073741824.0; // Convert to double. Divide by 2^30


 SF_OSAL_printf("Q1:    Q2:    Q3:  " __NL__) ;
 SF_OSAL_printf(" %8.4f  ", q1);
 SF_OSAL_printf(" %8.4f  ", q2);
 SF_OSAL_printf(" %8.4f  ", q3);
 SF_OSAL_printf(__NL__);
 delay(100);
  }

while(count1 < 500) {
    count1++;
 icm_20948_DMP_data_t data;
 myICM.readDMPdataFromFIFO(&data);
  acc_x = (float)data.Raw_Accel.Data.X; // Extract the raw accelerometer data
  acc_y = (float)data.Raw_Accel.Data.Y;
  acc_z = (float)data.Raw_Accel.Data.Z;


 SF_OSAL_printf("Accel: X:");
 SF_OSAL_printf(" %8.4f  ", acc_x);
 SF_OSAL_printf(" Y:");
 SF_OSAL_printf(" %8.4f  ", acc_y);
 SF_OSAL_printf(" Z:");
 SF_OSAL_printf(" %8.4f  " __NL__, acc_z);
 delay(100);
}

//  float x = (float)data.Raw_Gyro.Data.X; // Extract the raw gyro data
//  float y = (float)data.Raw_Gyro.Data.Y;
//  float z = (float)data.Raw_Gyro.Data.Z;


//  SF_OSAL_printf("Gyro: X:");
//  SF_OSAL_printf(" %8.4f  ", x);
//  SF_OSAL_printf(" Y:");
//  SF_OSAL_printf(" %8.4f  ", y);
//  SF_OSAL_printf(" Z:");
//  SF_OSAL_printf(" %8.4f  ", z);
  
  
//  float x = (float)data.Compass.Data.X; // Extract the compass data
//  float y = (float)data.Compass.Data.Y;
//  float z = (float)data.Compass.Data.Z;
//  SF_OSAL_printf("Compass: X:");
//  SF_OSAL_printf(" %8.4f  ", x);
//  SF_OSAL_printf(" Y:");
//  SF_OSAL_printf(" %8.4f  ", y);
//  SF_OSAL_printf(" Z:");
//  SF_OSAL_printf(" %8.4f  ", z);

 }