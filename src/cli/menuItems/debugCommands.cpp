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

int CLI_restart(void)
{
    System.reset();
    return 1;
}

int CLI_displayFLOG(void)
{
    FLOG_DisplayLog();
    return 1;
}

int CLI_clearFLOG(void)
{
    FLOG_ClearLog();
    return 1;
}