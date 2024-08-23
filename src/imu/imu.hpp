#ifndef  __ICM20648_HPP__
#define  __ICM20648_HPP__

/** Do a measurement on the gyroscope
 *
 * @param[out] temperature temp value in Celsius
 *
 * @returns true if measurement was successful
 */
bool getTemperature(float *temperature);

/** Do a measurement on the gyroscope
 *
 * @param[out] gyr_x Gyroscope measurement on X axis
 * @param[out] gyr_y Gyroscope measurement on Y axis
 * @param[out] gyr_z Gyroscope measurement on Z axis
 *
 * Unit: DPS
 * 
 * @returns true if measurement was successful
 */
bool getGyroscope(float *gyr_x, float *gyr_y, float *gyr_z);

/** Do a measurement on the accelerometer
 *
 * @param[out] acc_x Accelerometer measurement on X axis
 * @param[out] acc_y Accelerometer measurement on Y axis
 * @param[out] acc_z Accelerometer measurement on Z axis
 *
 * Unit: M/s^2
 * 
 * @returns true if measurement was successful
 */
bool getAccelerometer(float *acc_x, float *acc_y, float *acc_z);

/** Do a measurement on the magnetometer
 *
 * @param[out] mag_x magnetometer measurement on X axis
 * @param[out] mag_y magnetometer measurement on Y axis
 * @param[out] mag_z magnetometer measurement on Z axis
 * 
 * Unit: gauss (G)
 *
 * @returns true if measurement was successful
 */
bool getMagnetometer(float *mag_x, float *mag_y, float *mag_z);
/**
 * @brief Setup IMU 
*/
//bool getDMPRawAccelerometer(float *acc_x, float *acc_y, float *acc_z);
bool getDMPAccelerometer(float *acc_x, float *acc_y, float *acc_z);
bool getDMPQuaternion(double *q1, double *q2, double *q3, double *q0, double *acc);
bool getDMPGyroscope(float *g_x, float *g_y, float *g_z);
bool getDMPCompass(float *c_x, float *c_y, float *c_z);
bool getDMPQuat6(double *q1, double *q2, double *q3);
void setupICM(void);
void getDMPData(void);
//bool getDMPRaw(uint8_t *fb);
bool getDMPAccelerometerAcc(float* acc_acc);
#endif
