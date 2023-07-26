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

void CLI_connect(void)
{
    Particle.connect();
    waitFor(Particle.connected, 300000);
}

void CLI_disconnect(void)
{
    Particle.disconnect();
}
