/**
 * @file imu_interface.hpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Platform-agnostic abstract interface for the IMU driver.
 * @date 2026-05-26
 */
#ifndef IMU_INTERFACE_HPP
#define IMU_INTERFACE_HPP

#include <cstdio>

/**
 * @brief Abstract IMU interface.
 *
 * All application code that reads IMU data uses this type through a pointer
 * stored in SystemDesc.  The concrete implementation (ICM-20948 on Particle)
 * lives in newIMU.hpp / newIMU.cpp and is never included in the PC build.
 * The PC build substitutes a no-op stub declared in pc_hal/imu_stub.hpp.
 */
class IIMU
{
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IIMU()
    {
    }

    /**
     * @brief Initialise hardware and start the read loop.
     *
     * @return true if initialisation failed, false on success.
     */
    virtual bool begin() = 0;

    /**
     * @brief Stop the read loop and release hardware resources.
     *
     * @return true if an error occurred, false on success.
     */
    virtual bool end() = 0;

    /**
     * @brief Retrieve the IMU die temperature.
     *
     * @param temp Reference to store temperature in degrees C.
     * @return true on failure, false on success.
     */
    virtual bool getTemp_C(float &temp) = 0;

    /**
     * @brief Retrieve rotational velocity from the gyroscope.
     *
     * @param rot_x Reference to store x-axis rate in deg/s.
     * @param rot_y Reference to store y-axis rate in deg/s.
     * @param rot_z Reference to store z-axis rate in deg/s.
     * @return true on failure, false on success.
     */
    virtual bool getRotVel_dps(float &rot_x, float &rot_y, float &rot_z) = 0;

    /**
     * @brief Retrieve linear acceleration from the accelerometer.
     *
     * @param acc_x Reference to store x-axis acceleration in m/s^2.
     * @param acc_y Reference to store y-axis acceleration in m/s^2.
     * @param acc_z Reference to store z-axis acceleration in m/s^2.
     * @return true on failure, false on success.
     */
    virtual bool getAccel_ms2(float &acc_x, float &acc_y, float &acc_z) = 0;

    /**
     * @brief Retrieve magnetic field intensity from the magnetometer.
     *
     * @param mag_x Reference to store x-axis field in uT.
     * @param mag_y Reference to store y-axis field in uT.
     * @param mag_z Reference to store z-axis field in uT.
     * @return true on failure, false on success.
     */
    virtual bool getMag_uT(float &mag_x, float &mag_y, float &mag_z) = 0;

    /**
     * @brief Retrieve DMP-fused linear acceleration.
     *
     * @param acc_x Reference to store x-axis acceleration in m/s^2.
     * @param acc_y Reference to store y-axis acceleration in m/s^2.
     * @param acc_z Reference to store z-axis acceleration in m/s^2.
     * @return true on failure, false on success.
     */
    virtual bool getDmpAccel_ms2(float &acc_x, float &acc_y, float &acc_z) = 0;

    /**
     * @brief Retrieve DMP orientation as a double-precision quaternion.
     *
     * @param q0  Reference to store scalar component.
     * @param q1  Reference to store x component.
     * @param q2  Reference to store y component.
     * @param q3  Reference to store z component.
     * @param acc Pointer to store accuracy estimate; ignored if NULL.
     * @return true on failure, false on success.
     */
    virtual bool getDmpQuat(double &q0,
                            double &q1,
                            double &q2,
                            double &q3,
                            double *acc) = 0;

    /**
     * @brief Retrieve DMP orientation as a single-precision quaternion.
     *
     * @param q0  Reference to store scalar component.
     * @param q1  Reference to store x component.
     * @param q2  Reference to store y component.
     * @param q3  Reference to store z component.
     * @param acc Pointer to store accuracy estimate; ignored if NULL.
     * @return true on failure, false on success.
     */
    virtual bool getDmpQuatf(float &q0,
                             float &q1,
                             float &q2,
                             float &q3,
                             float *acc) = 0;

    /**
     * @brief Retrieve DMP-fused rotational velocity.
     *
     * @param rot_x Reference to store x-axis rate in deg/s.
     * @param rot_y Reference to store y-axis rate in deg/s.
     * @param rot_z Reference to store z-axis rate in deg/s.
     * @return true on failure, false on success.
     */
    virtual bool getDmpRotVel_dps(float &rot_x,
                                  float &rot_y,
                                  float &rot_z) = 0;

    /**
     * @brief Retrieve DMP-fused magnetic field intensity.
     *
     * @param mag_x Reference to store x-axis field in uT.
     * @param mag_y Reference to store y-axis field in uT.
     * @param mag_z Reference to store z-axis field in uT.
     * @return true on failure, false on success.
     */
    virtual bool getDmpMag_uT(float &mag_x, float &mag_y, float &mag_z) = 0;

    /**
     * @brief Dump raw register contents for diagnostics.
     *
     * @param printfn Printf-compatible function used for output.
     */
    virtual void dumpRegs(int (*printfn)(const char *s, ...) = printf) = 0;
};

#endif // IMU_INTERFACE_HPP
