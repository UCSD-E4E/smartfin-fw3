#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include "sys/NVRAM.hpp"
#include "sys/led.hpp"
#include "cellular/recorder.hpp"
#include "watersensor/waterSensor.hpp"

#include "temperature/tmpSensor.h"
#include "location_service.h"

#define SYS_CHARGER_MIN_CHARGING_MS 5000
#define SYS_CHARGER_MIN_CHARGED_MS 30000
#define SYS_CHARGER_REFRESH_MS  500
#define SYS_WATER_REFRESH_MS    1000
#define SYS_BATTERY_MONITOR_MS  1000

typedef volatile struct SystemFlags_
{
    bool batteryLow;
    bool hasCharger;
    bool inWater;
}SystemFlags_t;


typedef struct SystemDesc_
{
    const char* deviceID;
    LocationService* pLocService;
    Recorder* pRecorder;
    Timer* pChargerCheck;
    Timer* pWaterCheck;
    NVRAM* pNvram;
    WaterSensor* pWaterSensor;
    SFLed* pBatteryLED;
    SFLed* pWaterLED;
    tmpSensor* pTempSensor;
    LEDSystemTheme* systemTheme;
    FuelGauge* pBattery;
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
 * @brief Displays all system components
 * 
 */
void SYS_displaySys(void);

extern SystemDesc_t* pSystemDesc;

#endif
