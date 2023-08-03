/*
 * Copyright (c) 2020 Particle Industries, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "Particle.h"

//-----------------------------------------------------------------------------
// Tracker platform
//-----------------------------------------------------------------------------

#if ( (PLATFORM_ID != PLATFORM_TRACKER) && (PLATFORM_ID != PLATFORM_TRACKERM) )
#error "Platform not supported"
#endif

//
// Tracker types
//
#define TRACKER_MODEL_BARE_SOM_DEFAULT        (0xffff)
#define TRACKER_MODEL_BARE_SOM                (0x0000)
#define TRACKER_MODEL_EVAL                    (0x0001)
#define TRACKER_MODEL_TRACKERONE              (0x0002)
#define TRACKER_MODEL_MONITORONE              (0x0003)
#define TRACKER_MODEL_TRACKERM                (0x0004)


//
// Variables that can be passed in through compile flags (not Workbench)
//
// Simple macro to check if the product ID needs to be specified
#define TRACKER_PRODUCT_NEEDED                ((SYSTEM_VERSION >> 24) == 3)

#ifndef TRACKER_PRODUCT_ID
#define TRACKER_PRODUCT_ID                    (PLATFORM_ID)
#endif

#ifndef TRACKER_PRODUCT_VERSION
#define TRACKER_PRODUCT_VERSION               (119)
#endif

#if ( (SYSTEM_VERSION < SYSTEM_VERSION_ALPHA(5, 0, 0, 1)) && (PLATFORM_ID == PLATFORM_TRACKER) )
#define TRACKER_USE_MEMFAULT
#endif // SYSTEM_VERSION

//
// Pin and interface mapping
//
#if (PLATFORM_ID == PLATFORM_TRACKER)
    #define BMI_SPI_INTERFACE                 (SPI1)
    #define BMI_SPI_CS_PIN                    (SEN_CS)
    #define BMI_INT_PIN                       (SEN_INT)
#elif (PLATFORM_ID == PLATFORM_TRACKERM)
    #define BMI_SPI_INTERFACE                 (SPI)
    #define BMI_SPI_CS_PIN                    (Y3)
    #define BMI_INT_PIN                       (IO_EXP_B0)
#endif
#define BMI_INT_MODE                          (FALLING)

#define UBLOX_SPI_INTERFACE                   (SPI1)
#if (PLATFORM_ID == PLATFORM_TRACKER)
    #define UBLOX_CS_PIN                      (GPS_CS)
    #define UBLOX_PWR_EN_PIN                  (GPS_PWR)
    #define UBLOX_RESETN_PIN                  (GPS_RST)
    #define UBLOX_TX_READY_MCU_PIN            (GPS_INT)
#elif (PLATFORM_ID == PLATFORM_TRACKERM)
    #define UBLOX_CS_PIN                      (PIN_INVALID)
    #define UBLOX_PWR_EN_PIN                  (GNSS_PWR_EN)
    #define UBLOX_RESETN_PIN                  (GNSS_RST)
    #define UBLOX_TX_READY_MCU_PIN            (PIN_INVALID)
#endif
#define UBLOX_TX_READY_GPS_PIN                (14) // PIO 14 is EXTINT on GPS Module

#if (PLATFORM_ID == PLATFORM_TRACKER)
    #define QUECTEL_GNSS_I2C_INTERFACE        (Wire1)
    #define QUECTEL_GNSS_PWR_EN_PIN           (GPS_PWR)
    #define QUECTEL_GNSS_WAKEUP_PIN           (GPS_RST)
#elif (PLATFORM_ID == PLATFORM_TRACKERM)
    #define QUECTEL_GNSS_I2C_INTERFACE        (Wire)
    #define QUECTEL_GNSS_PWR_EN_PIN           (GNSS_PWR_EN)
    #define QUECTEL_GNSS_WAKEUP_PIN           (GNSS_RST)
#endif

#define ESP32_SPI_INTERFACE                   (SPI1)
#if (PLATFORM_ID == PLATFORM_TRACKER)
    #define ESP32_CS_PIN                      (WIFI_CS)
    #define ESP32_BOOT_MODE_PIN               (WIFI_BOOT)
    #define ESP32_PWR_EN_PIN                  (WIFI_EN)
    #define ESP32_INT_PIN                     (WIFI_INT)
#endif

#if (PLATFORM_ID == PLATFORM_TRACKER)
    #define MCP_CAN_SPI_INTERFACE             (SPI1)
    #define MCP_CAN_PWR_EN_PIN                (CAN_PWR)
    #define MCP_CAN_RESETN_PIN                (CAN_RST)
    #define MCP_CAN_CS_PIN                    (CAN_CS)
    #define MCP_CAN_INT_PIN                   (CAN_INT)
    #define MCP_CAN_STBY_PIN                  (CAN_STBY)
#elif (PLATFORM_ID == PLATFORM_TRACKERM)
    #define MCP_CAN_SPI_INTERFACE             (SPI)
    #define MCP_CAN_PWR_EN_PIN                (A0)
    #define MCP_CAN_RESETN_PIN                (IO_EXP_B2)
    #define MCP_CAN_CS_PIN                    (Y4)
    #define MCP_CAN_INT_PIN                   (IO_EXP_B1)
    #define MCP_CAN_STBY_PIN                  (IO_EXP_B3)
#endif


//
// Tracker One Specifc IO
//
#define TRACKER_THERMISTOR                    (A0)
#define TRACKER_USER_BUTTON                   (D1)
#if (PLATFORM_ID == PLATFORM_TRACKER)
    #define TRACKER_GNSS_LOCK_LED             (D2)
#elif (PLATFORM_ID == PLATFORM_TRACKERM)
    #define TRACKER_GNSS_LOCK_LED             (PIN_INVALID)
#endif

#if (PLATFORM_ID == PLATFORM_TRACKER)
    //#define RTC_WDT_DISABLE // Optional define for Tracker
#elif (PLATFORM_ID == PLATFORM_TRACKERM)
    #define RTC_WDT_DISABLE   // Always needs to be disabled for Tracker-M
#endif
#define TRACKER_89503_THERMISTOR              (A3)
#define TRACKER_89503_USER_BUTTON             (D2)
#define TRACKER_89503_VIN_EN_PIN              (NFC_PIN2)

#define TRACKER_89503_STS3X_I2C_INSTANCE      (Wire)
#define TRACKER_89503_STS3X_I2C_ADDR          (0x4a)

#define TRACKER_89503_ADP8866_I2C_INSTANCE    (Wire)
#define TRACKER_89503_ADP8866_I2C_ADDR        (0x27)
#define TRACKER_89503_ADP8866_RESETN_PIN      (NFC_PIN1)

//#define RTC_WDT_DISABLE
