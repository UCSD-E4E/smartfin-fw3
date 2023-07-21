#include "cli.hpp"
#include "states.hpp"
#include "Particle.h"
#include "conio.hpp"
#include "menuItems/systemCommands.hpp"
#include "menuItems/debugCommands.hpp"
#include "menuItems/gpsCommands.hpp"
#include "cliDebug.hpp"

#include <fstream>

const CLI_menu_t CLI_menu[] =
{
    {"menu", "display Menu", &CMD_displayMenu},
    {"disconnect", "disconnect particle", &CLI_disconnect},
    {"connect", "connect particle", &CLI_connect},
    {"debug", "debug mode", &CLI_doDebugMode},
    {"gps", "display gps", &CLI_GPS},
    {NULL,NULL,NULL}
};

static STATES_e CLI_nextState;

void CLI::init(void) 
{
    CLI_nextState = STATE_CLI;

    // While there is an avaliable character typed, get it
    while(kbhit())
    {
        getch();
    }
}


STATES_e CLI::run(void)
{
    CLI_menu_t *cmd;
    char* userInput = new char[100];
    userInput[0] = 0;

    SF_OSAL_printf("\n>");

    while (1) 
    {
        //TODO: check for input timeout
        


        userInput = getUserInput(userInput);

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
        
        userInput = new char[100];
        userInput[0] = 0;
    }

    delete[] userInput;

    return CLI_nextState;
}

void CLI::exit() {
    return;
}

CLI_menu_t const* CLI::CLI_findCommand(char *cmd)
{
    CLI_menu_t const* pCmd;

    for (pCmd = CLI_menu; pCmd->cmd; pCmd++)
    {
        if (strcmp(pCmd->cmd, cmd) == 0)
        {
            return pCmd;
        }
    }
    return NULL;
}