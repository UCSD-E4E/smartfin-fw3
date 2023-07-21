/**
 * Project smartfin-fw3
 * Description: System CLI commands
 * @file debugCommands.cpp
 * @author @emilybthorpe
 * @date Jul 20 2023 
 * 
*/

#include "systemCommands.hpp"
#include "../conio.hpp"
#include "../flog.hpp"

#include "Particle.h"

void CMD_displayMenu(void) {
    SF_OSAL_printf("CLI Menu\n menu - display system menu\n"
                   "connect - connect particle board\n"
                   "disconnect - disconnect particle board\n"
                   "debug - enter debug mode\n"
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
