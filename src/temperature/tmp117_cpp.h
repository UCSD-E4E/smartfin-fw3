/*******************************************************************************
* TI TMP117 Temperature Sensor Driver
*
* @file tmp117_cpp.h
* @brief C++ interface for TMP117 digital thermometer
*******************************************************************************
*/
#ifndef TMP117_CPP_H
#define TMP117_CPP_H
#include "i2c/mbed.h"

/**
 * @brief Digital thermometer and thermostat temperature sensor.
 * @version 1.0000.0001
 *
 * @details The TMP117 is a digital temperature sensor with high accuracy
 * and low power consumption.
 * Accuracy is ±0.1°C from -20°C to +50°C.
 * Operating temperature: -55°C to +150°C.
 * VDD: 1.62V to 5.5V.
 *
 * @code 
 * #include "i2c/mbed.h"
 * #include "tmp117_cpp.h"
 * I2C i2cBus(SDA, SCL);
 * int main()
 * {
 * float temperature;
 * TMP117 temp_sensor(i2cBus, TMP117_I2CADDR_DEFAULT);
 * i2cBus.frequency(400000);
 * temperature = temp_sensor.read_reg_as_temperature(TMP117_TEMP_DATA);
 * printf("Temperature = %3.4f Celsius, %3.4f Fahrenheit\r\n", 
 * temperature, temp_sensor.celsius_to_fahrenheit(temperature));
 * }
 * @endcode
 */

class TMP117
{
    public:

    /**********************************************************//**
     * @brief Constructor for TMP117 Class.  
     * * @details Allows user to use existing I2C object
     *
     * On Entry:
     * @param[in] i2c_bus - reference to existing I2C object
     * @param[in] slave_address - 7-bit slave address of TMP117
     *
     * On Exit:
     *
     * @return None
     **************************************************************/
    TMP117(I2C &i2c_bus, uint8_t slave_address);
 
    /**********************************************************//**
     * @brief Default destructor for TMP117 Class.  
     *
     * @details Destroys I2C object if owner 
     *
     * On Entry:
     *
     * On Exit:
     *
     * @return None
     **************************************************************/
    ~TMP117();

    /**
     * @brief  Read configuration register
     * @param[out] value - Read data on success
     * @return 0 on success, negative number on failure
     */
    int read_cfg_reg(uint8_t *value);

    /**
     * @brief  Read 16-bit register of device at slave address
     * @param[out] value - Read data on success
     * @param reg - Register address
     * @return 0 on success, negative number on failure
     */
    int read_reg16(int16_t *value, char reg);

    /**
     * @brief Reads the temperature registers
     * @param reg - the address of the temperature register
     * @return temperature in degrees Celsius, or NAN if error
     */
    float read_reg_as_temperature(uint8_t reg);

    /** * @brief Writes to the configuration register
     * @param cfg - configuration word
     * @return 0 on success, negative number on failure
     */
    int write_cfg_reg(uint8_t cfg);

    /** * @brief Writes to the low threshold register
     * @param temperature - the temperature in Celsius degrees
     * @return 0 on success, negative number on failure
     */
    int write_low_threshold(float temperature);

    /** * @brief Writes to the high threshold register
     * @param temperature - the temperature in Celsius degrees
     * @return 0 on success, negative number on failure
     */
    int write_high_threshold(float temperature);

    /** * @brief Converts Celsius degrees to Fahrenheit
     * @param temp_c - the temperature in Celsius degrees
     * @return temperature in Fahrenheit degrees
     */
    float celsius_to_fahrenheit(float temp_c);

protected: 
    /** * @brief Write a value to a register
     * @param value - value to write to the register
     * @param reg - register address
     * @return 0 on success, negative number on failure
     */
    int write_reg16(int16_t value, char reg);

private:
    /** @var m_i2c
     * @brief I2C object reference
     */
    I2C &m_i2c;
    /** @var m_write_address, m_read_address
     * @brief I2C address (write bit and read bit variants)
     */
    uint8_t m_write_address, m_read_address;
};

#endif/* TMP117_CPP_H */