#include "Particle.h"

#include "conio.hpp"
#include  <cstdlib>
#include <cstdio>
#include <cmath>
#include "cli.hpp"
#include "menuItems/debugCommands.hpp"
#include "product.hpp"
#include "util.hpp"
#include "cliDebug.hpp"

typedef const struct CLI_debugMenu_
{
    int cmd;
    const char *const fnName;
    void (*fn)(void);
} CLI_debugMenu_t;

CLI_debugMenu_t const* CLI_findCommand(int cmd);

const CLI_debugMenu_t CLI_debugMenu[] = 
{
    {1, "Display Fault Log", &CLI_displayFLOG},
    {2, "Clear Fault Log", &CLI_clearFLOG},
    {3, "Restart System", &CLI_restart},
    {0, NULL, NULL}
};

void CLI_doDebugMode(void)
{
    char* userInput = new char[SF_CLI_MAX_CMD_LEN];

    int CLI_debugRun = 1;
    while(CLI_debugRun)
    {
        CLI_displayDebugMenu();

        CLI_debugMenu_t *cmd;
        getline(userInput, SF_CLI_MAX_CMD_LEN);
        if(atoi(userInput) == 0) // Exiting debug mode
        {
            break;
        }

        SF_OSAL_printf("\r\n");
        cmd = CLI_findCommand(atoi(userInput));
        if(!cmd) 
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

CLI_debugMenu_t const* CLI_findCommand(int cmd)
{
    CLI_debugMenu_t const* pCmd;

    for (pCmd = CLI_debugMenu; pCmd->cmd; pCmd++)
    {
        if (pCmd->cmd == cmd)
        {
            return pCmd;
        }
    }
    return NULL;
}

void CLI_displayDebugMenu(void) {
    int i;
    for(i = 1; CLI_debugMenu[i - 1].fn; i++)
    {
        SF_OSAL_printf("%3d %s\n", i, CLI_debugMenu[i-1].fnName);
    }
    SF_OSAL_printf("0 Exit\n");
    return;
}