/**
 * @file newIMU.hpp
 * @author Nathan Hui (nthui@ucsd.edu)
 * @brief New IMU implementation
 * @version 0.1
 * @date 2025-05-28
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef __NEW_IMU_HPP__
#define __NEW_IMU_HPP__
#include "Particle.h"
#include "imu/ICM-20948/ICM_20948.h"

#include <cstdint>
#include <cstdio>
class IMU
{
private:
    struct FiFoData
    {
        std::int16_t RawAccel_X;
        std::int16_t RawAccel_Y;
        std::int16_t RawAccel_Z;
        std::int16_t RawGyro_X;
        std::int16_t RawGyro_Y;
        std::int16_t RawGyro_Z;
        std::int16_t RawGyro_BiasX;
        std::int16_t RawGyro_BiasY;
        std::int16_t RawGyro_BiasZ;
        std::int16_t Compass_X;
        std::int16_t Compass_Y;
        std::int16_t Compass_Z;
        std::int32_t Quat6_1;
        std::int32_t Quat6_2;
        std::int32_t Quat6_3;
        std::int32_t Quat9_1;
        std::int32_t Quat9_2;
        std::int32_t Quat9_3;
        std::int32_t Quat9_Acc;
        std::int32_t PQuat6_1;
        std::int32_t PQuat6_2;
        std::int32_t PQuat6_3;
        std::int32_t Geomag_1;
        std::int32_t Geomag_2;
        std::int32_t Geomag_3;
        std::int32_t Geomag_Acc;
        std::int32_t Accel_Acc;
        std::int32_t Gyro_Acc;
        std::int32_t Compass_Acc;
    };

    ICM_20948_I2C _device;
    Thread *_readLoop = NULL;
    Mutex *_data_mtx = NULL;
    Mutex *_device_mtx = NULL;
    struct FiFoData fifo_data;
    TwoWire &i2c_port;
    const bool AD0_VAL = 0;

protected:
    static void readLoop(void *args);

public:
    IMU(TwoWire &port, const bool ad0_val = 0) : i2c_port(port), AD0_VAL(ad0_val)
    {
    }
    ~IMU()
    {
    }
    /**
     * @brief Starts the IMU
     * 
     * This initializes IMU registers and starts the read loop
     * 
     * @return true 
     * @return false 
     */
    bool begin(void);

    bool getTemp_C(float &temp);
    bool getRotVel_dps(float &rot_x, float &rot_y, float &rot_z);
    bool getAccel_ms2(float &acc_x, float &acc_y, float &acc_z);
    bool getMag_uT(float &mag_x, float &mag_y, float &mag_z);
    bool getDmpAccel_ms2(float &acc_x, float &acc_y, float &acc_z);
    bool getDmpQuat(double &q0, double &q1, double &q2, double &q3, double *acc);
    bool getDmpQuatf(float &q0, float &q1, float &q2, float &q3, float *acc);
    bool getDmpRotVel_dps(float &rot_x, float &rot_y, float &rot_z);
    bool getDmpMag_uT(float &mag_x, float &mag_y, float &mag_z);
    void dumpRegs(int (*printfn)(const char *s, ...) = printf);
};
#endif // __NEW_IMU_HPP__