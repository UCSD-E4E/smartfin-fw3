/**
 * @file product.hpp
 * @author Nathan Hui (nthui@ucsd.edu)
 * @brief Product configuration header
 * @version 0.1
 * @date 2024-11-01
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef __PRODUCT_HPP__
#define __PRODUCT_HPP__

/******************************************************************************
 * Pin Definitions
 *****************************************************************************/
/**
 * USB Power Detection Pin TODO
 */

#define SF_USB_PWR_DETECT_PIN   A4

/**
 * Pin for the Battery Status LED
 */
#define STAT_LED_PIN          A5

/**
 * Water Detect Enable Pin
 */
#define WATER_DETECT_EN_PIN   A2
/**
 * Water Detect Pin
 */
#define WATER_DETECT_PIN      A6

/**
 * @brief Water Detection Status LED
 *
 */
#define WATER_STATUS_LED D9
/**
 * @brief Manufacturing Water Detect Pin
 *
 */
#define WATER_MFG_TEST_EN     A3


/**
 * @brief ICM20648 Address
 * 
 */
#define SF_ICM20648_ADDR    (0x68 << 1)

/**
 * @brief Wakeup pin 
 * 
 */
#define WKP_PIN               A7


/*******************************************************************************
 * Peripheral Configurations
 ******************************************************************************/
#define SERIAL_DEBUG_BAUD_RATE 115200
/**
 * SPI Flash Size
 */
#define SF_FLASH_SIZE_MB    4

/**
 * window sizes are how many water detect samples are looked at in a moving 
 * average to determine if we are in or out of the water.  Generally a sample
 * happens 1/second
 */
#define WATER_DETECT_SURF_SESSION_INIT_WINDOW   40

/**
 * How long (in us) to turn on water detection circuit when looking for water
 */
#define WATER_DETECT_EN_TIME_US     1000

/**
 * Charging voltage (mV)
 */
#define SF_CHARGE_VOLTAGE   4112

/**
 * @brief Below what battery voltage should the system shutdown
 * 
 */
#define SF_BATTERY_SHUTDOWN_VOLTAGE 3.0

/**
 * @brief Particle IO device
 * 
 */
#define PARTICLE_IO 1
/**
 * @brief hardware revision
 * 
 */
#define HARDWARE_REV 3


/*******************************************************************************
 * System Configuration
 ******************************************************************************/

#define RIDE_DATA_DIR "ridedata"

#define LITTLEFS_OBJ_NAME_LEN 31

#define USE_ICM_TEMP_SENSOR 0

/**
 * how long to stay in CLI mode if there are no key presses at all
 */
#define CLI_NO_INPUT_TIMEOUT_MS 600000

/**
 * The default state that the Smartfin comes up in
 */
#define SF_DEFAULT_STATE   STATE_CHARGE

/**
 * The CLI RGB LED Color
 */
#define SF_CLI_RGB_LED_COLOR        RGB_COLOR_RED
#define SF_CLI_RGB_LED_PATTERN      LED_PATTERN_SOLID
#define SF_CLI_RGB_LED_PERIOD       3000
#define SF_CLI_RGB_LED_PRIORITY     LED_PRIORITY_IMPORTANT

/**
 * The Ride RGB LED Color
 */
#define RIDE_RGB_LED_COLOR RGB_COLOR_WHITE
#define RIDE_RGB_LED_PATTERN_GPS LED_PATTERN_BLINK
#define RIDE_RGB_LED_PERIOD_GPS 500
#define RIDE_RGB_LED_PATTERN_NOGPS LED_PATTERN_SOLID
#define RIDE_RGB_LED_PERIOD_NOGPS 0
#define RIDE_RGB_LED_PRIORITY LED_PRIORITY_IMPORTANT

/**
 * The Data Upload RGB LED Color
 */
#define SF_DUP_RGB_LED_COLOR        RGB_COLOR_BLUE
#define SF_DUP_RGB_LED_PERIOD       500
#define SF_DUP_RGB_LED_PRIORITY LED_PRIORITY_IMPORTANT
#define SF_DUP_RGB_LED_PATTERN LED_PATTERN_BLINK
#define SF_DUP_CONNECT_RGB_LED_PATTERN LED_PATTERN_FADE

#define SF_TCAL_RGB_LED_COLOR       RGB_COLOR_ORANGE
#define SF_TCAL_RGB_LED_PATTERN     LED_PATTERN_FADE
#define SF_TCAL_RGB_LED_PERIOD      3000
#define SF_TCAL_RGB_LED_PRIORITY    LED_PRIORITY_IMPORTANT

/**
 * Minimum battery voltage to start an upload
 */ 
#define SF_BATTERY_UPLOAD_VOLTAGE 3.6

/**
 * how long to stay in data upload without a cell signal/connection/succesful upload
 */
#define SF_CELL_SIGNAL_TIMEOUT_MS 300000

/**
 * Max size of the window/how large the FIFO array is
 */
#define WATER_DETECT_ARRAY_SIZE 200

/**
 * @brief How long to stay in session init while waiting to get into water
 * before going to sleep
 *
 */
#define SURF_SESSION_GET_INTO_WATER_TIMEOUT_MS 300000

/**
 * @brief Flag allowing deploy while in charge
 *
 * This should not be allowed in production, but is useful during debug.  Set
 * to 0 for production
 *
 */
#define SF_CHARGE_ALLOW_DEPLOY 1

/**
 * @brief Seconds to sleep between upload attempts
 * 
 */
#define SF_UPLOAD_REATTEMPT_DELAY_SEC 600
 /**
  * @brief Milliseconds between transmit attempts
  *
  */
#define SF_UPLOAD_MS_PER_TRANSMIT   1000
/**
 * @brief how many ms is a GPS data point valid for a given data log
 * 
 */
#define GPS_AGE_VALID_MS 5000


/**
 * @brief How long to wait for a cell connection in during manufacturing test
 * 
 */
#define MANUFACTURING_CELL_TIMEOUT_MS 180000

/**
 * @brief A voltage that's slightly higher than the max battery voltage
 * 
 */
#define SF_BATTERY_MAX_VOLTAGE 4.3

/**
 * @brief Lost Bird Smartfin Z7 Product ID
 * 
 */
#define PRODUCT_ID_SMARTFIN_Z7  8977
/**
 * @brief UCSD Smartfin Product ID
 * 
 */
#define PRODUCT_ID_UCSD_SMARTFIN    17293

/**
 * @brief Set to use a hexadecimal product version, otherwise use a decimal
 * product version
 * 
 */
#define PRODUCT_VERSION_USE_HEX 0

/**
 * @brief Enable initialization delay
 * 
 */
// #define SF_ENABLE_DEBUG_DELAY   15

/**
 * @brief Base85 encoding flag
 * 
 */
#define SF_UPLOAD_BASE85 1
/**
 * @brief Base64 encoding flag
 * 
 */
#define SF_UPLOAD_BASE64 2
/**
 * @brief Base64url encoding flag
 * 
 */
#define SF_UPLOAD_BASE64URL 3

#define SF_UPLOAD_ENCODING SF_UPLOAD_BASE64URL


#if SF_UPLOAD_ENCODING == SF_UPLOAD_BASE85
 /**
  * How many bytes to store chunks of data in on the SPI flash.
  *
  * 816 * 5/4 (base85 encoding compression rate) = 1020 which is less than 1024
  * bytes which is the maximum size of publish events.
  */
#define SF_PACKET_SIZE  816
#define SF_RECORD_SIZE  1020
#elif SF_UPLOAD_ENCODING == SF_UPLOAD_BASE64 || SF_UPLOAD_ENCODING == SF_UPLOAD_BASE64URL
 /**
  * How many bytes to store chunks of data in on the SPI flash.
  *
  * 768 * 4/3 (base64 encoding compression rate) = 1024 which is the maximum size
  * of publish events.
  */
#define SF_PACKET_SIZE  768
#define SF_RECORD_SIZE  1024
#endif



#define SF_SERIAL_SPEED 9600

#define SF_CLI_MAX_CMD_LEN 100


#define SF_NAME_MAX 64

/**
 * @brief Maximum number of attempts to connect to the cloud
 * 
 */
#define SF_CLOUD_CONNECT_MAX_ATTEMPTS   5

/**
 * @brief Particle Platform Selector
 *
 */
#define SF_PLATFORM_PARTICLE 1
/**
 * @brief GCC Platform Selector
 *
 */
#define SF_PLATFORM_GLIBC 2

/**
 * @brief GoogleTest Platform Selector
 *
 */
#define SF_PLATFORM_GOOGLETEST 3

/**
 * @brief Smartfin Platform Designator
 *
 */
#ifdef PARTICLE
#define SF_PLATFORM SF_PLATFORM_PARTICLE
#else
#define SF_PLATFORM SF_PLATFORM_GLIBC
#endif

/**
 * @brief Inhibit Upload Flag
 *
 */
#define SF_INHIBIT_UPLOAD
/**
 * @brief High Data Rate mode
 *
 *
 * This mode should inhibit cellular upload and enable high data rate streams
 */
#define SF_HIGH_DATA_RATE

#define SF_CAN_UPLOAD !defined(SF_INHIBIT_UPLOAD) && !defined(SF_HIGH_DATA_RATE)
#endif // __PRODUCT_HPP__