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

/*******************************************************************************
 * Platform Configuration
 ******************************************************************************/
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
/** 
 * @brief Defines the platform being used in the system
 */
#define SF_PLATFORM SF_PLATFORM_PARTICLE
#else
#define SF_PLATFORM SF_PLATFORM_GLIBC
#endif
#endif // __PRODUCT_HPP__


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
 * System Configuration
 ******************************************************************************/
/**
 * @brief Directory path for storing ride data.
 * 
 */
#define RIDE_DATA_DIR "ridedata"
/**
 * @brief Object name length in LittleFS filesystem
 * 
 */

#define LITTLEFS_OBJ_NAME_LEN 31
/**
 * @brief Flags tto enabale or disable use of ICM20648 temperature sensor
 * 
 */
#define USE_ICM_TEMP_SENSOR 0

/**
 * how long to stay in CLI mode if there are no key presses at all
 */
#define CLI_NO_INPUT_TIMEOUT_MS 600000

/**
 * The default state that the Smartfin comes up in
 */
#define SF_DEFAULT_STATE   STATE_CHARGE


/*******************************************************************************
 * CLI RGB LED Configuration
 ******************************************************************************/
/**
 * The CLI RGB LED Color
 */
#define SF_CLI_RGB_LED_COLOR        RGB_COLOR_RED
/**
 * The CLI RGB LED Pattern
 */
#define SF_CLI_RGB_LED_PATTERN      LED_PATTERN_SOLID
/**
 * The CLI RGB LED behavior period
 */
#define SF_CLI_RGB_LED_PERIOD       3000
/**
 * The CLI RGB LED Priority
 */
#define SF_CLI_RGB_LED_PRIORITY     LED_PRIORITY_IMPORTANT

/*******************************************************************************
 * RIDE RGB LED Configuration
 ******************************************************************************/
/**
 * The Ride RGB LED Color
 */
#define RIDE_RGB_LED_COLOR RGB_COLOR_WHITE
/**
 * The Ride RGB LED pattern when GPS is in use
 */
#define RIDE_RGB_LED_PATTERN_GPS LED_PATTERN_BLINK
/**
 * The Ride RGB LED period when GPS is in use
 */
#define RIDE_RGB_LED_PERIOD_GPS 500
/**
 * The Ride RGB LED pattern when GPS is not in use
 */
#define RIDE_RGB_LED_PATTERN_NOGPS LED_PATTERN_SOLID
/**
 * The Ride RGB LED period when GPS is not in use
 */
#define RIDE_RGB_LED_PERIOD_NOGPS 0
/**
 * The Ride RGB LED Priority
 */
#define RIDE_RGB_LED_PRIORITY LED_PRIORITY_IMPORTANT



/*******************************************************************************
 * Data Upload RGB LED Configuration
 ******************************************************************************/
/**
 * The Data Upload RGB LED Color
 */
#define SF_DUP_RGB_LED_COLOR        RGB_COLOR_BLUE
/**
 * The Data Upload RGB LED behavior period
 */
#define SF_DUP_RGB_LED_PERIOD       500



/*******************************************************************************
 * Temperature Calibrator RGB LED Configuration
 ******************************************************************************/
/**
 * The Temperature Calibrator RGB LED color
 */
#define SF_TCAL_RGB_LED_COLOR       RGB_COLOR_ORANGE
/**
 * The Temperature Calibrator RGB LED pattern
 */
#define SF_TCAL_RGB_LED_PATTERN     LED_PATTERN_FADE
/**
 * The Temperature Calibrator RGB LED behavior period
 */
#define SF_TCAL_RGB_LED_PERIOD      3000
/**
 * The Temperature Calibrator RGB LED Priority
 */
#define SF_TCAL_RGB_LED_PRIORITY    LED_PRIORITY_IMPORTANT



/*******************************************************************************
 * Peripheral Configurations
 ******************************************************************************/
/**
 * @brief Serial Debugging Baud Rate
 */
#define SERIAL_DEBUG_BAUD_RATE 115200
/**
 * SPI Flash Size
 */
#define SF_FLASH_SIZE_MB    4

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



/******************************************************************************
 * Battery and Power Configuration
 *****************************************************************************/
/**
 * Minimum battery voltage to start an upload
 */ 
#define SF_BATTERY_UPLOAD_VOLTAGE 3.6

/**
 * @brief A voltage that's slightly higher than the max battery voltage
 * 
 */
#define SF_BATTERY_MAX_VOLTAGE 4.3

/**
 * Charging voltage (mV)
 */
#define SF_CHARGE_VOLTAGE   4112

/**
 * @brief Below what battery voltage should the system shutdown
 * 
 */
#define SF_BATTERY_SHUTDOWN_VOLTAGE 3.0



/******************************************************************************
 * Upload and Communication Configuration
 *****************************************************************************/
 /** 
 * @brief Baud rate for serial communication.
 */
#define SF_SERIAL_SPEED 9600

/** 
 * @brief Maximum length of a command line in the CLI.
 */
#define SF_CLI_MAX_CMD_LEN 100

/** 
 * @brief Maximum length for a name field
 */
#define SF_NAME_MAX 64

/**
 * @brief Maximum number of attempts to connect to the cloud
 * 
 */
#define SF_CLOUD_CONNECT_MAX_ATTEMPTS   5



/******************************************************************************
 * Data Upload Configuration
 *****************************************************************************/
/**
 * how long to stay in data upload without a cell signal/connection/succesful upload
 */
#define SF_CELL_SIGNAL_TIMEOUT_MS 300000

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

/******************************************************************************
 * Water Detection Configuration
 *****************************************************************************/
/**
 * Max size of the window/how large the FIFO array is
 */
#define WATER_DETECT_ARRAY_SIZE 200
/**
 * Window sizes are how many water detect samples are looked at in a moving 
 * average to determine if we are in or out of the water.  Generally a sample
 * happens 1/second
 */
#define WATER_DETECT_SURF_SESSION_INIT_WINDOW   40

/**
 * How long (in us) to turn on water detection circuit when looking for water
 */
#define WATER_DETECT_EN_TIME_US     1000



/******************************************************************************
 * Product and Version Information
 *****************************************************************************/
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


/******************************************************************************
 * Debugging and Testing Configuration
 *****************************************************************************/
/**
 * @brief Enable initialization delay
 * 
 */
// #define SF_ENABLE_DEBUG_DELAY   15


/**
 * @brief How long to wait for a cell connection in during manufacturing test
 * 
 */
#define MANUFACTURING_CELL_TIMEOUT_MS 180000



/******************************************************************************
 * Encoding Configuration
 *****************************************************************************/
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
/**
 * @brief Upload encoding type
 * 
 */
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
  * @brief How many bytes to store chunks of data in on the SPI flash.
  *
  * 768 * 4/3 (base64 encoding compression rate) = 1024 which is the maximum size
  * of publish events.
  */
#define SF_PACKET_SIZE  768
/**
  * @brief How many bytes to store chunks of a single record
  *
  * Set to 1024 bytes to match the maximum size allowed for publish events after encoding
  */
#define SF_RECORD_SIZE  1024
#endif