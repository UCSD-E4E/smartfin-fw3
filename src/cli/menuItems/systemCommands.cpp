/**
 * Project smartfin-fw3
 * Description: System CLI commands
 * @file debugCommands.cpp
 * @author @emilybthorpe
 * @date Jul 20 2023 
 * 
*/

#include "systemCommands.hpp"


#include "cli/conio.hpp"
#include "cli/cli.hpp"
#include "cli/flog.hpp"

#include "states.hpp"

#include "Particle.h"

void CLI_connect(void)
{
    Particle.connect();
    waitFor(Particle.connected, 300000);
}

void CLI_disconnect(void)
{
    Particle.disconnect();
}

void CLI_doSleep(void)
{
    SF_OSAL_printf("Next state is sleep\n");
    CLI_nextState = STATE_DEEP_SLEEP;
}

