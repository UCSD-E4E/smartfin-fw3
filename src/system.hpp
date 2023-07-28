#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include "gps/location_service.h"
#include "sys/NVRAM.hpp"



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
    NVRAM* pNvram;
    const SystemFlags_t* flags;
}SystemDesc_t;

extern SystemDesc_t* pSystemDesc;


/**
 * @brief Initialization function for GPS 
 * Ublox gps, handled by @file gps/location_service.cpp
 * @return int sucsess
 */
static int SYS_initGPS();
/**
 * @brief Initializes all system components
 * 
 * @return int 
 */
int SYS_initSys(void);
/**
 * @brief Initializes NVRAM 
 * 
 * @return int sucsess
 */
static int SYS_initNVRAM(void);

#endif
