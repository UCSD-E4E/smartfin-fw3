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
#include "menuItems/debugCommands.hpp"
#include "product.hpp"
#include "util.hpp"
#include "menu.hpp"

#include "system.hpp"

#include "Particle.h"

#include <cmath>
#include <cstdlib>
#include <cstdio>


const Menu_t CLI_debugMenu[] = 
{
    {1, "Display Fault Log", &CLI_displayFLOG},
    {2, "Clear Fault Log", &CLI_clearFLOG},
    {3, "Restart System", &CLI_restart},
    {4, "reinitialize system", &SYS_initSys},
    {5, "create test file", &CLI_createTestFile},
    {6, "test has data", &CLI_testHasData},
    {7, "test num files", &CLI_testGetNumFiles},
    {0, NULL, NULL}
};

void CLI_doDebugMode(void)
{
    char* userInput = new char[SF_CLI_MAX_CMD_LEN];

    int CLI_debugRun = 1;
    while (CLI_debugRun)
    {
        MNU_displayMenu(CLI_debugMenu);

        Menu_t *cmd;
        getline(userInput, SF_CLI_MAX_CMD_LEN);
        if (atoi(userInput) == 0) // Exiting debug mode
        {
            break;
        }

        SF_OSAL_printf("\r\n");
        cmd = MNU_findCommand(userInput, CLI_debugMenu);
        if (!cmd) 
        {
            putch(userInput[0]);
            SF_OSAL_printf("Unknown command\n");
            SF_OSAL_printf(">");
        } else {
            cmd->fn();
            SF_OSAL_printf(">");
        }
    }
    return;
}
