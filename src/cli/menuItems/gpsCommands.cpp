#include "Particle.h"

#include "gpsCommands.hpp"
#include "../conio.hpp"

#include "gps/location_service.h"

void CLI_GPS() 
{
	LocationServiceConfiguration config = create_location_service_config();
	LocationService::instance().setModuleType();
    LocationService::instance().begin(config);
    LocationService::instance().start();
	LocationService::instance().setFastLock(true);
    LocationPoint point = {};
	bool run = true;
	while(run) {
		if(kbhit()) 
		{
			char ch = getch();
			if(ch == 113) // if q is pressed
			{
				run = false;
			}
		}

		LocationService::instance().getLocation(point);
		// SF_OSAL_printf("Status: %d", LocationService::instance().getStatus());
		displayInfo(point);
	}
}


void displayInfo(LocationPoint point)
{
	SF_OSAL_printf("Latitude %f", point.latitude);
	SF_OSAL_printf("Longitude %f", point.longitude);
	SF_OSAL_printf("Altitude %f", point.altitude);
	SF_OSAL_printf("Speed %f", point.speed);
	SF_OSAL_printf("Heading %f", point.heading);
	SF_OSAL_printf("Accuracy (horizontal / vertical) (%f/%f)", 
					point.horizontalAccuracy, 
					point.verticalAccuracy);
	SF_OSAL_printf("Satellites in use %d", point.satsInUse);
	SF_OSAL_printf("Satellites in view", point.satsInView);
	SF_OSAL_printf("Locked: %d", point.locked);
}

