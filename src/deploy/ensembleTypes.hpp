#ifndef __ENSEMBLE_TYPES_HPP__
#define __ENSEMBLE_TYPES_HPP__

#include "Particle.h"
#include <cstdint>

/**
 * @brief Ensemble Data Types
 * 
 */
typedef enum EnsembleID_
{
    ENS_TEMP = 0x01,
    ENS_ACC,
    ENS_GPS,
    ENS_TEMP_ACC,
    ENS_TEMP_GPS,
    ENS_TEMP_ACC_GPS,
    ENS_BATT,
    ENS_TEMP_TIME,
    ENS_IMU,
    ENS_TEMP_IMU,
    ENS_TEMP_IMU_GPS,
    ENS_TEXT = 0x0F,
    ENS_NUM_ENSEMBLES
}EnsembleID_e;

#pragma pack(push, 1)
/**
 * @brief struct to contain header info for an ensemble
 */
typedef struct EnsembleHeader_
{
    unsigned int ensembleType : 4;
    unsigned int elapsedTime_ds : 20;
}EnsembleHeader_t;

/**
 * @brief Ensemble 07 - Battery
 * 
 */
typedef struct Ensemble07_data_
{
    /**
     * @brief Battery voltage (mV)
     * 
     */
    uint16_t batteryVoltage;
}Ensemble07_data_t;

/**
 * @brief Ensemble 08 - Temp and Time
 * 
 */
typedef struct Ensemble08_data_
{
    /**
     * @brief Temperature value of water given by the IMU sensor or MAX31725
     * temperature sensor at a given time.
     *
     * IMU sensor:
     * rawTemp = (temperature-21.0)/333.87
     *
     * MAX31725:
     * rawTemp = temperature * 128.0
     * 
     * where `temperature` is the scaled_temperature in Celcius and rawTemp is 
     * the stored representation in `int16_t`.
     * 
     */
    int16_t rawTemp;
    /**
     * @brief Time since Unix epoch
     *
     */
    uint32_t timestamp;
}Ensemble08_data_t;

/**
 * @brief Ensemble 10:
 * 
 * Temperature
 * Acceleration (x,y,z)
 * Angular Velocity (x,y,z)
 * Magnetic Field (x,y,z)
 */
typedef struct Ensemble10_data_
{
    /**
     * @brief Temperature value of water given by the IMU sensor or MAX31725
     * temperature sensor at a given time.
     *
     * IMU sensor:
     * rawTemp = (temperature-21.0)/333.87
     *
     * MAX31725:
     * rawTemp = temperature * 128.0
     *
     * where `temperature` is the scaled_temperature in Celcius and rawTemp is
     * the stored representation in `int16_t`.
     *
     */
    int16_t rawTemp;
    /**
     * @brief Array saving the accelerometer data on the x, y, and z axis at a given time
     * such that rawAcceleration[i] = acceleration[i] * 64, where `acceleration` is the
     * acceleration in G's (multiple of Earth gravity), and rawAcceleration is the stored
     * representation, stored as `int32_t`.Thus, rawAcceleration[i] = 64 = 1 G.
     *
     * G is approximately equal to 9.81 m/s^2
     */
    int16_t rawAcceleration[3];
    /**
     * @brief Array of length 3 saving the gyroscope data on the x, y, and z axis at a given time
     * such that rawAngularVel[i] = angular_velocity[i]/131.072, where 'angular_velocity' is
     * the angular velocity in degrees per second and rawAngularVel[3] is the stored representation,
     * stored as `int32_t`. Thus, rawAngularVel[3] = 1 = 131.072 degree/second
     */
    int16_t rawAngularVel[3];
    /**
     * @brief Array of length 3 saving the accumulated magnetometer data on the x, y, and z axis
     * at a given time in rawMagField[i] = magnetic_field[i]*0.15, where 'magnetic_field' is the
     * magnetic_field in uT and rawMagField is the stored representation, stored as `int32_t`. Thus,
     * rawMagField[i] = 1 = 6.667 uT
     *
     */
    int16_t rawMagField[3];
}Ensemble10_data_t;

/**
 * @brief Ensemble 11:
 * 
 * Temperature
 * Acceleration (x,y,z)
 * Angular Velocity (x,y,z)
 * Magnetic Field (x,y,z)
 * location (x,y)
 */
typedef struct Ensemble11_data_
{
    /**
     * @brief Temperature value of water given by the IMU sensor or MAX31725
     * temperature sensor at a given time.
     *
     * IMU sensor:
     * rawTemp = (temperature-21.0)/333.87
     *
     * MAX31725:
     * rawTemp = temperature * 128.0
     *
     * where `temperature` is the scaled_temperature in Celcius and rawTemp is
     * the stored representation in `int16_t`.
     *
     */
    int16_t rawTemp;
    /**
     * @brief Array saving the accelerometer data on the x, y, and z axis at a given time
     * such that rawAcceleration[i] = acceleration[i] * 64, where `acceleration` is the
     * acceleration in G's (multiple of Earth gravity), and rawAcceleration is the stored
     * representation, stored as `int32_t`.Thus, rawAcceleration[i] = 64 = 1 G.
     *
     * G is approximately equal to 9.81 m/s^2
     */
    int16_t rawAcceleration[3];
    /**
     * @brief Array of length 3 saving the gyroscope data on the x, y, and z axis at a given time
     * such that rawAngularVel[i] = angular_velocity[i]/131.072, where 'angular_velocity' is
     * the angular velocity in degrees per second and rawAngularVel[3] is the stored representation,
     * stored as `int32_t`. Thus, rawAngularVel[3] = 1 = 131.072 degree/second
     */
    int16_t rawAngularVel[3];
    /**
     * @brief Array of length 3 saving the accumulated magnetometer data on the x, y, and z axis
     * at a given time in rawMagField[i] = magnetic_field[i]*0.15, where 'magnetic_field' is the
     * magnetic_field in uT and rawMagField is the stored representation, stored as `int32_t`. Thus,
     * rawMagField[i] = 1 = 6.667 uT
     *
     */
    int16_t rawMagField[3];
    /**
     * @brief Array of length 2 saving latitude and longitude values, multiplied
     * by 1e6, of the point at which data was collected
     */
    int32_t location[2];
}Ensemble11_data_t;
#pragma pack(pop)

unsigned int Ens_getStartTime(system_tick_t sessionStart);
#endif
