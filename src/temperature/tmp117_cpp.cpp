#include "cli/conio.hpp"
#include "consts.hpp"
#include "i2c/mbed.h"
#include "tmp117.h"
#include "tmp117_cpp.h"

#include <cmath>

#if PARTICLE_IO
#define printf Serial.printf
#endif

/******************************************************************************
 * C++ version for TMP117 driver                                             *
 ******************************************************************************
 */

/******************************************************************************/
TMP117::TMP117(I2C &i2c_bus, uint8_t slave_address):
m_i2c(i2c_bus), 
m_write_address(slave_address <<1),
m_read_address ((slave_address << 1) | 1)
{
    // removed undeclared tmp117_extended_format variable
}
 
/******************************************************************************/
TMP117::~TMP117(void) 
{
  /** empty block */
}

/******************************************************************************/
int TMP117::read_cfg_reg(uint16_t *value) 
{
    
    int32_t ret;
    char data[2] = {0, 0};
    char reg = TMP117_CONFIGURATION;
    tmp117_raw_data tmp;

    /* write to the Register Select, true is for repeated start */
    ret = m_i2c.write(m_write_address, &reg, 1, true);
    if (ret == 0) {
        ret = m_i2c.read(m_read_address, data, 2, false);
        if (ret == 0) {
            tmp.msb = data[0];
            tmp.lsb = data[1];
            *value = tmp.uwrd;
            return TMP117_NO_ERROR;
        } else {
            SF_OSAL_printf(
                "%s: failed to read data: ret: %ld\r" __NL__, __func__, ret);
            }
    } else {                
        SF_OSAL_printf("%s: failed to write to Register Select: ret: %ld\r" __NL__,
            __func__, ret);
    }
    return TMP117_ERROR;
}

/******************************************************************************/
int TMP117::read_device_id(uint16_t *value)
{
    int32_t ret;
    char data[2] = {0, 0};
    char reg = TMP117_DEVICE_ID;
    tmp117_raw_data tmp;

    /* write to the Register Select, true is for repeated start */
    ret = m_i2c.write(m_write_address, &reg, 1, true);
    if (ret == 0) {
        /* read the two bytes of data */
        ret = m_i2c.read(m_read_address, data, 2, false);
        if (ret == 0) {
            tmp.msb = data[0];
            tmp.lsb = data[1];
            *value = tmp.uwrd;
            return TMP117_NO_ERROR;
        } else {
            SF_OSAL_printf("%s: failed to read device ID: ret: %ld\r" __NL__,
                           __func__, ret);
        }
    } else {
        SF_OSAL_printf("%s: failed to select device ID register: ret: %ld\r" __NL__,
                       __func__, ret);
    }
    return TMP117_ERROR;
}

/******************************************************************************/
int TMP117::read_reg16(int16_t *value, char reg) 
{
    int32_t ret;
    char data[2] = {0, 0};
    tmp117_raw_data tmp;
     
    if (reg == TMP117_TEMP_DATA || 
        reg == TMP117_T_LOW_LIMIT || reg == TMP117_T_HIGH_LIMIT) {
        /* write to the Register Select, true is for repeated start */
        ret = m_i2c.write(m_write_address, &reg, 1, true);
        /* read the two bytes of data */
        if (ret == 0) {
            ret = m_i2c.read(m_read_address, data, 2, false);
            if (ret == 0) {
                tmp.msb = data[0];
                tmp.lsb = data[1];
                *value = tmp.swrd;
                return TMP117_NO_ERROR;
            } else {
                SF_OSAL_printf("%s: failed to read data: ret: %ld\r" __NL__, __func__, ret);
            }
        } else {                
            SF_OSAL_printf("%s: failed to write to Register Select: ret: %ld\r" __NL__,
                __func__, ret);
        }
    } else {
        SF_OSAL_printf("%s: register address is not correct: register: %d\r" __NL__,
                __func__, reg);
    }                
    return TMP117_ERROR;
}

/******************************************************************************/
float TMP117::read_reg_as_temperature(uint8_t reg)
{
    tmp117_raw_data tmp;
    float temperature;
    if (reg == TMP117_TEMP_DATA || 
        reg == TMP117_T_LOW_LIMIT || reg == TMP117_T_HIGH_LIMIT) {
        read_reg16(&tmp.swrd, reg);
        temperature = (float)tmp.swrd;  /* values are 2's complement */
        temperature *= TMP117_RESOLUTION;
        return temperature;
    } else {
        SF_OSAL_printf("%s: register is invalid, %d r" __NL__, __func__, reg);
        return NAN;
    }
}

/******************************************************************************/
int TMP117::write_reg16(int16_t value, char reg) 
{
    int32_t ret;
    char cmd[3];
    tmp117_raw_data tmp;

    if (reg >= TMP117_T_LOW_LIMIT && reg <= TMP117_T_HIGH_LIMIT) {
        cmd[0] = reg;
        tmp.swrd = value;
        cmd[1] = tmp.msb;
        cmd[2] = tmp.lsb;
        ret = m_i2c.write(m_write_address, cmd, 3, false);
        if (ret == 0) {
            return TMP117_NO_ERROR;
        } else {
            SF_OSAL_printf("Write error" __NL__);
            SF_OSAL_printf("%s: I2C write error %ld\r" __NL__,__func__, ret);
            return TMP117_ERROR;
        }
    } else {
        SF_OSAL_printf("%s: register value invalid %x\r" __NL__,__func__, reg);
        return TMP117_ERROR;
    }
}


/******************************************************************************/
int TMP117::write_cfg_reg(uint16_t cfg)
{
    int32_t ret;
    char cmd[3];
    tmp117_raw_data tmp;

    cmd[0] = TMP117_CONFIGURATION;
    tmp.uwrd = cfg;
    cmd[1] = tmp.msb;
    cmd[2] = tmp.lsb;
    ret = m_i2c.write(m_write_address, cmd, 3, false);

    if (ret == 0) {
        return TMP117_NO_ERROR;
    } else {
        SF_OSAL_printf("%s: I2C write error %ld\r" __NL__,__func__, ret);
        return TMP117_ERROR;
    }
}

/******************************************************************************/
int TMP117::write_low_threshold(float temperature)
{
    tmp117_raw_data raw;
    temperature /= TMP117_RESOLUTION;
    raw.swrd = int16_t(temperature);
    return write_reg16(raw.swrd, TMP117_T_LOW_LIMIT);
}

/******************************************************************************/
int TMP117::write_high_threshold(float temperature)
{
    tmp117_raw_data raw;
    temperature /= TMP117_RESOLUTION;
    raw.swrd = int16_t(temperature);
    return write_reg16(raw.swrd, TMP117_T_HIGH_LIMIT);
}

/******************************************************************************/
float TMP117::celsius_to_fahrenheit(float temp_c)
{
    float temp_f;
    temp_f = ((temp_c * 9)/5) + 32;
    return temp_f;
}