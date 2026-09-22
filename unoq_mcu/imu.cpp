/**
 * @file imu.cpp
 * @brief Implementation of lightweight ICM-20948 I2C driver for STM32U585.
 */
#include "imu.hpp"

#include "Core/Inc/i2c.h"
#include "stm32u5xx_hal.h"

#include <cstring>

namespace sf_mcu
{

namespace
{

// ICM-20948 I2C Address (AD0 pulled low = 0x68, shifted left by 1 for STM32 HAL)
constexpr uint16_t kIcmI2cAddress = (0x68 << 1);

// Bank 0 Registers
constexpr uint8_t kRegWhoAmI        = 0x00;
constexpr uint8_t kRegPwrMgmt1      = 0x06;
constexpr uint8_t kRegPwrMgmt2      = 0x07;
constexpr uint8_t kRegAccelXoutH    = 0x2D;
constexpr uint8_t kRegBankSel       = 0x7F;

// Expected WHO_AM_I response
constexpr uint8_t kWhoAmIExpected   = 0xEA;

// I2C timeout in milliseconds
constexpr uint32_t kI2cTimeoutMs    = 50;

/**
 * @brief Select the active register bank (0-3).
 */
bool select_bank(uint8_t bank)
{
    uint8_t val = (bank << 4);
    return HAL_I2C_Mem_Write(&hi2c4, kIcmI2cAddress, kRegBankSel,
                             I2C_MEMADD_SIZE_8BIT, &val, 1, kI2cTimeoutMs) == HAL_OK;
}

} // namespace

bool imu_init()
{
    // Ensure we are in Bank 0
    if (!select_bank(0))
    {
        return false;
    }

    // 1. Verify WHO_AM_I
    uint8_t who_am_i = 0;
    if (HAL_I2C_Mem_Read(&hi2c4, kIcmI2cAddress, kRegWhoAmI,
                         I2C_MEMADD_SIZE_8BIT, &who_am_i, 1, kI2cTimeoutMs) != HAL_OK)
    {
        return false;
    }

    if (who_am_i != kWhoAmIExpected)
    {
        return false; // Wrong chip or not responding
    }

    // 2. Wake up chip from sleep: write 0x01 to PWR_MGMT_1 (Auto select clock source)
    uint8_t pwr_mgmt_1 = 0x01;
    if (HAL_I2C_Mem_Write(&hi2c4, kIcmI2cAddress, kRegPwrMgmt1,
                          I2C_MEMADD_SIZE_8BIT, &pwr_mgmt_1, 1, kI2cTimeoutMs) != HAL_OK)
    {
        return false;
    }

    HAL_Delay(10); // Wait for internal clocks to stabilize

    // 3. Enable accelerometer and gyroscope: write 0x00 to PWR_MGMT_2 (all axes enabled)
    uint8_t pwr_mgmt_2 = 0x00;
    if (HAL_I2C_Mem_Write(&hi2c4, kIcmI2cAddress, kRegPwrMgmt2,
                          I2C_MEMADD_SIZE_8BIT, &pwr_mgmt_2, 1, kI2cTimeoutMs) != HAL_OK)
    {
        return false;
    }

    return true;
}

bool imu_read_sample(SF_RPC_IMUSampleRecord &sample)
{
    // Buffer for 12 bytes:
    // [0..1]: Accel X (H, L)
    // [2..3]: Accel Y (H, L)
    // [4..5]: Accel Z (H, L)
    // [6..7]: Gyro X (H, L)
    // [8..9]: Gyro Y (H, L)
    // [10..11]: Gyro Z (H, L)
    uint8_t raw_buf[12];

    // Burst read 12 contiguous registers starting at ACCEL_XOUT_H (0x2D)
    if (HAL_I2C_Mem_Read(&hi2c4, kIcmI2cAddress, kRegAccelXoutH,
                         I2C_MEMADD_SIZE_8BIT, raw_buf, sizeof(raw_buf), kI2cTimeoutMs) != HAL_OK)
    {
        return false;
    }

    sample.timestamp_ms = HAL_GetTick();

    // Reconstruct 16-bit big-endian signed integers
    sample.accel_x = static_cast<int16_t>((raw_buf[0] << 8) | raw_buf[1]);
    sample.accel_y = static_cast<int16_t>((raw_buf[2] << 8) | raw_buf[3]);
    sample.accel_z = static_cast<int16_t>((raw_buf[4] << 8) | raw_buf[5]);

    sample.gyro_x  = static_cast<int16_t>((raw_buf[6] << 8) | raw_buf[7]);
    sample.gyro_y  = static_cast<int16_t>((raw_buf[8] << 8) | raw_buf[9]);
    sample.gyro_z  = static_cast<int16_t>((raw_buf[10] << 8) | raw_buf[11]);

    // Quaternions and Magnetometer are set to 0 for now.
    // We will add 3D orientation/quaternion math later once basic accel + gyro readings are tested.
    sample.mag_x = 0;
    sample.mag_y = 0;
    sample.mag_z = 0;
    sample.quat9_1 = 0;
    sample.quat9_2 = 0;
    sample.quat9_3 = 0;
    sample.quat9_accuracy = 0;

    return true;
}

} // namespace sf_mcu
