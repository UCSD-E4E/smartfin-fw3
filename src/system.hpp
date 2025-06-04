#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include "cellular/recorder.hpp"
#include "imu/newIMU.hpp"
#include "location_service.h"
#include "sys/NVRAM.hpp"
#include "sys/led.hpp"
#include "temperature/tmpSensor.h"
#include "watersensor/waterSensor.hpp"

#define SYS_CHARGER_MIN_CHARGING_MS 5000
#define SYS_CHARGER_MIN_CHARGED_MS 30000
#define SYS_CHARGER_REFRESH_MS  500
#define SYS_WATER_REFRESH_MS    1000
#define SYS_BATTERY_MONITOR_MS  1000

typedef volatile struct SystemFlags_
{
    bool batteryLow;
    bool hasCharger;
}SystemFlags_t;

/**
 * @brief Contains all system components
 */
typedef struct SystemDesc_
{
    /**
     * @brief Unique device identifier
     */
    const char* deviceID;
    /**
     * @brief Pointer to Location service object that handles GPS tracking and settings
     * 
     */
    LocationService* pLocService;
     /**
     * @brief Pointer to Recorder object that stores and manages data packets
     * 
     */
    Recorder* pRecorder;
    /**
     * @brief Pointer to Timer object that regularly checks if device is charging
     * 
     */
    Timer* pChargerCheck;
    /**
     * @brief Pointer to Timer object that regularly checks if device is in water
     * 
     */
    Timer* pWaterCheck;
    /**
     * @brief Counts number of times device is checked for if it is in water
     */
    std::size_t water_check_count;
    /**
     * @brief Point to NVRAM object that handles non-volatile memory storage
     * 
     * persists system state during deep sleep and power cycle
     */
    NVRAM* pNvram;
    /**
     * @brief Point to WaterSensor object that detects water presence using a moving window of samples
     * 
     */    
    WaterSensor* pWaterSensor;
    /**
     * @brief Pointer to SFLed object that handles LED state when charging
     * 
     * on if charged and charger plugged in,
     * blinking if charging,
     * off if not charging
     */
    SFLed* pBatteryLED;
    /**
     * @brief Pointer to SFLed object that handles LED state to indicate whether in water
     * 
     * on if in water, off otherwise
     */
    SFLed* pWaterLED;
    /**
     * @brief  Pointer to tmpSensor object that handles temperature readings and sensor initalization
     * 
     */
    tmpSensor* pTempSensor;
    /**
     * @brief Pointer to LEDSystemTheme object that handles LED color and pattern settings
     * 
     */
    LEDSystemTheme* systemTheme;
    /**
     * @brief Pointer to FuelGauge object that handles voltage readings
     * 
     */
    FuelGauge* pBattery;
    /**
     * @brief Pointer to IMU object
     *
     */
    IMU *pIMU;
    /**
     * @brief Pointer to SystemFlags_t object that contains current system status flag
     *
     */
    const SystemFlags_t* flags;
}SystemDesc_t;

extern SystemDesc_t* pSystemDesc;


/**
 * @brief Initializes all system components
 * 
 * @return int 
 */
void SYS_initSys(void);

/**
 * @brief Delayed initialization
 *
 */
void SYS_delayedInitSys(void);

/**
 * @brief Displays all system components
 * 
 */
void SYS_displaySys(void);

/**
 * @brief Dumps a text view of all system components and state
 * 
 * @param indent Indentation amount.
 */
void SYS_dumpSys(int indent);

extern SystemDesc_t* pSystemDesc;

#endif
