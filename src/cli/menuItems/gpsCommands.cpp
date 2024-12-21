#include "Particle.h"

#include "gpsCommands.hpp"
#include "cli/conio.hpp"
#include "consts.hpp"

#include "location_service.h"

void CLI_GPS() 
{
    LocationPoint point;
    LocationService& instance= LocationService::instance();
    char ch;
    int row = 0;
    for (row = 0; ; row++)
    {
        if (SF_OSAL_kbhit())
        {
            ch = SF_OSAL_getch();
            if('q' == ch)
            {
                break;
            }
        }
        if (row % CLI_SCREEN_HEIGHT == 0)
        {
            SF_OSAL_printf("%11s %11s %6s %4s %6s %4s %4s %9s %9s %1s" __NL__,
                        "Latitude",
                        "Longitude",
                        "Alt",
                        "Spd",
                        "Hdg",
                        "HDOP",
                        "VDOP",
                        "Sats Used",
                        "Sats View",
                        "Locked");
        }
        instance.getLocation(point);
        SF_OSAL_printf("%11.6f %11.6f %6.3f %4.2f %6.3f %4.2f %4.2f %9d %9d %6d" __NL__, 
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
}
