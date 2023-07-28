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
#include "../../states.hpp"
#include "../cli.hpp"

#include "Particle.h"

void CLI_connect(void)
{
    waitFor(Particle.connected, 300000);
}

void CLI_disconnect(void)
{
    Particle.disconnect();
}

void CLI_doSleep(void)
{
    CLI_nextState = STATE_DEEP_SLEEP;
}

