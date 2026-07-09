/**
 * @file imu_stub.hpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief No-op IMU implementation for the PC/GLibC build.
 * @date 2026-05-26
 */
#ifndef __IMU_STUB_HPP__
#define __IMU_STUB_HPP__

#include "imu/imu_interface.hpp"

#include <cstdio>
/**
 * @brief No-op IMU stub used in place of the ICM-20948 driver on PC builds.
 *
 * All methods return false (success) and leave output references unchanged.
 * This allows the PC simulator to compile and run without any Particle SDK or
 * ICM-20948 vendor headers.
 */
class IMUStub : public IIMU
{
public:
    /**
     * @brief No-op initialisation.
     * @return false (success).
     */
    bool begin() override
    {
        return false;
    }

    /**
     * @brief No-op shutdown.
     * @return false (success).
     */
    bool end() override
    {
        return false;
    }

    /**
     * @brief Returns false; leaves temp unchanged.
     * @param temp Unused.
     * @return false.
     */
    bool getTemp_C(float & /*temp*/) override
    {
        return false;
    }

    /**
     * @brief Returns false; leaves rotation references unchanged.
     * @param rot_x Unused.
     * @param rot_y Unused.
     * @param rot_z Unused.
     * @return false.
     */
    bool getRotVel_dps(float & /*rot_x*/,
                       float & /*rot_y*/,
                       float & /*rot_z*/) override
    {
        return false;
    }

    /**
     * @brief Returns false; leaves acceleration references unchanged.
     * @param acc_x Unused.
     * @param acc_y Unused.
     * @param acc_z Unused.
     * @return false.
     */
    bool getAccel_ms2(float & /*acc_x*/,
                      float & /*acc_y*/,
                      float & /*acc_z*/) override
    {
        return false;
    }

    /**
     * @brief Returns false; leaves magnetic field references unchanged.
     * @param mag_x Unused.
     * @param mag_y Unused.
     * @param mag_z Unused.
     * @return false.
     */
    bool getMag_uT(float & /*mag_x*/,
                   float & /*mag_y*/,
                   float & /*mag_z*/) override
    {
        return false;
    }

    /**
     * @brief Returns false; leaves DMP acceleration references unchanged.
     * @param acc_x Unused.
     * @param acc_y Unused.
     * @param acc_z Unused.
     * @return false.
     */
    bool getDmpAccel_ms2(float & /*acc_x*/,
                         float & /*acc_y*/,
                         float & /*acc_z*/) override
    {
        return false;
    }

    /**
     * @brief Returns false; leaves quaternion references unchanged.
     * @param q0  Unused.
     * @param q1  Unused.
     * @param q2  Unused.
     * @param q3  Unused.
     * @param acc Unused.
     * @return false.
     */
    bool getDmpQuat(double & /*q0*/,
                    double & /*q1*/,
                    double & /*q2*/,
                    double & /*q3*/,
                    double * /*acc*/) override
    {
        return false;
    }

    /**
     * @brief Returns false; leaves quaternion references unchanged.
     * @param q0  Unused.
     * @param q1  Unused.
     * @param q2  Unused.
     * @param q3  Unused.
     * @param acc Unused.
     * @return false.
     */
    bool getDmpQuatf(float & /*q0*/,
                     float & /*q1*/,
                     float & /*q2*/,
                     float & /*q3*/,
                     float * /*acc*/) override
    {
        return false;
    }

    /**
     * @brief Returns false; leaves DMP rotation references unchanged.
     * @param rot_x Unused.
     * @param rot_y Unused.
     * @param rot_z Unused.
     * @return false.
     */
    bool getDmpRotVel_dps(float & /*rot_x*/,
                          float & /*rot_y*/,
                          float & /*rot_z*/) override
    {
        return false;
    }

    /**
     * @brief Returns false; leaves DMP magnetic field references unchanged.
     * @param mag_x Unused.
     * @param mag_y Unused.
     * @param mag_z Unused.
     * @return false.
     */
    bool getDmpMag_uT(float & /*mag_x*/,
                      float & /*mag_y*/,
                      float & /*mag_z*/) override
    {
        return false;
    }

    /**
     * @brief Returns false; leaves Quat9 references unchanged.
     * @param q1 Unused.
     * @param q2 Unused.
     * @param q3 Unused.
     * @param accuracy_q12 Unused.
     * @return false.
     */
    bool getRawQuat9(int32_t & /*q1*/,
                     int32_t & /*q2*/,
                     int32_t & /*q3*/,
                     int16_t & /*accuracy_q12*/) override
    {
        return false;
    }

    /**
     * @brief No-op register dump.
     * @param printfn Unused.
     */
    void dumpRegs(int (* /*printfn*/)(const char *, ...) = printf) override
    {
    }
};

#endif // __IMU_STUB_HPP__
