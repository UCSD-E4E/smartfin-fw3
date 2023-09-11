/**
 * @file cliDebug.cpp
 * @author Emily Thorpe (ethorpe@macalster.edu)
 * @brief 
 * @version 0.1
 * @date 2023-07-26
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "cliDebug.hpp"

#include "conio.hpp"
#include "cli.hpp"
#include "menu.hpp"

#include "menuItems/debugCommands.hpp"

#include "product.hpp"
#include "util.hpp"
#include "menu.hpp"

#include "system.hpp"

#include "consts.hpp"

#include <cmath>
#include <cstdlib>
#include <cstdio>


#include "Particle.h"


const Menu_t CLI_debugMenu[] = 
{
    {1, "Display Fault Log", &CLI_displayFLOG},
    {2, "Clear Fault Log", &CLI_clearFLOG},
    {3, "Restart System", &CLI_restart},
    {4, "MFG Test", &CLI_doMfgTest},
    {5, "Wet/Dry Monitor", &CLI_monitorWetDry},
    {6, "Monitor IMU", &CLI_monitorIMU},
    {7, "Monitor Temperature", &CLI_monitorTempSensor},
    {8, "reinitialize system", &SYS_initSys},
    {9, "create test file", &CLI_createTestFile},
    {10, "test has data", &CLI_testHasData},
    {11, "test num files", &CLI_testGetNumFiles},
    {12, "wipe file system", &CLI_wipeFileSystem},
    {0, NULL, NULL}
};

void CLI_doDebugMode(void)
{
    char* userInput = new char[SF_CLI_MAX_CMD_LEN];

    int CLI_debugRun = 1;
    while (CLI_debugRun)
    {

        if(CLI_nextState != STATE_CLI)
        {
            break;
        }

        MNU_displayMenu(CLI_debugMenu);

        Menu_t *cmd;
        getline(userInput, SF_CLI_MAX_CMD_LEN);
        
        if (atoi(userInput) == 0) // Exiting debug mode
        {
            break;
        }

        SF_OSAL_printf(__NL__);

        cmd = MNU_findCommand(userInput, CLI_debugMenu);
        if (!cmd) 
        {
            SF_OSAL_printf("Unknown command" __NL__);
        } else {
            cmd->fn();
        }
        
        SF_OSAL_printf(">");
    }
    return;
}
