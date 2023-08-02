#include "system.hpp"

#include "location_service.h"
#include "Particle.h"
#include "cli/conio.hpp"
#include "consts.hpp"

char SYS_deviceID[32];

SystemDesc_t systemDesc, *pSystemDesc = &systemDesc;
SystemFlags_t systemFlags;


static int SYS_initGPS();


int SYS_initSys(void)
{
    memset(pSystemDesc, 0, sizeof(SystemDesc_t));
    systemDesc.deviceID = SYS_deviceID;
    systemDesc.flags = &systemFlags;

    memset(SYS_deviceID, 0, 32);
    strncpy(SYS_deviceID, System.deviceID(), 31);

    SYS_initGPS();
    return 1;
}

/**
 * @brief Initialization function for GPS 
 * Ublox gps, handled by @file gps/location_service.cpp
 * @return int sucsess
 */
static int SYS_initGPS() 
{
    LocationServiceConfiguration config = create_location_service_config();
    LocationService::instance().setModuleType();
    if(LocationService::instance().begin(config) != 0)
    {
        SF_OSAL_printf("GPS Initialization Failed" __NL__);
        SF_OSAL_printf("Check pin map and reboot" __NL__);
        return -1;
    }

    if(LocationService::instance().start() != 0)
    {
        SF_OSAL_printf("GPS Start Failed" __NL__);
        SF_OSAL_printf("Please check GPS and reboot" __NL__);
        return -1;
    }
    LocationService::instance().setFastLock(true);

    systemDesc.pLocService = &LocationService::instance();
    
    return 1;
}
