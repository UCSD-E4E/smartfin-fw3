#include "Particle.h"

#include "gpsCommands.hpp"
#include "cli/conio.hpp"
#include "consts.hpp"

#include "location_service.h"

void CLI_GPS() 
{
    LocationPoint point = {};
    char ch;
    
    while(1) 
    {
        if(kbhit()) 
        {
            ch = getch();
            if('q' == ch)
            {
                break;
            }
        }

        LocationService::instance().getLocation(point);
        displayInfo(point);
    }
}


void displayInfo(LocationPoint point)
{
    SF_OSAL_printf("Latitude %f" __NL__ , point.latitude);
    SF_OSAL_printf("Longitude %f" __NL__, point.longitude);
    SF_OSAL_printf("Altitude %f" __NL__, point.altitude);
    SF_OSAL_printf("Speed %f" __NL__, point.speed);
    SF_OSAL_printf("Heading %f" __NL__, point.heading);
    SF_OSAL_printf("Accuracy (horizontal / vertical) (%f/%f)" __NL__, 
                    point.horizontalAccuracy, 
                    point.verticalAccuracy);
    SF_OSAL_printf("Satellites in use %d" __NL__, point.satsInUse);
    SF_OSAL_printf("Satellites in view" __NL__, point.satsInView);
    SF_OSAL_printf("Locked: %d" __NL__, point.locked);
}

