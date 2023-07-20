/*
Debug commands:

Todo: 

clearFlog - clear fault log

displayFlog - display fault log

restart - restart the system

and whatever GPS function

*/
#include "debugCommands.hpp"
#include "../conio.hpp"
#include "Particle.h"
#include "../flog.hpp"

void CLI_restart(void)
{
    System.reset();
}

void CLI_displayFLOG(void)
{
    FLOG_DisplayLog();
}

void CLI_clearFLOG(void)
{
    FLOG_ClearLog();
}