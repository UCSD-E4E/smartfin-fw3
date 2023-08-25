#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include "sys/NVRAM.hpp"
#include "sys/led.hpp"

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
    Timer* pChargerCheck;
    NVRAM* pNvram;
    SFLed* pBatteryLED;
    const SystemFlags_t* flags;
    LEDSystemTheme* systemTheme;
}SystemDesc_t;

extern SystemDesc_t* pSystemDesc;


/**
 * @brief Initializes all system components
 * 
 * @return int 
 */
int SYS_initSys(void);

/**
 * @brief Displays all system components
 * 
 */
void SYS_displaySys(void);

extern SystemDesc_t* pSystemDesc;

#endif
