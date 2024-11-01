/**
 * @file imu.hpp
 * @author Emily Thorpe (ethorpe@macalester.edu)
 * @author Nathan Hui (nthui@ucsd.edu)
 * @brief Inertial Measurement Unit Interface
 * @version 0.1
 * @date 2024-10-31
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef  __ICM20648_HPP__
#define  __ICM20648_HPP__

/** Do a measurement on the gyroscope
 *
 * @param temperature temp value in Celsius
 *
 * @returns true if measurement was successful
 */
bool getTemperature(float *temperature);

/** Do a measurement on the gyroscope
 *
 * @param gyr_x Gyroscope measurement on X axis
 * @param gyr_y Gyroscope measurement on Y axis
 * @param gyr_z Gyroscope measurement on Z axis
 *
 * Unit: DPS
 *
 * @returns true if measurement was successful
 */
bool getGyroscope(float *gyr_x, float *gyr_y, float *gyr_z);

/** Do a measurement on the accelerometer
 *
 * @param acc_x Accelerometer measurement on X axis
 * @param acc_y Accelerometer measurement on Y axis
 * @param acc_z Accelerometer measurement on Z axis
 *
 * Unit: M/s^2
 *
 * @returns true if measurement was successful
 */
bool getAccelerometer(float *acc_x, float *acc_y, float *acc_z);

/** Do a measurement on the magnetometer
 *
 * @param mag_x magnetometer measurement on X axis
 * @param mag_y magnetometer measurement on Y axis
 * @param mag_z magnetometer measurement on Z axis
 *
 * Unit: microTesla (uT)
 *
 * @returns true if measurement was successful
 */
bool getMagnetometer(float *mag_x, float *mag_y, float *mag_z);

/**
 * @brief Retrieves the Accelerometer data from the DMP
 *
 * @param acc_x Pointer to variable to populate with x axis acceleration in m/s^2
 * @param acc_y Pointer to variable to populate with y axis acceleration in m/s^2
 * @param acc_z Pointer to variable to populate with z axis acceleration in m/s^2
 * @return True if measurement was successful, otherwise False
 */
bool getDMPAccelerometer(float *acc_x, float *acc_y, float *acc_z);

/**
 * @brief Retrieves the currently computed orientation as a quarternion from the DMP
 *
 * @param q1 Pointer to variable to populate with the first element of the quaternion
 * @param q2 Pointer to variable to populate with the second element of the quaternion
 * @param q3 Pointer to variable to populate with the third element of the quaternion
 * @param q0 Pointer to variable to populate with the fourth element of the quaternion
 * @param acc Pointer to variable to populate with the quaternion estimation accuracy
 * @return True if measurement was successful, otherwise False
 */
bool getDMPQuaternion(double *q1, double *q2, double *q3, double *q0, double *acc);

/**
 * @brief Retrieves the Gyroscope data from the DMP
 *
 * @param g_x Pointer to variable to populate with the x axis gyroscope data in deg/s
 * @param g_y Pointer to variable to populate with the y axis gyroscope data in deg/s
 * @param g_z Pointer to variable to populate with the z axis gyroscope data in deg/s
 * @return True if measurement was successful, otherwise False
 */
bool getDMPGyroscope(float *g_x, float *g_y, float *g_z);

/**
 * @brief Retrieves the Accelerometer accuracy metric from the DMP
 *
 * @param acc_acc Pointer to a variable to populate with the accelerometer accuracy
 * @return True if measurement was successful, otherwise False
 */
bool getDMPAccelerometerAcc(float *acc_acc);

/**
 * @brief Setup IMU
 */
void setupICM(void);
void whereDMP(void);
#endif
