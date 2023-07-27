/**
 * Project smartfin-fw3
 * Description: GPS CLI commands
 * @file gpsCommands.cpp
 * @author @emilybthorpe
 * @date Jul 27 2023 
 * 
*/

#include "Particle.h"

#include "gpsCommands.hpp"
#include "../conio.hpp"
#include "../../consts.hpp"

#include "../lib/gps-tracker/location_service.h"

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
	SF_OSAL_printf(__NL__ "Latitude %f", point.latitude);
	SF_OSAL_printf(__NL__ "Longitude %f", point.longitude);
	SF_OSAL_printf(__NL__ "Altitude %f", point.altitude);
	SF_OSAL_printf(__NL__ "Speed %f", point.speed);
	SF_OSAL_printf(__NL__ "Heading %f", point.heading);
	SF_OSAL_printf(__NL__ "Accuracy (horizontal / vertical) (%f/%f)", 
					point.horizontalAccuracy, 
					point.verticalAccuracy);
	SF_OSAL_printf(__NL__ "Satellites in use %d", point.satsInUse);
	SF_OSAL_printf(__NL__ "Satellites in view", point.satsInView);
	SF_OSAL_printf(__NL__ "Locked: %d", point.locked);
}

