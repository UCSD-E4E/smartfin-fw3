/***************************************************************************//**
 * @file ICM20648.h
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2017 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************/

#ifndef ICM20648_H
#define ICM20648_H

#include "../i2c/mbed.h"
#include "../product.hpp"

/** ICM20648 class.
 *  Used for taking accelerometer and gyroscope measurements.
 *
 * Example:
 * @code
 * #include "mbed.h"
 * #include "ICM20648.h"
 *
 * //Create an ICM20648 object
 * ICM20648 sensor(PC4, PC5);
 *
 * int main()
 * {
 *     //Try to open the ICM20648
 *     if (sensor.open()) {
 *         printf("Device detected!\n");
 *
 *         while (1) {
 *             float acc_x, acc_y, acc_z;
 *             float gyr_x, gyr_y, gyr_z;
 *             
 *             sensor.measure();
 *
 *             sensor.get_accelerometer(&acc_x, &acc_y, &acc_z);
 *             sensor.get_gyroscope(&gyr_x, &gyr_y, &gyr_z);
 *
 *             //Print the current accelerometer measurement
 *             printf("acc: %.3f  %.3f  %.3f\n", acc_x, acc_y, acc_z);
 *             //Print the current gyroscope measurement
 *             printf("gyr: %.3f  %.3f  %.3f\n", gyr_x, gyr_y, gyr_z);
 *
 *             //Sleep for 0.5 seconds
 *             wait(0.5);
 *         }
 *     } else {
 *         error("Device not detected!\n");
 *     }
 * }
 * @endcode
 */
class ICM20648
{
public:

#if PARTICLE_IO
    /** Create an ICM20648 object with the specified I2C address
     *
     * @param address I2C address.
     */
    ICM20648(uint8_t address);
#else
    /** Create an ICM20648 object connected to the specified SPI pins
     *
     * @param mosi The SPI MOSI pin.
     * @param miso The SPI MISO pin.
     * @param sclk The SPI clock pin.
     * @param cs The SPI Chip Select pin.
     * @param irq The ICM20648 irq pin.
     */
    ICM20648(PinName mosi, PinName miso, PinName sclk, PinName cs, PinName irq = NC);
#endif

    /**
    * ICM20648 destructor
    */
    ~ICM20648(void);

    /** Probe for the ICM20648 and try to initialize the sensor
     *
     * @returns
     *   'true' if the device exists on the bus,
     *   'false' if the device doesn't exist on the bus.
     */
    bool open();

    /** Perform a measurement
     *
     * @returns true if measurement was successful
     */
    bool measure();

    /** Do a measurement on the gyroscope
     *
     * @param[out] gyr_x Gyroscope measurement on X axis
     * @param[out] gyr_y Gyroscope measurement on Y axis
     * @param[out] gyr_z Gyroscope measurement on Z axis
     *
     * @returns true if measurement was successful
     */
    bool get_gyroscope(float *gyr_x, float *gyr_y, float *gyr_z);

    /** Do a measurement on the accelerometer
     *
     * @param[out] acc_x Accelerometer measurement on X axis
     * @param[out] acc_y Accelerometer measurement on Y axis
     * @param[out] acc_z Accelerometer measurement on Z axis
     *
     * @returns true if measurement was successful
     */
    bool get_accelerometer(float *acc_x, float *acc_y, float *acc_z);

    /** Do a measurement of the temperature sensor
     *
     * @param[out] temperature Measured temperature
     *
     * @returns true if measurement was successful
     */
    bool get_temperature(float *temperature);

#if PARTICLE_IO
    void close(void);
    void get_accel_raw_data(uint8_t *data);
    void get_gyro_raw_data(uint8_t *data);
#endif

private:
    /* Private functions */
    void     read_register(uint16_t addr, int numBytes, uint8_t *data);
    void     write_register(uint16_t addr, uint8_t data);
    void     select_bank(uint8_t bank);
    uint32_t reset(void);
    uint32_t set_sample_rate(float sampleRate);
    float    set_gyro_sample_rate(float sampleRate);
    float    set_accel_sample_rate(float sampleRate);
    uint32_t set_gyro_bandwidth(uint8_t gyroBw);
    uint32_t set_accel_bandwidth(uint8_t accelBw);
    uint32_t read_accel_data(float *accel);
    uint32_t read_gyro_data(float *gyro);
    uint32_t get_accel_resolution(float *accelRes);
    uint32_t get_gyro_resolution(float *gyroRes);
    uint32_t set_accel_fullscale(uint8_t accelFs);
    uint32_t set_gyro_fullscale(uint8_t gyroFs);
    uint32_t enable_sleepmode(bool enable);
    uint32_t enable_cyclemode(bool enable);
    uint32_t enable_sensor(bool accel, bool gyro, bool temp);
    uint32_t enter_lowpowermode(bool enAccel, bool enGyro, bool enTemp);
    uint32_t enable_irq(bool dataReadyEnable, bool womEnable);
    uint32_t read_irqstatus(uint32_t *int_status);
    bool     is_data_ready(void);
    uint32_t enable_wake_on_motion(bool enable, uint8_t womThreshold, float sampleRate);
    uint32_t calibrate(float *accelBiasScaled, float *gyroBiasScaled);
    uint32_t calibrate_gyro(float *gyroBiasScaled);
    uint32_t read_temperature(float *temperature);
    uint32_t get_device_id(uint8_t *device_id);

    void irq_handler(void);

    /* Member variables */
#if PARTICLE_IO
    I2C m_I2C;
    uint8_t m_address;
    uint8_t m_accelRawData[6];
    uint8_t m_gyroRawData[6];
#else
    SPI m_SPI;
    DigitalOut m_CS;
    InterruptIn m_IRQ;
#endif
};

#endif