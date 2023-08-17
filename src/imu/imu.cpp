/****************************************************************
 * Example999_Portable.ino
 * ICM 20948 Arduino Library Demo 
 * Uses underlying portable C skeleton with user-defined read/write functions
 * Owen Lyke @ SparkFun Electronics
 * Original Creation Date: April 17 2019
 * 
 * This code is beerware; if you see me (or any other SparkFun employee) at the
 * local, and you've found our code helpful, please buy us a round!
 * 
 * Distributed as-is; no warranty is given.
 ***************************************************************/
#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "imu.hpp"

#define SERIAL_PORT Serial



#define WIRE_PORT Wire
#define I2C_ADDR ICM_20948_I2C_ADDR_AD1


ICM_20948_Status_e my_write_i2c(uint8_t reg, uint8_t* data, uint32_t len, void* user);
ICM_20948_Status_e my_read_i2c(uint8_t reg, uint8_t* buff, uint32_t len, void* user);
void printPaddedInt16b( int16_t val );
void printRawAGMT( ICM_20948_AGMT_t agmt);
float getAccMG( int16_t raw, uint8_t fss );
float getGyrDPS( int16_t raw, uint8_t fss );
float getMagUT( int16_t raw );
float getTmpC( int16_t raw );

const ICM_20948_Serif_t mySerif = {
my_write_i2c, // write
my_read_i2c,  // read
NULL,         
};

// Now declare the structure that represents the ICM.
ICM_20948_Device_t myICM;

void setupIMU(void) {
    
    // Perform platform initialization
    Serial.begin(115200);


    WIRE_PORT.begin();
    WIRE_PORT.setClock(400000);

    // Link the serif
    ICM_20948_link_serif(&myICM, &mySerif);


    while(ICM_20948_check_id( &myICM ) != ICM_20948_Stat_Ok){
        SF_OSAL_printf("whoami does not match. Halting...\n");
        delay(1000);
    }

    ICM_20948_Status_e stat = ICM_20948_Stat_Err;
    uint8_t whoami = 0x00;
    while( (stat != ICM_20948_Stat_Ok) || (whoami != ICM_20948_WHOAMI) ) {
        whoami = 0x00;
        stat = ICM_20948_get_who_am_i(&myICM, &whoami);
        SF_OSAL_printf("whoami does not match (0x");
        SF_OSAL_printf("%d%d", whoami, HEX);
        SF_OSAL_printf("). Halting...");
        SF_OSAL_printf("\n");
        delay(1000);
    }

    // Here we are doing a SW reset to make sure the device starts in a known state
    ICM_20948_sw_reset( &myICM );
    delay(250);

    // Set Gyro and Accelerometer to a particular sample mode
    ICM_20948_set_sample_mode( &myICM, (ICM_20948_InternalSensorID_bm)(ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), ICM_20948_Sample_Mode_Continuous ); // optiona: ICM_20948_Sample_Mode_Continuous. ICM_20948_Sample_Mode_Cycled

    // Set full scale ranges for both acc and gyr
    ICM_20948_fss_t myfss;
    myfss.a = gpm2;   // (ICM_20948_ACCEL_CONFIG_FS_SEL_e)
    myfss.g = dps250; // (ICM_20948_GYRO_CONFIG_1_FS_SEL_e)
    ICM_20948_set_full_scale( &myICM, (ICM_20948_InternalSensorID_bm)(ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), myfss );

    // Set up DLPF configuration
    ICM_20948_dlpcfg_t myDLPcfg;
    myDLPcfg.a = acc_d473bw_n499bw;
    myDLPcfg.g = gyr_d361bw4_n376bw5;
    ICM_20948_set_dlpf_cfg    ( &myICM, (ICM_20948_InternalSensorID_bm)(ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), myDLPcfg );

    // Choose whether or not to use DLPF
    ICM_20948_enable_dlpf    ( &myICM, ICM_20948_Internal_Acc, false );
    ICM_20948_enable_dlpf    ( &myICM, ICM_20948_Internal_Gyr, false );

    // ICM_20948_I2C icmObj;
    // icmObj.startupMagnetometer();
    // startupMagnetometer();


    // Now wake the sensor up
    ICM_20948_sleep         ( &myICM, false );
    ICM_20948_low_power     ( &myICM, false );

}

bool getAMGTValues(ICM_20948_AGMT_t *agmt)
{
    ICM_20948_AGMT_t icmAgmt = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0}};
    if(ICM_20948_get_agmt( &myICM, &icmAgmt ) == ICM_20948_Stat_Ok)
    {
        *agmt = icmAgmt;
        return 0;
    } else
    {
        SF_OSAL_printf("ICM Fail\n");
        FLOG_AddError(FLOG_ICM_FAIL, 0);
        return 1;
    }
}

bool getAccelerometer(float *acc_x, float *acc_y, float *acc_z)
{
    ICM_20948_AGMT_t agmt = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0}};
    getAMGTValues(&agmt);
    
    *acc_x = getAccMG(agmt.acc.axes.x, agmt.fss.a);
    *acc_y = getAccMG(agmt.acc.axes.y, agmt.fss.a);
    *acc_z = getAccMG(agmt.acc.axes.z, agmt.fss.a);

    return true;
}

bool getGyroscope(float *gyr_x, float *gyr_y, float *gyr_z)
{
    ICM_20948_AGMT_t agmt = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0}};
    getAMGTValues(&agmt);
    
    *gyr_x = getGyrDPS(agmt.gyr.axes.x, agmt.fss.g);
    *gyr_y = getGyrDPS(agmt.gyr.axes.y, agmt.fss.g);
    *gyr_z = getGyrDPS(agmt.gyr.axes.z, agmt.fss.g);

    return true;
}

bool getMagnetometer(float *mag_x, float *mag_y, float *mag_z)
{
    ICM_20948_AGMT_t agmt = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0}};
    getAMGTValues(&agmt);

    SF_OSAL_printf("mag x \n %f", agmt.mag.axes.x);
    
    *mag_x = getMagUT(agmt.mag.axes.x);
    *mag_y = getMagUT(agmt.mag.axes.y);
    *mag_z = getMagUT(agmt.mag.axes.z);

    return true;
}

bool getTemperature(float *temperature)
{
    ICM_20948_AGMT_t agmt = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0}};
    getAMGTValues(&agmt);

    *temperature = getTmpC(agmt.tmp.val);
    return true;
}


ICM_20948_Status_e my_write_i2c(uint8_t reg, uint8_t* data, uint32_t len, void* user)
{
    WIRE_PORT.beginTransmission(I2C_ADDR);
    WIRE_PORT.write(reg);
    WIRE_PORT.write(data, len);
    WIRE_PORT.endTransmission();

    return ICM_20948_Stat_Ok;
}

ICM_20948_Status_e my_read_i2c(uint8_t reg, uint8_t *buff, uint32_t len, void *user)
{
    WIRE_PORT.beginTransmission(I2C_ADDR);
    WIRE_PORT.write(reg);
    WIRE_PORT.endTransmission(false); // Send repeated start

    uint32_t num_received = WIRE_PORT.requestFrom(I2C_ADDR, len);
    if (num_received == len)
    {
      for (uint32_t i = 0; i < len; i++)
      {
          buff[i] = WIRE_PORT.read();
      }
    }
    WIRE_PORT.endTransmission();

    return ICM_20948_Stat_Ok;
}


void stopICM(void)
{
    ICM_20948_sleep         ( &myICM, true );
    ICM_20948_low_power     ( &myICM, true );
}

void printPaddedInt16b( int16_t val )
{
  if(val > 0){
    SF_OSAL_printf(" ");
    if(val < 10000){ SF_OSAL_printf("0"); }
    if(val < 1000 ){ SF_OSAL_printf("0"); }
    if(val < 100  ){ SF_OSAL_printf("0"); }
    if(val < 10   ){ SF_OSAL_printf("0"); }
  }else{
    SF_OSAL_printf("-");
    if(abs(val) < 10000){ SF_OSAL_printf("0"); }
    if(abs(val) < 1000 ){ SF_OSAL_printf("0"); }
    if(abs(val) < 100  ){ SF_OSAL_printf("0"); }
    if(abs(val) < 10   ){ SF_OSAL_printf("0"); }
  }
  SF_OSAL_printf("%d", abs(val));
}

void printRawAGMT( ICM_20948_AGMT_t agmt){
  SF_OSAL_printf("RAW. Acc [ ");
  printPaddedInt16b( agmt.acc.axes.x );
  SF_OSAL_printf(", ");
  printPaddedInt16b( agmt.acc.axes.y );
  SF_OSAL_printf(", ");
  printPaddedInt16b( agmt.acc.axes.z );
  SF_OSAL_printf(" ], Gyr [ ");
  printPaddedInt16b( agmt.gyr.axes.x );
  SF_OSAL_printf(", ");
  printPaddedInt16b( agmt.gyr.axes.y );
  SF_OSAL_printf(", ");
  printPaddedInt16b( agmt.gyr.axes.z );
  SF_OSAL_printf(" ], Mag [ ");
  SF_OSAL_printf("%d", agmt.mag.axes.x );
  SF_OSAL_printf(", ");
  SF_OSAL_printf("%d", agmt.mag.axes.y );
  SF_OSAL_printf(", ");
  SF_OSAL_printf("%d", agmt.mag.axes.z );
  SF_OSAL_printf(" ], Tmp [ ");
  printPaddedInt16b( agmt.tmp.val );
  SF_OSAL_printf(" ]");
  SF_OSAL_printf("\n");
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
