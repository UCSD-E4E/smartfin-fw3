#include "systemCommands.hpp"
#include "../conio.hpp"
#include "Particle.h"


void CMD_displayMenu(void) {
    SF_OSAL_printf("CLI Menu\n MENU to display this menu\n"
                   "CONNECT to connect, DISCONNECT to disconnect "
                   "");
}

static int CLI_connect(void)
{
    Particle.connect();
    waitFor(Particle.connected, 300000);
    return 1;
}
static int CLI_disconnect(void)
{
    Particle.disconnect();
    return 1;
}