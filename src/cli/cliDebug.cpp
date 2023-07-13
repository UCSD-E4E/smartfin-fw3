#include "Particle.h"

#include "conio.hpp"
#include "cli.hpp"
#include "menuItems/debugCommands.hpp"
#include "cliDebug.hpp"

typedef const struct CLI_debugMenu_
{
    int cmd;
    const char *const fnName;
    int (*fn)(void);
} CLI_debugMenu_t;

const CLI_debugMenu_t CLI_debugMenu[] = 
{
    {1, "Display Fault Log", &CLI_displayFLOG},
    {2, "Clear Fault Log", &CLI_clearFLOG},
    {3, "Restart System", &CLI_restart},
    {0, NULL, NULL}
};


void CLI_doDebugMode(void)
{
    char inputBuffer[80];
    int cmdInput;

    int debugRun = 1;
    while(debugRun) 
    {
        CLI_displayDebugMenu();
        SF_OSAL_printf(">\n");

        if(strlen(inputBuffer) == 0)
        {
            continue;
        }

        cmdInput = atoi(inputBuffer);
        if(cmdInput == 0) {
            debugRun = 0;
        }
        CLI_executeDebugMenu(cmdInput);
    }
    return;
}

int CLI_executeDebugMenu(const int cmd) 
{
    int i;

    for(i = 0; CLI_debugMenu[i -1].fn; i++) 
    {
        if(CLI_debugMenu[i -1].cmd == cmd)
        {
            SF_OSAL_printf("Executing %s\n", CLI_debugMenu[i-1].fnName);
            return CLI_debugMenu[i-1].fn();
        }
    }
    SF_OSAL_printf("Command not found\n");
    return 0;
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