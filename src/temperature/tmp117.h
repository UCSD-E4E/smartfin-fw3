#ifndef TMP117_H
#define TMP117_H

#include "i2c/mbed.h"

#define TMP117_I2CADDR_DEFAULT 0x48 ///< TMP117 default i2c address
#define TMP117_CHIP_ID 0x0117       ///< TMP117 default device id from WHOAMI

#define TMP117_WHOAMI 0x0F  ///< Chip ID register
#define _CONFIGURATION 0x01 ///< Configuration register

#define TMP117_TEMP_DATA 0x00     ///< Temperature data register
#define TMP117_CONFIGURATION 0x01 ///< Configuration register
#define TMP117_T_HIGH_LIMIT 0x02  ///< High limit set point register
#define TMP117_T_LOW_LIMIT 0x03   ///<  Low limit set point register
#define TMP117_TEMP_OFFSET 0x07   ///< Temp offset register
#define TMP117_DEVICE_ID 0x0F     ///< Device ID register
#define WHOAMI_ANSWER 0x0117      ///< Correct 2-byte ID register value response

#define HIGH_ALRT_FLAG 0b100 ///< mask to check high threshold alert
#define LOW_ALRT_FLAG 0b010  ///< mask to check low threshold alert
#define DRDY_ALRT_FLAG 0b001 ///< mask to check data ready flag

#define TMP117_RESOLUTION 0.0078125f ///< Scalar to convert from LSB value to degrees C

///////////////////////////////////////////////////////////////

/**
 * @brief
 *
 * Allowed values for `setDataRate`.
 */
typedef enum {
  TMP117_RATE_ONE_SHOT,
} tmp117_rate_t;

/**
 * @brief A struct to hold alert state information.
 *
 * The alert state register is auto-clearing and so must be read together
 *
 */
typedef struct {
  bool high;       ///< Status of the high temperature alert
  bool low;        ///< Status of the low temperature alert
  bool data_ready; ///< Status of the data_ready alert
} tmp117_alerts_t;

/**
 * @brief Options for setAveragedSampleCount
 *
 */
typedef enum {
  TMP117_AVERAGE_1X,
  TMP117_AVERAGE_8X,
  TMP117_AVERAGE_32X,
  TMP117_AVERAGE_64X,
} tmp117_average_count_t;

/**
 * @brief Options to specify the minimum delay between new measurements.
 *
 */
typedef enum {
  TMP117_DELAY_0_MS,
  TMP117_DELAY_125_MS,
  TMP117_DELAY_250_MS,
  TMP117_DELAY_500_MS,
  TMP117_DELAY_1000_MS,
  TMP117_DELAY_4000_MS,
  TMP117_DELAY_8000_MS,
  TMP117_DELAY_16000_MS,
} tmp117_delay_t;

/**
 * @brief Options to set the measurement mode of the sensor
 *
 * In `TMP117_MODE_CONTINUOUS`, new measurements are read and available
 * according to the interval determined by the number of averaged samples and
 * the delay between reads.
 *
 * When the mode is `TMP117_MODE_SHUTDOWN` the sensor is placed in a low power
 * state and new measurements are not taken until a different mode is set. In
 * this mode, active circuitry within this sensor is deactivated, lowering the
 * power consumption dramatically.
 *
 * When the mode is set to `TMP117_MODE_ONE_SHOT`, a single new measurement is
 * calculated from the configured number of samples to be averaged and available
 * as soon as the measurements are Complete.
 *
 * Once the new measurement is calculated and available, the sensor switches to
 * `TMP117_MODE_SHUTDOWN` until `TMP117_MODE_ONE_SHOT` is set again to calculate
 * a new measurement or the mode is switched to `TMP117_MODE_CONTINUOUS`.
 *
 * **NOTE:** This setting ignores the configured minimum delay between
 * measurements.
 *
 */
typedef enum {
  TMP117_MODE_CONTINUOUS,
  TMP117_MODE_SHUTDOWN,
  TMP117_MODE_ONE_SHOT = 3, // skipping 0x2 which is a duplicate CONTINUOUS
} tmp117_mode_t;

#endif