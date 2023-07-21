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
#include "states.hpp"
#include "product.hpp"
#include "consts.hpp"

#include "Particle.h"

#include <fstream>
#include<bits/stdc++.h>

const CLI_menu_t CLI_menu[] =
{
    {"menu", "display Menu", &CMD_displayMenu},
    {"disconnect", "disconnect particle", &CLI_disconnect},
    {"connect", "connect particle", &CLI_connect},
    {"showerrors", "show flog errors", &CLI_displayFLOG},
    {NULL,NULL,NULL}
};

static STATES_e CLI_nextState;

char *userInput[SF_CLI_MAX_CMD_LEN];

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
        char ch;
        int i = 0;
        char backspace = '\0';


        bool finishedTyping = false;
        // Loop through user input until they finish typing their command
        while (!finishedTyping) 
        {
            if(kbhit()) 
            {
                ch = getch();

                if(i > 99) 
                {
                    // too long of a command
                    SF_OSAL_printf(__NL__ "Command too long");

                    break;
                }

                switch(ch) 
                {
                    case '\b':
                        i--;
                        SF_OSAL_printf("\b \b");
                        userInput[i] = &backspace;
                        break;
                    case '\r':
                    case '\n':
                        userInput[i++] = 0;
                        finishedTyping = true;
                        break;
                    default:
                        putch(ch);
                        userInput[i++] = &ch;
                        break;
                }
            } 
            else
            {
                continue;
            }
        }

        
        if(userInput[0] != 0) //If there is a command
        {
            cmd = CLI_findCommand(*userInput);
            if(!cmd) 
            {
                putch(*userInput[0]);
                SF_OSAL_printf("Unknown command" __NL__);
                SF_OSAL_printf(">");
            } 
            else 
            {
                cmd->fn();
                SF_OSAL_printf(__NL__">");
            }
        }
        
        i = 0;
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

    for (pCmd = CLI_menu; pCmd->cmd; pCmd++)
    {
        if (strcmp(pCmd->cmd, cmd) == 0)
        {
            return pCmd;
        }
    }
    return nullptr;
}
