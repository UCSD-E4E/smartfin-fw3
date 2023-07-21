#include "Particle.h"


static int CLI_gpsTerminal(void)
{
    bool run = true;
    char user;
    digitalWrite(GPS_PWR_EN_PIN, HIGH);
    delay(500);
    pSystemDesc->pGPS->gpsModuleInit();

    while(run)
    {
        if(GPS_kbhit())
        {
            putch(GPS_getch());
        }
        if(kbhit())
        {
            user = getch();
            if(user == 27)
            {
                run = false;
            }
            else
            {
                GPS_putch(user);
            }
        }
    }
    digitalWrite(GPS_PWR_EN_PIN, LOW);
    return 1;
}
