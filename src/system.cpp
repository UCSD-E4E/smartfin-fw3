#include "system.hpp"

#include "gps/location_service.h"
#include "Particle.h"


char SYS_deviceID[32];

SystemDesc_t systemDesc, *pSystemDesc = &systemDesc;
SystemFlags_t systemFlags;


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


static int SYS_initGPS() 
{
    LocationServiceConfiguration config = create_location_service_config();
	LocationService::instance().setModuleType();
    LocationService::instance().begin(config);
    LocationService::instance().start();
	LocationService::instance().setFastLock(true);
    systemDesc.pLocService = &LocationService::instance();
    
    return 1;
}