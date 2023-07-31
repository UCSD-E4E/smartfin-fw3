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
* @file MAX31725.h
*******************************************************************************
*/
#ifndef MAX31725_H
#define MAX31725_H
#include "mbed.h"

#define MAX31725_NO_ERROR   0
#define MAX31725_ERROR      -1

#define MAX31725_REG_TEMPERATURE      0X00
#define MAX31725_REG_CONFIGURATION    0X01
#define MAX31725_REG_THYST_LOW_TRIP   0X02 
#define MAX31725_REG_TOS_HIGH_TRIP    0X03 
#define MAX31725_REG_MAX              0X03

#define WAIT_MARGIN (0.0000)
#define MAX31725_WAIT_CONV_TIME  (.050+WAIT_MARGIN)

#define MAX31725_CFG_CONTINUOUS       (0X00 << 0)
#define MAX31725_CFG_SHUTDOWN         (0X01 << 0)

#define MAX31725_CFG_COMPARATOR_MODE  (0X00 << 1)
#define MAX31725_CFG_INTERRUPT_MODE   (0X01 << 1)

#define MAX31725_CFG_OS_POLARITY_ACT_LOW  (0x00 << 2)
#define MAX31725_CFG_OS_POLARITY_ACT_HIGH (0x01 << 2)

#define MAX31725_CFG_FAULT_FILTER_1   (0x00 << 3)
#define MAX31725_CFG_FAULT_FILTER_2   (0x01 << 3)
#define MAX31725_CFG_FAULT_FILTER_4   (0x02 << 3)
#define MAX31725_CFG_FAULT_FILTER_6   (0x03 << 3)

#define MAX31725_CFG_NORMAL_FORMAT    (0x00 << 5)
#define MAX31725_CFG_EXTENDED_FORMAT  (0x01 << 5)  /* add 64°C to temperature */
#define MAX31725_EXTENDED_FORMAT_OFFSET (64.0f)
#define MAX31725_CFG_TIMEOUT_ENABLE   (0X00 << 6)
#define MAX31725_CFG_TIMEOUT_DISABLE  (0X01 << 6)
#define MAX31725_CFG_ONE_SHOT_START   (0X01 << 7)

                                               /* A1  A2  A3  */
#define MAX31725_I2C_SLAVE_ADR_00 (0x90 >> 1)  /* GND GND GND */
#define MAX31725_I2C_SLAVE_ADR_01 (0x92 >> 1)  /* GND GND VDD */
#define MAX31725_I2C_SLAVE_ADR_02 (0x82 >> 1)  /* GND GND SCL */
#define MAX31725_I2C_SLAVE_ADR_03 (0x80 >> 1)  /* GND GND SDA */
#define MAX31725_I2C_SLAVE_ADR_04 (0x94 >> 1)  /* GND VDD GND */
                                              
#define MAX31725_I2C_SLAVE_ADR_05 (0x96 >> 1)  /* GND VDD VDD */
#define MAX31725_I2C_SLAVE_ADR_06 (0x86 >> 1)  /* GND VDD SCL */
#define MAX31725_I2C_SLAVE_ADR_07 (0x84 >> 1)  /* GND VDD SDA */
#define MAX31725_I2C_SLAVE_ADR_08 (0xB4 >> 1)  /* GND SCL GND */
#define MAX31725_I2C_SLAVE_ADR_09 (0xB6 >> 1)  /* GND SCL VDD */
                                              
#define MAX31725_I2C_SLAVE_ADR_10 (0xA6 >> 1)  /* GND SCL SCL */
#define MAX31725_I2C_SLAVE_ADR_11 (0xA4 >> 1)  /* GND SCL SDA */
#define MAX31725_I2C_SLAVE_ADR_12 (0xB0 >> 1)  /* GND SDA GND */
#define MAX31725_I2C_SLAVE_ADR_13 (0xB2 >> 1)  /* GND SDA VDD */
#define MAX31725_I2C_SLAVE_ADR_14 (0xA2 >> 1)  /* GND SDA SCL */
                                              
#define MAX31725_I2C_SLAVE_ADR_15 (0xA0 >> 1)  /* GND SDA SDA */
#define MAX31725_I2C_SLAVE_ADR_16 (0x98 >> 1)  /* VDD GND GND */
#define MAX31725_I2C_SLAVE_ADR_17 (0x9A >> 1)  /* VDD GND VDD */
#define MAX31725_I2C_SLAVE_ADR_18 (0x8A >> 1)  /* VDD GND SCL */
#define MAX31725_I2C_SLAVE_ADR_19 (0x88 >> 1)  /* VDD GND SDA */
                                              
#define MAX31725_I2C_SLAVE_ADR_20 (0x9C >> 1)  /* VDD VDD GND */
#define MAX31725_I2C_SLAVE_ADR_21 (0x9E >> 1)  /* VDD VDD VDD */
#define MAX31725_I2C_SLAVE_ADR_22 (0x8E >> 1)  /* VDD VDD SCL */
#define MAX31725_I2C_SLAVE_ADR_23 (0x8C >> 1)  /* VDD VDD SDA */
#define MAX31725_I2C_SLAVE_ADR_24 (0xBC >> 1)  /* VDD SCL GND */
                                              
#define MAX31725_I2C_SLAVE_ADR_25 (0xBE >> 1)  /* VDD SCL VDD */
#define MAX31725_I2C_SLAVE_ADR_26 (0xAE >> 1)  /* VDD SCL SCL */
#define MAX31725_I2C_SLAVE_ADR_27 (0xAC >> 1)  /* VDD SCL SDA */
#define MAX31725_I2C_SLAVE_ADR_28 (0xB8 >> 1)  /* VDD SDA GND */
#define MAX31725_I2C_SLAVE_ADR_29 (0xBA >> 1)  /* VDD SDA VDD */
                                              
#define MAX31725_I2C_SLAVE_ADR_30 (0xAA >> 1)  /* VDD SDA SCL */
#define MAX31725_I2C_SLAVE_ADR_31 (0xA8 >> 1)  /* VDD SDA SDA */


#define MAX31726_I2C_SLAVE_ADR_00 (0x98 >> 1)  /* GND GND */
#define MAX31726_I2C_SLAVE_ADR_01 (0x9A >> 1)  /* GND VDD */
#define MAX31726_I2C_SLAVE_ADR_02 (0x8A >> 1)  /* GND SCL */
#define MAX31726_I2C_SLAVE_ADR_03 (0x88 >> 1)  /* GND SDA */
#define MAX31726_I2C_SLAVE_ADR_04 (0x9C >> 1)  /* VDD GND */

#define MAX31726_I2C_SLAVE_ADR_05 (0x9E >> 1)  /* VDD VDD */
#define MAX31726_I2C_SLAVE_ADR_06 (0x8E >> 1)  /* VDD SCL */
#define MAX31726_I2C_SLAVE_ADR_07 (0x8C >> 1)  /* VDD SDA */
#define MAX31726_I2C_SLAVE_ADR_08 (0xBC >> 1)  /* SCL GND */
#define MAX31726_I2C_SLAVE_ADR_09 (0xBE >> 1)  /* SCL VDD */

#define MAX31726_I2C_SLAVE_ADR_10 (0xAE >> 1)  /* SCL SCL */
#define MAX31726_I2C_SLAVE_ADR_11 (0xAC >> 1)  /* SCL SDA */
#define MAX31726_I2C_SLAVE_ADR_12 (0xB8 >> 1)  /* SDA GND */
#define MAX31726_I2C_SLAVE_ADR_13 (0xBA >> 1)  /* SDA VDD */
#define MAX31726_I2C_SLAVE_ADR_14 (0xAA >> 1)  /* SDA SCL */

#define MAX31726_I2C_SLAVE_ADR_15 (0xA8 >> 1)  /* SDA SDA */


#define MAX31725_CF_LSB           (0.00390625F)

/** @union max31725_raw_data
 * @brief union data structure for byte word manipulations
 */
union max31725_raw_data {
    struct {
        uint8_t lsb;
        uint8_t msb;
    };
    struct {
        uint16_t magnitude_bits:15;
        uint16_t sign_bit:1;
    };
    uint16_t uwrd;
    int16_t swrd;
};

#endif/* MAX31875_H */