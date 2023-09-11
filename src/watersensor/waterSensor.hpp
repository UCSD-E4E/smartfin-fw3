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

class WaterSensor
{
private:
    // which pin is the water sensor on?
    uint8_t water_detect_en_pin;
    uint8_t water_detect_pin;
    // how many samples to look at for the moving window
    uint8_t moving_window_size;
    uint8_t low_detect_percentage = DEFAULT_WATER_SENSOR_LOW_PERCENTAGE;
    uint8_t high_detect_percentage = DEFAULT_WATER_SENSOR_HIGH_PERCENTAGE;
    // how many samples have been taken since a reset (signally a valid measurement)
    uint8_t samples_taken_since_reset = 0;
    // sum of the array, initially set to zero.
    uint8_t array_sum = 0;
    // current location in the array
    uint8_t array_location = 0;
    // the last water reading, for hystersis, starting with "out of the water"
    uint8_t last_water_detect = 0;

public:
    WaterSensor(uint8_t water_detect_en_pin, uint8_t water_detect_pin_to_set, uint8_t window_size);
    ~WaterSensor();

    // resets the array to zero
    bool resetArray();
    // switch the window size parameter and clear the sum (for resumming)
    void setWindowSize(uint8_t window_size_to_set);
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
