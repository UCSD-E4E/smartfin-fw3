/**
* Project smartfin-fw3
* @file systemCommands.cpp
* Description: System commands for CI
* @author @emilybthorpe
* @date Jul 20 2023
*/

#include "systemCommands.hpp"
#include "../conio.hpp"
#include "Particle.h"
#include "../flog.hpp"


void CMD_displayMenu(void) {
    SF_OSAL_printf("CLI Menu\n MENU to display this menu\n"
                   "CONNECT to connect, DISCONNECT to disconnect "
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

void CLI_displayFLOG(void)
{
    FLOG_DisplayLog();
}