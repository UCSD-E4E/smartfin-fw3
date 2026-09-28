/**
 * @file imu.cpp
 * @brief Implementation of lightweight ICM-20948 SPI driver for STM32U585.
 */
#include "imu.hpp"

#include "dmp_firmware.hpp"
#include "stm32u5xx_hal.h"
#include <cstring>

namespace sf_mcu
{

namespace
{

// ICM-20948 Bank 0 Registers
constexpr uint8_t kRegWhoAmI        = 0x00;
constexpr uint8_t kRegPwrMgmt1      = 0x06;
constexpr uint8_t kRegPwrMgmt2      = 0x07;
constexpr uint8_t kRegAccelXoutH    = 0x2D;
constexpr uint8_t kRegMemStartAddr  = 0x7C;
constexpr uint8_t kRegMemRW         = 0x7D;
constexpr uint8_t kRegMemBankSel    = 0x7E;
constexpr uint8_t kRegBankSel       = 0x7F;

// ICM-20948 Bank 2 Registers
constexpr uint8_t kRegPrgmStartH    = 0x50;
constexpr uint8_t kRegPrgmStartL    = 0x51;

// DMP Constants
constexpr uint16_t kDmpLoadStart    = 0x0090;
constexpr uint16_t kDmpStartAddr    = 0x1000;

// Expected WHO_AM_I response
constexpr uint8_t kWhoAmIExpected   = 0xEA;

// SPI timeout in milliseconds
constexpr uint32_t kSpiTimeoutMs    = 50;

// Default CS pin on Uno Q (Arduino D10 / PB12 or dedicated JSPI CS)
#ifndef IMU_DEFAULT_CS_PORT
#define IMU_DEFAULT_CS_PORT GPIOB
#endif
#ifndef IMU_DEFAULT_CS_PIN
#define IMU_DEFAULT_CS_PIN  GPIO_PIN_12
#endif

// Hardware handles
static SPI_HandleTypeDef s_hspi2;
static GPIO_TypeDef *s_cs_port   = IMU_DEFAULT_CS_PORT;
static uint16_t s_cs_pin         = IMU_DEFAULT_CS_PIN;
static uint8_t s_last_mems_bank  = 0xFF;

/**
 * @brief Low-level SPI register write.
 *
 * Pulls CS low, sets Bit 7 = 0 to signify a write, sends the value,
 * and releases CS high.
 */
bool spi_write_reg(uint8_t reg, uint8_t value)
{
    uint8_t tx_data[2] = {
        static_cast<uint8_t>(reg & 0x7F), // Bit 7 cleared (0) for WRITE
        value
    };

    HAL_GPIO_WritePin(s_cs_port, s_cs_pin, GPIO_PIN_RESET);
    HAL_StatusTypeDef status = HAL_SPI_Transmit(&s_hspi2, tx_data, 2, kSpiTimeoutMs);
    HAL_GPIO_WritePin(s_cs_port, s_cs_pin, GPIO_PIN_SET);

    return status == HAL_OK;
}

/**
 * @brief Low-level SPI burst register write.
 *
 * Pulls CS low, sets Bit 7 = 0 to signify a write, sends register address,
 * transmits len bytes, and releases CS high.
 */
bool spi_write_regs(uint8_t reg, const uint8_t *buf, uint16_t len)
{
    if (buf == nullptr || len == 0)
    {
        return false;
    }

    uint8_t reg_addr = static_cast<uint8_t>(reg & 0x7F); // Bit 7 cleared (0) for WRITE

    HAL_GPIO_WritePin(s_cs_port, s_cs_pin, GPIO_PIN_RESET);
    HAL_StatusTypeDef status = HAL_SPI_Transmit(&s_hspi2, &reg_addr, 1, kSpiTimeoutMs);
    if (status == HAL_OK)
    {
        status = HAL_SPI_Transmit(&s_hspi2, const_cast<uint8_t *>(buf), len, kSpiTimeoutMs);
    }
    HAL_GPIO_WritePin(s_cs_port, s_cs_pin, GPIO_PIN_SET);

    return status == HAL_OK;
}

/**
 * @brief Low-level SPI burst register read.
 *
 * Pulls CS low, sets Bit 7 = 1 to signify a read, reads len bytes,
 * and releases CS high.
 */
bool spi_read_regs(uint8_t reg, uint8_t *buf, uint16_t len)
{
    if (buf == nullptr || len == 0)
    {
        return false;
    }

    uint8_t reg_addr = static_cast<uint8_t>((reg & 0x7F) | 0x80); // Bit 7 set (1) for READ

    HAL_GPIO_WritePin(s_cs_port, s_cs_pin, GPIO_PIN_RESET);
    HAL_StatusTypeDef status = HAL_SPI_Transmit(&s_hspi2, &reg_addr, 1, kSpiTimeoutMs);
    if (status == HAL_OK)
    {
        status = HAL_SPI_Receive(&s_hspi2, buf, len, kSpiTimeoutMs);
    }
    HAL_GPIO_WritePin(s_cs_port, s_cs_pin, GPIO_PIN_SET);

    return status == HAL_OK;
}

/**
 * @brief Select the active ICM-20948 register bank (0-3).
 */
bool select_bank(uint8_t bank)
{
    uint8_t val = (bank << 4);
    return spi_write_reg(kRegBankSel, val);
}

/**
 * @brief Write data into DMP memory across 256-byte banks.
 *
 * Automatically manages MEM_BANK_SEL, MEM_START_ADDR, and handles
 * chunking so burst writes never cross 256-byte bank boundaries.
 */
bool dmp_write_mems(uint16_t reg, const uint8_t *data, uint16_t length)
{
    if (!select_bank(0) || data == nullptr)
    {
        return false;
    }

    uint16_t bytes_written = 0;
    while (bytes_written < length)
    {
        uint16_t current_addr = reg + bytes_written;
        uint8_t bank = static_cast<uint8_t>(current_addr >> 8);
        uint8_t start_addr = static_cast<uint8_t>(current_addr & 0xFF);

        if (bank != s_last_mems_bank)
        {
            if (!spi_write_reg(kRegMemBankSel, bank))
            {
                return false;
            }
            s_last_mems_bank = bank;
        }

        if (!spi_write_reg(kRegMemStartAddr, start_addr))
        {
            return false;
        }

        // Chunk size: up to 16 bytes, without crossing the 256-byte bank boundary
        uint16_t chunk = length - bytes_written;
        if (chunk > 16)
        {
            chunk = 16;
        }
        uint16_t remaining_in_bank = 256 - start_addr;
        if (chunk > remaining_in_bank)
        {
            chunk = remaining_in_bank;
        }

        if (!spi_write_regs(kRegMemRW, &data[bytes_written], chunk))
        {
            return false;
        }

        bytes_written += chunk;
    }

    return true;
}

/**
 * @brief Read data from DMP memory across 256-byte banks.
 */
bool dmp_read_mems(uint16_t reg, uint8_t *data, uint16_t length)
{
    if (!select_bank(0) || data == nullptr)
    {
        return false;
    }

    uint16_t bytes_read = 0;
    while (bytes_read < length)
    {
        uint16_t current_addr = reg + bytes_read;
        uint8_t bank = static_cast<uint8_t>(current_addr >> 8);
        uint8_t start_addr = static_cast<uint8_t>(current_addr & 0xFF);

        if (bank != s_last_mems_bank)
        {
            if (!spi_write_reg(kRegMemBankSel, bank))
            {
                return false;
            }
            s_last_mems_bank = bank;
        }

        if (!spi_write_reg(kRegMemStartAddr, start_addr))
        {
            return false;
        }

        uint16_t chunk = length - bytes_read;
        if (chunk > 16)
        {
            chunk = 16;
        }
        uint16_t remaining_in_bank = 256 - start_addr;
        if (chunk > remaining_in_bank)
        {
            chunk = remaining_in_bank;
        }

        if (!spi_read_regs(kRegMemRW, &data[bytes_read], chunk))
        {
            return false;
        }

        bytes_read += chunk;
    }

    return true;
}

/**
 * @brief Load and verify the 14,290-byte DMP firmware image into the ICM-20948.
 */
bool dmp_load_firmware()
{
    // 1. Write the 14,290 bytes starting at kDmpLoadStart (0x0090)
    if (!dmp_write_mems(kDmpLoadStart, kDmpFirmwareImage, kDmpFirmwareSize))
    {
        return false;
    }

    // 2. Verify: read back in chunks and compare with firmware image
    constexpr uint16_t kVerifyChunk = 16;
    uint8_t verify_buf[kVerifyChunk];
    uint16_t verified = 0;

    while (verified < kDmpFirmwareSize)
    {
        uint16_t chunk = kDmpFirmwareSize - verified;
        if (chunk > kVerifyChunk)
        {
            chunk = kVerifyChunk;
        }

        if (!dmp_read_mems(kDmpLoadStart + verified, verify_buf, chunk))
        {
            return false;
        }

        if (std::memcmp(verify_buf, &kDmpFirmwareImage[verified], chunk) != 0)
        {
            return false; // Verification failed
        }

        verified += chunk;
    }

    // 3. Set the Program Start Address (0x1000) in Bank 2 registers
    if (!select_bank(2))
    {
        return false;
    }
    if (!spi_write_reg(kRegPrgmStartH, static_cast<uint8_t>(kDmpStartAddr >> 8)))
    {
        return false;
    }
    if (!spi_write_reg(kRegPrgmStartL, static_cast<uint8_t>(kDmpStartAddr & 0xFF)))
    {
        return false;
    }

    // Return to Bank 0
    return select_bank(0);
}

/**
 * @brief Initialize SPI2 Master hardware and CS GPIO pin.
 */
bool init_spi_master()
{
    // Enable Clocks
    __HAL_RCC_SPI2_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    // 1. Configure CS GPIO pin as Output Push-Pull, initial state HIGH (deselected)
    GPIO_InitTypeDef gpio_cs = {0};
    gpio_cs.Pin = s_cs_pin;
    gpio_cs.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_cs.Pull = GPIO_NOPULL;
    gpio_cs.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(s_cs_port, &gpio_cs);
    HAL_GPIO_WritePin(s_cs_port, s_cs_pin, GPIO_PIN_SET);

    // 2. Configure SPI2 Master pins (JSPI header: MISO PC2, MOSI PC3, SCK PD1)
    GPIO_InitTypeDef gpio_spi = {0};

    // SCK (PD1 - AF5_SPI2)
    gpio_spi.Pin = GPIO_PIN_1;
    gpio_spi.Mode = GPIO_MODE_AF_PP;
    gpio_spi.Pull = GPIO_NOPULL;
    gpio_spi.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_spi.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOD, &gpio_spi);

    // MISO (PC2 - AF5_SPI2) & MOSI (PC3 - AF5_SPI2)
    gpio_spi.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    gpio_spi.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOC, &gpio_spi);

    // 3. Configure SPI2 Master instance (Mode 0: CPOL Low, CPHA 1Edge, up to ~7 MHz)
    s_hspi2.Instance               = SPI2;
    s_hspi2.Init.Mode              = SPI_MODE_MASTER;
    s_hspi2.Init.Direction         = SPI_DIRECTION_2LINES;
    s_hspi2.Init.DataSize          = SPI_DATASIZE_8BIT;
    s_hspi2.Init.CLKPolarity       = SPI_POLARITY_LOW;
    s_hspi2.Init.CLKPhase          = SPI_PHASE_1EDGE;
    s_hspi2.Init.NSS               = SPI_NSS_SOFT; // Software CS management
    s_hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32; // ~5 MHz from 160 MHz clock
    s_hspi2.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    s_hspi2.Init.TIMode            = SPI_TIMODE_DISABLE;
    s_hspi2.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;

    return HAL_SPI_Init(&s_hspi2) == HAL_OK;
}

// DMP Memory Addresses
constexpr uint16_t kDmpAccScale     = 480;  // 30 * 16 + 0
constexpr uint16_t kDmpAccScale2    = 1268; // 79 * 16 + 4
constexpr uint16_t kDmpDataOutCtl1  = 64;   // 4 * 16
constexpr uint16_t kDmpDataOutCtl2  = 66;   // 4 * 16 + 2
constexpr uint16_t kDmpDataRdyStat  = 138;  // 8 * 16 + 10
constexpr uint16_t kDmpOdrQuat9     = 168;  // 10 * 16 + 8
constexpr uint16_t kDmpOdrCntrQuat9 = 158;  // 9 * 16 + 14
constexpr uint16_t kDmpOdrAccel     = 190;  // 11 * 16 + 14
constexpr uint16_t kDmpOdrCntrAccel = 146;  // 9 * 16 + 2
constexpr uint16_t kDmpOdrGyro      = 186;  // 11 * 16 + 10
constexpr uint16_t kDmpOdrCntrGyro  = 150;  // 9 * 16 + 6

// Bank 0 FIFO and Control Registers
constexpr uint8_t kRegUserCtrl      = 0x03;
constexpr uint8_t kRegHwFixDisable  = 0x75;
constexpr uint8_t kRegFifoCountH    = 0x70;
constexpr uint8_t kRegFifoCountL    = 0x71;
constexpr uint8_t kRegFifoRW        = 0x72;

/**
 * @brief Configure DMP to compute 9-axis Quaternions at 55 Hz and stream to FIFO.
 */
bool dmp_configure_quat9()
{
    // 1. Accel scaling for 4g FSR:
    // FSR stands for Full Scale Range (the maximum measurement limit before clipping).
    // Available ranges are ±2g, ±4g, ±8g, or ±16g. We configure ±4g (±39.2 m/s²) because
    // surfing dynamics typically experience peak accelerations between 1g and 3g; ±4g provides
    // fine resolution for ocean waves without saturating during wave turns or paddling strokes.
    // The DMP fusion engine expects raw counts scaled such that 2^25 = 1g in ±4g mode.
    const uint8_t acc_scale[4]  = {0x04, 0x00, 0x00, 0x00};
    const uint8_t acc_scale2[4] = {0x00, 0x04, 0x00, 0x00};
    if (!dmp_write_mems(kDmpAccScale, acc_scale, 4) ||
        !dmp_write_mems(kDmpAccScale2, acc_scale2, 4))
    {
        return false;
    }

    // 2. Set 55 Hz Output Data Rate (ODR) dividers: 1125 / (1 + 19) = 56.25 Hz ≈ 55 Hz
    const uint8_t odr_val[2]  = {0x00, 19};
    const uint8_t odr_zero[2] = {0x00, 0x00};

    // Quat9 ODR
    dmp_write_mems(kDmpOdrQuat9, odr_val, 2);
    dmp_write_mems(kDmpOdrCntrQuat9, odr_zero, 2);
    // Accel ODR
    dmp_write_mems(kDmpOdrAccel, odr_val, 2);
    dmp_write_mems(kDmpOdrCntrAccel, odr_zero, 2);
    // Gyro ODR
    dmp_write_mems(kDmpOdrGyro, odr_val, 2);
    dmp_write_mems(kDmpOdrCntrGyro, odr_zero, 2);

    // 3. Configure DATA_OUT_CTL1 and DATA_OUT_CTL2
    // DATA_OUT_CTL1: Accel (0x8000) | Gyro (0x4000) | Quat9 (0x0400) = 0xC400
    const uint8_t data_out_ctl1[2] = {0xC4, 0x00};
    if (!dmp_write_mems(kDmpDataOutCtl1, data_out_ctl1, 2))
    {
        return false;
    }

    // DATA_OUT_CTL2: Compass Accuracy flag (0x0008) for Quat9 heading accuracy
    const uint8_t data_out_ctl2[2] = {0x00, 0x08};
    if (!dmp_write_mems(kDmpDataOutCtl2, data_out_ctl2, 2))
    {
        return false;
    }

    // DATA_RDY_STATUS: Gyro (1) | Accel (2) | Compass (8) = 0x000B
    const uint8_t data_rdy_status[2] = {0x00, 0x0B};
    if (!dmp_write_mems(kDmpDataRdyStat, data_rdy_status, 2))
    {
        return false;
    }

    // 4. Return to Bank 0 and enable FIFO streaming for DMP
    if (!select_bank(0))
    {
        return false;
    }

    // Set hardware fix disable to 0x48 (required by InvenSense DMP application note)
    spi_write_reg(kRegHwFixDisable, 0x48);

    // Reset FIFO & DMP
    spi_write_reg(kRegUserCtrl, 0x0C); // DMP_RST | FIFO_RST
    HAL_Delay(10);

    // Enable DMP and FIFO in USER_CTRL (0x80 DMP_EN | 0x40 FIFO_EN)
    if (!spi_write_reg(kRegUserCtrl, 0xC0))
    {
        return false;
    }

    return true;
}

} // namespace

bool imu_init()
{
    // Initialize SPI2 Master and CS pin
    if (!init_spi_master())
    {
        return false;
    }

    // Ensure we are in Bank 0
    if (!select_bank(0))
    {
        return false;
    }

    // 1. Verify WHO_AM_I (0x00) -> expected 0xEA
    uint8_t who_am_i = 0;
    if (!spi_read_regs(kRegWhoAmI, &who_am_i, 1))
    {
        return false;
    }

    if (who_am_i != kWhoAmIExpected)
    {
        return false; // Wrong chip or SPI connection issue
    }

    // 2. Wake up chip from sleep: write 0x01 to PWR_MGMT_1 (Auto-select clock source)
    if (!spi_write_reg(kRegPwrMgmt1, 0x01))
    {
        return false;
    }

    HAL_Delay(10); // Wait for internal clocks to stabilize

    // 3. Enable accelerometer and gyroscope: write 0x00 to PWR_MGMT_2 (all axes enabled)
    if (!spi_write_reg(kRegPwrMgmt2, 0x00))
    {
        return false;
    }

    // 4. Load and verify DMP firmware image into ICM-20948 SRAM
    if (!dmp_load_firmware())
    {
        return false;
    }

    // 5. Configure 55 Hz Quat9 DMP engine and start FIFO
    if (!dmp_configure_quat9())
    {
        return false;
    }

    return true;
}

bool imu_read_sample(SF_RPC_IMUSampleRecord &sample)
{
    if (!select_bank(0))
    {
        return false;
    }

    // 1. Attempt to read from DMP FIFO
    uint8_t count_buf[2];
    if (spi_read_regs(kRegFifoCountH, count_buf, 2))
    {
        uint16_t fifo_count = static_cast<uint16_t>((count_buf[0] << 8) | count_buf[1]);

        // DMP packet layout:
        // [0..1]: Header
        // [2..3]: Header2
        // [4..9]: Accel X, Y, Z (6 bytes)
        // [10..21]: Gyro X, Y, Z + Bias (12 bytes)
        // [22..35]: Quat9: Q1, Q2, Q3 (12 bytes) + Accuracy (2 bytes)
        // Total = 36 bytes
        constexpr uint16_t kDmpPacketSize = 36;
        if (fifo_count >= kDmpPacketSize)
        {
            uint8_t packet[kDmpPacketSize];
            if (spi_read_regs(kRegFifoRW, packet, kDmpPacketSize))
            {
                sample.timestamp_ms = HAL_GetTick();

                // Accelerometer (16-bit big-endian)
                sample.accel_x = static_cast<int16_t>((packet[4] << 8) | packet[5]);
                sample.accel_y = static_cast<int16_t>((packet[6] << 8) | packet[7]);
                sample.accel_z = static_cast<int16_t>((packet[8] << 8) | packet[9]);

                // Gyroscope (16-bit big-endian)
                sample.gyro_x  = static_cast<int16_t>((packet[10] << 8) | packet[11]);
                sample.gyro_y  = static_cast<int16_t>((packet[12] << 8) | packet[13]);
                sample.gyro_z  = static_cast<int16_t>((packet[14] << 8) | packet[15]);

                // Quat9 3D orientation (Q30 fixed-point 32-bit big-endian).
                // Note on q0 (scalar component) and Gimbal Lock:
                // Quaternions completely prevent Gimbal Lock (the loss of a rotational degree of
                // freedom that occurs in Euler angles when pitch reaches 90 degrees). A 3D rotation
                // quaternion is a unit vector: q0^2 + q1^2 + q2^2 + q3^2 = 1.0.
                // Because of this identity, q0 is algebraically redundant: q0 = sqrt(1 - q1^2 - q2^2 - q3^2).
                // The DMP omits q0 to save 4 bytes per sample across the SPI bus and FIFO.
                // Host software or cloud pipelines reconstruct q0 using this formula (see Ensemble13_data_t).
                sample.quat9_1 = static_cast<int32_t>((static_cast<uint32_t>(packet[22]) << 24) |
                                                      (static_cast<uint32_t>(packet[23]) << 16) |
                                                      (static_cast<uint32_t>(packet[24]) << 8)  |
                                                      static_cast<uint32_t>(packet[25]));

                sample.quat9_2 = static_cast<int32_t>((static_cast<uint32_t>(packet[26]) << 24) |
                                                      (static_cast<uint32_t>(packet[27]) << 16) |
                                                      (static_cast<uint32_t>(packet[28]) << 8)  |
                                                      static_cast<uint32_t>(packet[29]));

                sample.quat9_3 = static_cast<int32_t>((static_cast<uint32_t>(packet[30]) << 24) |
                                                      (static_cast<uint32_t>(packet[31]) << 16) |
                                                      (static_cast<uint32_t>(packet[32]) << 8)  |
                                                      static_cast<uint32_t>(packet[33]));

                // Heading Accuracy (Q12 radians 16-bit big-endian)
                sample.quat9_accuracy = static_cast<int16_t>((packet[34] << 8) | packet[35]);

                // Note on magnetometer (mag_x, mag_y, mag_z):
                // The on-package AK09916 3-axis magnetometer is interfaced via the ICM-20948's internal
                // auxiliary I2C bus. The DMP engine directly samples the magnetometer to perform 9-axis
                // sensor fusion, computing true heading for quat9_1..3 and quat9_accuracy.
                // Passing through raw microtesla (µT) readings into sample.mag_x/y/z requires configuring
                // auxiliary I2C slave channels (I2C_SLV0/I2C_SLV1), which is deferred to keep the Quat9 FIFO
                // bring-up clean and efficient.
                sample.mag_x = 0;
                sample.mag_y = 0;
                sample.mag_z = 0;
                return true;
            }
        }
    }

    // Timing Fallback:
    // If the FIFO does not have a complete 36-byte DMP packet ready yet (e.g., during the initial
    // milliseconds after boot before the first 55 Hz / 18 ms calculation finishes, or if the host
    // polls slightly faster than 55 Hz), fall back to reading the hardware ADC registers directly.
    // Design decision: We chose this fallback so the caller always receives fresh, real-time Accel
    // and Gyro readings without stalling or dropping a sample cycle, while leaving Quat9 and Accuracy
    // at 0 until the next fused FIFO packet arrives.
    uint8_t raw_buf[12];
    if (!spi_read_regs(kRegAccelXoutH, raw_buf, sizeof(raw_buf)))
    {
        return false;
    }

    sample.timestamp_ms = HAL_GetTick();

    sample.accel_x = static_cast<int16_t>((raw_buf[0] << 8) | raw_buf[1]);
    sample.accel_y = static_cast<int16_t>((raw_buf[2] << 8) | raw_buf[3]);
    sample.accel_z = static_cast<int16_t>((raw_buf[4] << 8) | raw_buf[5]);

    sample.gyro_x  = static_cast<int16_t>((raw_buf[6] << 8) | raw_buf[7]);
    sample.gyro_y  = static_cast<int16_t>((raw_buf[8] << 8) | raw_buf[9]);
    sample.gyro_z  = static_cast<int16_t>((raw_buf[10] << 8) | raw_buf[11]);

    // Raw mag is 0 (requires aux I2C master slave configuration; see note above)
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
