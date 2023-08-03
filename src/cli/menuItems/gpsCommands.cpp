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
    SF_OSAL_printf("%11s %11s %6s %6s %6s %4s %4s %9s %9s %1s",
                "Latitude",
                "Longitude",
                "Alt",
                "Speed",
                "Hdg",
                "HDOP",
                "VDOP",
                "Sats Used",
                "Sats View",
                "Locked");
    SF_OSAL_printf("%11.6f %11.6f %6.3f %6.3f %6.3f %4.2f %4.2f %9d %9d %6d\r", 
                    point.latitude,
                    point.longitude,
                    point.altitude,
                    point.speed,
                    point.heading,
                    point.horizontalAccuracy,
                    point.verticalAccuracy,
                    point.satsInUse,
                    point.satsInView,
                    point.locked
                    );
}

