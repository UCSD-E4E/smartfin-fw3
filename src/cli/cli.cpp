/**
 * Project smartfin-fw3
 * Description: Command Line interface for development, manufacturing tests, and user interaction
 * Author: Emily Thorpe
 * Date: Jul 20 2023 
 * 
*/

#include "cli.hpp"

#include "conio.hpp"
#include "menuItems/systemCommands.hpp"
#include "menuItems/debugCommands.hpp"

#include "states.hpp"
#include "product.hpp"
#include "consts.hpp"

#include "Particle.h"

#include "cliDebug.hpp"


#include <fstream>
#include <bits/stdc++.h>

void CMD_displayMenu(void);


const CLI_menu_t CLI_menu[] =
{
    {0, "display Menu", &CMD_displayMenu},
    {1, "disconnect particle", &CLI_disconnect},
    {2, "connect particle", &CLI_connect},
    {3, "show flog errors", &CLI_displayFLOG},
    {4, "test printf", &CLI_testPrintf},
    {5, "debug menu,", &CLI_doDebugMode},
    {0, nullptr, nullptr}
};

static STATES_e CLI_nextState;

char userInput[SF_CLI_MAX_CMD_LEN];

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
    
    userInput[0] = 0;

    SF_OSAL_printf(__NL__ ">");

  

    while (1) 
    {
        memset(userInput, 0, SF_CLI_MAX_CMD_LEN);
        //TODO: check for input timeout
        


        getUserInput(userInput);

        
        if(userInput[0] != 0) //If there is a command
        {
            SF_OSAL_printf("\r\n");
            cmd = CLI_findCommand(userInput);
            if(!cmd) 
            {
                putch(userInput[0]);
                SF_OSAL_printf("Unknown command" __NL__);
                SF_OSAL_printf(">");
            } 
            else 
            {
                cmd->fn();
                SF_OSAL_printf(__NL__">");
            }
        }
    }


    return CLI_nextState;
}

void CLI::exit() 
{
    return;
}

CLI_menu_t const* CLI::CLI_findCommand(const char *cmd)
{
    CLI_menu_t const* pCmd;
    int cmd_value = atoi(cmd);

    for (pCmd = CLI_menu; pCmd->fn; pCmd++)
    {
        if (cmd_value == pCmd->cmd)
        {
            return pCmd;
        }
    }
    return nullptr;
}

void CMD_displayMenu(void)
{
    CLI_menu_t const* pCmd;
    for(pCmd = CLI_menu; pCmd->fn; pCmd++)
    {
        SF_OSAL_printf("%6d: %s" __NL__, pCmd->cmd, pCmd->fnName);
    }
}