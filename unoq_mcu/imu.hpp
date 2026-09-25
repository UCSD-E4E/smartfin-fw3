/**
 * @file imu.hpp
 * @brief Lightweight ICM-20948 SPI driver for the STM32U585 MCU.
 */
#ifndef SF_MCU_IMU_HPP
#define SF_MCU_IMU_HPP

#include "../ipc/hal_rpc_protocol.h"
#include <stdbool.h>

namespace sf_mcu
{

/**
 * @brief Initialize the ICM-20948 sensor over SPI (Master mode).
 *
 * Configures the SPI interface and CS pin, verifies WHO_AM_I (0xEA),
 * wakes the sensor, and enables the accelerometer and gyroscope.
 *
 * @return true if initialization succeeded, false on error.
 */
bool imu_init();

/**
 * @brief Read the latest accelerometer and gyroscope raw measurements.
 *
 * Reads 12 contiguous registers starting at ACCEL_XOUT_H (0x2D) over SPI
 * in a single burst read and populates the sample record.
 *
 * @param sample Destination record to fill.
 * @return true if read succeeded, false on error.
 */
bool imu_read_sample(SF_RPC_IMUSampleRecord &sample);

} // namespace sf_mcu

#endif // SF_MCU_IMU_HPP
