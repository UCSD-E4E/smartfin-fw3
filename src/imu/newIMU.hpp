/**
 * @file newIMU.hpp
 * @author Nathan Hui (nthui@ucsd.edu)
 * @brief New IMU implementation
 * @version 0.1
 * @date 2025-05-28
 *
 * @copyright Copyright (c) 2025
 *
 * TODO(290-hal-clean): ICM_20948_I2C _device and TwoWire &i2c_port are
 * Particle/Arduino types that prevent this header from being fully
 * board-agnostic. Abstracting the IMU driver requires either a HAL-level
 * SPI/I2C device handle or a platform-specific IMU wrapper class.
 */
#ifndef __NEW_IMU_HPP__
#define __NEW_IMU_HPP__
#include "imu/ICM-20948/ICM_20948.h"
#include "imu/imu_interface.hpp"
#include "platform/hal.hpp"

#include <cstdint>
#include <cstdio>
/**
 * @brief ICM-20948 IMU driver for Particle/Arduino targets.
 *
 * Implements IIMU using the SparkFun ICM-20948 library.  Not included in
 * the PC/GLibC build; the PC build uses the no-op stub in pc_hal/imu_stub.hpp.
 */
class IMU : public IIMU
{

private:
    /**
     * @brief FIFO Data structure
     *
     *
     */
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
    /**
     * @brief Device handle
     *
     */
    ICM_20948_I2C _device;
    /**
     * @brief read loop
     *
     */
    void *_readLoop = NULL;
    /**
     * @brief FIFO data access mutex
     *
     */
    SF_HAL::Mutex *_data_mtx = NULL;
    /**
     * @brief Device handle mutex
     *
     */
    SF_HAL::Mutex *_device_mtx = NULL;
    /**
     * @brief Data structure
     *
     */
    struct FiFoData fifo_data;
    /**
     * @brief Port handle
     *
     */
    TwoWire &i2c_port;
    /**
     * @brief Address
     *
     */
    const bool AD0_VAL = 0;

    /**
     * @brief Stop flag
     *
     */
    volatile bool stop_flag = false;

    /**
     * @brief Flag for ready
     *
     * If set, IMU is running.  Otherwise, if error_flag is set, init failed. If
     * error_flag not set, IMU still initializing
     *
     */
    volatile bool ready_flag = false;
    /**
     * @brief Flag for initialization error
     *
     * If set, async initialization failed
     *
     */
    volatile bool error_flag = false;

protected:
    /**
     * @brief Read loop function
     *
     * @param args Args (set to void)
     */
    static void readLoop(void *args);

public:
    /**
     * @brief Construct a new IMU object
     *
     * @param port I2C port handle
     * @param ad0_val AD0 value
     */
    IMU(TwoWire &port, const bool ad0_val = 0);
    /**
     * @brief Destroy the IMU object
     *
     */
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
    bool begin() override;

    /**
     * @brief Stops the IMU threads
     *
     * @return true Error occurred
     * @return false Successfully stopped the IMU
     */
    bool end() override;

    /**
     * @brief Retrieves the current IMU temperature in deg C
     *
     * @param temp Reference in which to store temperature
     * @return Failure flag - true if failure, otherwise false
     */
    bool getTemp_C(float &temp) override;
    /**
     * @brief Retrieves current rotational velocity in deg/s
     *
     * @param rot_x Reference to store x axis
     * @param rot_y Reference to store y axis
     * @param rot_z Reference to store z axis
     * @return Failure flag - true if failure, otherwise false
     */
    bool getRotVel_dps(float &rot_x, float &rot_y, float &rot_z) override;
    /**
     * @brief Retrieves current acceleration in m/s^2
     *
     * @param acc_x Reference to store x axis
     * @param acc_y Reference to store y axis
     * @param acc_z Reference to store z axis
     * @return Failure flag - true if failure, otherwise false
     */
    bool getAccel_ms2(float &acc_x, float &acc_y, float &acc_z) override;
    /**
     * @brief Retrieves current magnetic field intensity in uT
     *
     * @param mag_x Reference to store x axis
     * @param mag_y Reference to store y axis
     * @param mag_z Reference to store z axis
     * @return Failure flag - true if failure, otherwise false
     */
    bool getMag_uT(float &mag_x, float &mag_y, float &mag_z) override;
    /**
     * @brief Retrieves current acceleration in m/s^2 from DMP
     *
     * @param acc_x Reference to store x axis
     * @param acc_y Reference to store y axis
     * @param acc_z Reference to store z axis
     * @return Failure flag - true if failure, otherwise false
     */
    bool getDmpAccel_ms2(float &acc_x, float &acc_y, float &acc_z) override;
    /**
     * @brief Get the DMP orientation as a quaternion
     *
     * @param q0 Reference to store q0
     * @param q1 Reference to store q1
     * @param q2 Reference to store q2
     * @param q3 Reference to store q3
     * @param acc Pointer to store accuracy in; ignored if NULL
     * @return Failure flag - true if failure, otherwise false
     */
    bool getDmpQuat(double &q0,
                    double &q1,
                    double &q2,
                    double &q3,
                    double *acc) override;
    /**
     * @brief Get the DMP orientation as a single-precision quaternion
     *
     * @param q0 Reference to store q0
     * @param q1 Reference to store q1
     * @param q2 Reference to store q2
     * @param q3 Reference to store q3
     * @param acc Pointer to store accuracy in; ignored if NULL
     * @return Failure flag - true if failure, otherwise false
     */
    bool getDmpQuatf(float &q0,
                     float &q1,
                     float &q2,
                     float &q3,
                     float *acc) override;
    /**
     * @brief Retrieves current rotational velocity in deg/s from DMP
     *
     * @param rot_x Reference to store x axis
     * @param rot_y Reference to store y axis
     * @param rot_z Reference to store z axis
     * @return Failure flag - true if failure, otherwise false
     */
    bool getDmpRotVel_dps(float &rot_x, float &rot_y, float &rot_z) override;
    /**
     * @brief Retrieves current magnetic field intensity in uT from DMP
     *
     * @param mag_x Reference to store x axis
     * @param mag_y Reference to store y axis
     * @param mag_z Reference to store z axis
     * @return Failure flag - true if failure, otherwise false
     */
    bool getDmpMag_uT(float &mag_x, float &mag_y, float &mag_z) override;
    /**
     * @brief Retrieves raw Quat9 components and heading accuracy from the DMP
     *
     * Components are Q30 integers; accuracy is a Q12 integer (radians).
     * Recover q0 = sqrt(1 - q1^2 - q2^2 - q3^2) after scaling.
     *
     * @param q1 Reference to store Q1 (Q30)
     * @param q2 Reference to store Q2 (Q30)
     * @param q3 Reference to store Q3 (Q30)
     * @param accuracy_q12 Reference to store heading accuracy (Q12, radians)
     * @return Failure flag - true if failure, otherwise false
     */
    bool getRawQuat9(int32_t &q1, int32_t &q2, int32_t &q3,
                     int16_t &accuracy_q12) override;
    /**
     * @brief Dumps the registers
     *
     * @param printfn Printf function to use
     */
    void dumpRegs(int (*printfn)(const char *s, ...) = printf) override;
};
#endif // __NEW_IMU_HPP__