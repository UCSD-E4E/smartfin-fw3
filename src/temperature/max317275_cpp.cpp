/*******************************************************************************
* Copyright (C) 2019 Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
*******************************************************************************
*/
#include "max31725.h"
#include "max31725_cpp.h"
#include "mbed.h" 
// #include "USBSerial.h"

#if PARTICLE_IO
#define printf Serial.printf
#endif

/******************************************************************************
 *  C++ version for MAX31725 driver                                           *
 ******************************************************************************
 */

/******************************************************************************/
MAX31725::MAX31725(I2C &i2c_bus, uint8_t slave_address):
m_i2c(i2c_bus), 
m_write_address(slave_address <<1),
m_read_address ((slave_address << 1) | 1)
{
        max31725_extended_format = 0;
}
 
/******************************************************************************/
MAX31725::~MAX31725(void) 
{
  /** empty block */
}

/******************************************************************************/
int MAX31725::read_cfg_reg(uint8_t *value) 
{
    int32_t ret;
    char data[1] = {0};
    char reg = MAX31725_REG_CONFIGURATION;

    /* write to the Register Select, true is for repeated start */
    ret = m_i2c.write(m_write_address, &reg, 1, true);
    if (ret == 0) {
        ret = m_i2c.read(m_read_address, data, 1, false);
        if (ret == 0) {
            *value = data[0];
            return MAX31725_NO_ERROR;
        } else {
            printf(
                "%s: failed to read data: ret: %ld\r\n", __func__, ret);
            }
    } else {                
        printf("%s: failed to write to Register Select: ret: %ld\r\n",
            __func__, ret);
    }
    return MAX31725_ERROR;
}

/******************************************************************************/
int MAX31725::read_reg16(int16_t *value, char reg) 
{
    int32_t ret;
    char data[2] = {0, 0};
    max31725_raw_data tmp;
     
    if (reg == MAX31725_REG_TEMPERATURE || 
        reg == MAX31725_REG_THYST_LOW_TRIP || reg == MAX31725_REG_TOS_HIGH_TRIP) {
        /* write to the Register Select, true is for repeated start */
        ret = m_i2c.write(m_write_address, &reg, 1, true);
        /* read the two bytes of data */
        if (ret == 0) {
            ret = m_i2c.read(m_read_address, data, 2, false);
            if (ret == 0) {
                tmp.msb = data[0];
                tmp.lsb = data[1];
                *value = tmp.swrd;
                return MAX31725_NO_ERROR;
            } else {
                printf(
                    "%s: failed to read data: ret: %ld\r\n", __func__, ret);
            }
        } else {                
            printf("%s: failed to write to Register Select: ret: %ld\r\n",
                __func__, ret);
        }
    } else {
        printf("%s: register address is not correct: register: %d\r\n",
                __func__, reg);
    }                
    return MAX31725_ERROR;
}

/******************************************************************************/
float MAX31725::read_reg_as_temperature(uint8_t reg)
{
    max31725_raw_data tmp;
    float temperature;
    if (reg == MAX31725_REG_TEMPERATURE ||
        reg == MAX31725_REG_THYST_LOW_TRIP || reg == MAX31725_REG_TOS_HIGH_TRIP) {
        read_reg16(&tmp.swrd, reg);
        temperature = (float)tmp.swrd;  /* values are 2's complement */
        temperature *= MAX31725_CF_LSB;
        if (reg == MAX31725_REG_TEMPERATURE && max31725_extended_format)
            temperature += MAX31725_EXTENDED_FORMAT_OFFSET;

        return temperature;
    } else {
        printf("%s: register is invalid, %d r\n", __func__, reg);
        return 0;
    }
}

/******************************************************************************/
int MAX31725::write_reg16(int16_t value, char reg) 
{
    int32_t ret;
    char cmd[3];
    max31725_raw_data tmp;

    if (reg >= MAX31725_REG_THYST_LOW_TRIP && reg <= MAX31725_REG_MAX) {
        cmd[0] = reg;
        tmp.swrd = value;
        cmd[1] = tmp.msb;
        cmd[2] = tmp.lsb;
        ret = m_i2c.write(m_write_address, cmd, 3, false);
        if (ret == 0) {
            return MAX31725_NO_ERROR;
        } else {
            printf("%s: I2C write error %ld\r\n",__func__, ret);
            return MAX31725_ERROR;
        }
    } else {
        printf("%s: register value invalid %x\r\n",__func__, reg);
        return MAX31725_ERROR;
    }
}


/******************************************************************************/
int MAX31725::write_cfg_reg(uint8_t cfg)
{
    int32_t ret;
    char cmd[2];

    cmd[0] = MAX31725_REG_CONFIGURATION;
    cmd[1] = cfg;
    ret = m_i2c.write(m_write_address, cmd, 2, false);
    if (ret == 0) {
        max31725_extended_format = 0;
        if (cfg & MAX31725_CFG_EXTENDED_FORMAT)
            max31725_extended_format = 1;

        return MAX31725_NO_ERROR;
    } else {
        printf("%s: I2C write error %ld\r\n",__func__, ret);
        return MAX31725_ERROR;
    }
}

/******************************************************************************/
int MAX31725::write_trip_low_thyst(float temperature)
{
    max31725_raw_data raw;
    temperature /= MAX31725_CF_LSB;
    raw.swrd = int16_t(temperature);
    return write_reg16(raw.swrd, MAX31725_REG_THYST_LOW_TRIP);
}

/******************************************************************************/
int MAX31725::write_trip_high_tos(float temperature)
{
    max31725_raw_data raw;
    temperature /= MAX31725_CF_LSB;
    raw.swrd = int16_t(temperature);
    return write_reg16(raw.uwrd, MAX31725_REG_TOS_HIGH_TRIP);
}

/******************************************************************************/
float MAX31725::celsius_to_fahrenheit(float temp_c)
{
    float temp_f;
    temp_f = ((temp_c * 9)/5) + 32;
    return temp_f;
}