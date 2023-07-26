/**
 * Project smartfin-fw3
 * Description: Command Line interface for development, manufacturing tests, and user interaction
 * Author: Emily Thorpe
 * Date: Jul 20 2023 
 * 
*/

#include "cli.hpp"

#include "cliDebug.hpp"
#include "conio.hpp"
#include "consts.hpp"
#include "menu.hpp"
#include "menuItems/systemCommands.hpp"
#include "menuItems/debugCommands.hpp"
#include "states.hpp"
#include "product.hpp"

#include "Particle.h"

#include <fstream>
#include <bits/stdc++.h>

void CLI_displayMenu(void);


const Menu_t CLI_menu[] =
{
    {1, "display Menu", &CLI_displayMenu},
    {2, "disconnect particle", &CLI_disconnect},
    {3, "connect particle", &CLI_connect},
    {4, "show flog errors", &CLI_displayFLOG},
    {5, "test printf", &CLI_testPrintf},
    {6, "debug menu,", &CLI_doDebugMode},
    {0, nullptr, nullptr}
};

static STATES_e CLI_nextState;

char userInput[SF_CLI_MAX_CMD_LEN];

void CLI::init(void) 
{
    CLI_nextState = STATE_CLI;

    // While there is an avaliable character typed, get it
    while (kbhit())
    {
        getch();
    }
}


STATES_e CLI::run(void)
{
    Menu_t *cmd;
    
    userInput[0] = 0;

    SF_OSAL_printf(__NL__ ">");

  

    while (1) 
    {
        memset(userInput, 0, SF_CLI_MAX_CMD_LEN);        

        getUserInput(userInput);

        if (strlen(userInput) != 0) //If there is a command
        {
            SF_OSAL_printf("\r\n");
            cmd = MNU_findCommand(userInput, CLI_menu);
            if (!cmd) 
            {
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

void CLI_displayMenu(void)
{
    MNU_displayMenu(CLI_menu);
}