#include "Particle.h"

#include "conio.hpp"
#include  <cstdlib>
#include <cstdio>
#include <cmath>
#include "cli.hpp"
#include "menuItems/debugCommands.hpp"
#include "util.hpp"
#include "cliDebug.hpp"

typedef const struct CLI_debugMenu_
{
    char* cmd;
    const char *const fnName;
    void (*fn)(void);
} CLI_debugMenu_t;

CLI_debugMenu_t const* CLI_findCommand(char *cmd);

const CLI_debugMenu_t CLI_debugMenu[] = 
{
    {"1", "Display Fault Log", &CLI_displayFLOG},
    {"2", "Clear Fault Log", &CLI_clearFLOG},
    {"3", "Restart System", &CLI_restart},
    {"0", NULL, NULL}
};




void CLI_doDebugMode(void)
{
    char* userInput = new char[100];

    int CLI_debugRun = 1;
    while(CLI_debugRun)
    {
        CLI_displayDebugMenu();

        CLI_debugMenu_t *cmd;
        userInput = getUserInput(userInput);
        SF_OSAL_printf("Entered command: %s\n", userInput);

        if(strlen(userInput) != 0) 
        {
            SF_OSAL_printf("\r\n");
            cmd = CLI_findCommand(userInput);
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
    }
    return;
}

CLI_debugMenu_t const* CLI_findCommand(char *cmd)
{
    CLI_debugMenu_t const* pCmd;

    for (pCmd = CLI_debugMenu; pCmd->cmd; pCmd++)
    {
        if (strcmp(pCmd->cmd, cmd) == 0)
        {
            return pCmd;
        }
    }
    return NULL;
}


// int CLI_executeDebugMenu(const char cmd) 
// {
//     int i;
//     for(i = 0; CLI_debugMenu[i -1].fn; i++) 
//     {
//         if(CLI_debugMenu[i -1].cmd == cmd)
//         {
//             SF_OSAL_printf("Executing %s\n", CLI_debugMenu[i-1].fnName);
//             CLI_debugMenu[i-1].fn();
//             return 1;
//         }
//     }
//     SF_OSAL_printf("Command not found\n");
//     return 0;
// }

void CLI_displayDebugMenu(void) {
    int i;
    for(i = 1; CLI_debugMenu[i - 1].fn; i++)
    {
        SF_OSAL_printf("%3d %s\n", i, CLI_debugMenu[i-1].fnName);
    }
    SF_OSAL_printf("0 Exit\n");
    return;
}