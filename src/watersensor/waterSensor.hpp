#ifndef __WATERSENSOR_H__
#define __WATERSENSOR_H__
#include "Particle.h"
#include <cstdlib>
#include <cstdint>
#include <cstring>

// manages a moving window/fifo of water detect samples
//  e.g. if a 0 represents being out of the water and a 1 is in
//  a run of samples might look like this:
//  0 0 0 0 0 0 0 0 1 1 1 0 1 1 1 0 1 1 1 0 1
//  if we are taking samples 1/second with a moving window of 10 samples
//  we might say when over 75% are 1s we are in the water and 25% are when
//  we are out of the water (there is hystersis, starting with us out of the water)

////////////////////////////////////////////////////////////////////////////////////////////////////
//Water Detect Defines//
#define DEFAULT_WATER_SENSOR_LOW_PERCENTAGE 25
// how much of the window needs to be "in water" to trigger a change from "out of water"
//  to "out of water"
#define DEFAULT_WATER_SENSOR_HIGH_PERCENTAGE 75
// just defines for clarity that signify different commands for waterDetect function

#define WATER_SENSOR_HIGH_STATE 1
#define WATER_SENSOR_LOW_STATE 0

/**
 * @brief Represents a sensor that detects water presence using a moving window of samples
 */
class WaterSensor
{
private:
    /**
     * @brief Pin that enables the water detection sensor
     */
    uint8_t water_detect_en_pin;
    /**
     * @brief Pin that reads the water detection status
     */
    uint8_t water_detect_pin;
    /**
     * @brief Size of the moving sample window used for water detection
     */
    uint8_t moving_window_size;
    /**
     * @brief Minimum percentage of high state readings to trigger a change to "out of water"
     */
    uint8_t low_detect_percentage = DEFAULT_WATER_SENSOR_LOW_PERCENTAGE;
    /**
     * @brief Minimum percentage of high state readings to trigger a change to "in water"
     */
    uint8_t high_detect_percentage = DEFAULT_WATER_SENSOR_HIGH_PERCENTAGE;
    /**
     * @brief Number of samples taken since the last reset
     */
    uint8_t samples_taken_since_reset = 0;
    /**
     * @brief Sum of the current samples in the window
     */
    uint8_t array_sum = 0;
    /**
     * @brief Current position in the array of samples
     */
    uint8_t array_location = 0;
    /**
     * @brief Previous water detection status used for hystersis
     */
    uint8_t last_water_detect = 0;

public:
    /**
     * @brief Initializes the WaterSensor with specific pins and window size
     *
     * @param water_detect_en_pin Pin that enables the water sensor
     * @param water_detect_pin_to_set Pin that reads the sensor value
     * @param window_size Size of the moving sample window used for water detection
     */
    WaterSensor(uint8_t water_detect_en_pin, uint8_t water_detect_pin_to_set);
    /**
     * @brief Destroys the WaterSensor instance
     */
    ~WaterSensor();

    // resets the array to zero
    bool resetArray();
    // switch the window size parameter and clear the sum (for resumming)
    void setWindowSize();
    // take a reading. Also returns the current in/out water status.
    uint8_t takeReading();
    // gets the current in/out of water status (return true = in water, false = out)
    uint8_t getCurrentStatus();
    // forces the in/out of water status.  Useful because of hystersis
    void forceState(uint8_t water_detect_state);
    // gets the current reading of the sensor (not the overall status w/ hystersis)
    uint8_t getCurrentReading();
    // set the low detection percentage of the moving window for hystersis
    bool setLowDetectPercentage(uint8_t low_percentage);
    // set the high detection percentage of the moving window for hystersis
    bool setHighDetectPercentage(uint8_t high_percentage);

    /**
     * @brief Causes the sensor to be read and the filter to be updated.
     * 
     * This should be called once every second.
     */
    void update();

    /**
     * @brief Get the last reading (unfiltered)
     * 
     * @return uint8_t 1 if wet, otherwise 0
     */
    uint8_t getLastReading();

    /**
     * @brief Get the Last Status (filtered)
     * 
     * @return uint8_t 1 if wet, otherwise 0
     */
    uint8_t getLastStatus();

private:
    // returns a valid location (between 0 and WATER_DETECT_ARRAY_SIZE) for a location + an offset
    uint8_t waterDetectArrayLocation(int16_t location, int16_t offset);
};

#endif
