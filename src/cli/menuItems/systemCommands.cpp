#include "systemCommands.hpp"
#include "../conio.hpp"
#include "Particle.h"
#include "../flog.hpp"


void CMD_displayMenu(void) {
    SF_OSAL_printf("CLI Menu\n menu - display system menu\n"
                   "connect - connect particle board , DISCONNECT to disconnect "
                   "");
}

void CLI_connect(void)
{
    Particle.connect();
    waitFor(Particle.connected, 300000);
}

void CLI_disconnect(void)
{
    Particle.disconnect();
}
