#include "cli.hpp"
#include "states.hpp"
#include "Particle.h"
#include "conio.hpp"
#include "menuItems/systemCommands.hpp"

#include <fstream>

const CLI_menu_t CLI_menu[] =
{
    {"menu", "display Menu", &CMD_displayMenu},
    {"disconnect", "disconnect particle", &CLI_disconnect},
    {"connect", "connect particle", &CLI_connect},
    {"showerrors", "show flog errors", &CLI_displayFLOG},
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
    // uint32_t lastKeyPressTime;
    CLI_menu_t *cmd;
    char* userInput = new char[100];

    // lastKeyPressTime = millis();

    SF_OSAL_printf(">");

    while (1) 
    {
        //TODO: check for input timeout
        char ch;
        int i = 0;


        bool finishedTyping = false;
        // Loop through user input until they finish typing their command
        while (!finishedTyping) 
        {
            if(kbhit()) 
            {
                ch = getch();

                if(i > 99) {
                    // too long of a command
                    SF_OSAL_printf("\n\nCommand too long");

                    break;
                }

                switch(ch) 
                {
                    case '\b':
                        i--;
                        SF_OSAL_printf("\b \b");
                        userInput[i] = '\0';
                        break;
                    case '\r':
                    case '\n':
                        userInput[i] = ch;
                        ++i;
                        finishedTyping = true;
                        break;
                    default:
                        putch(ch);
                        break;
                }
            } else{
                break;
            }
        }

        if(userInput != NULL) 
        {
            cmd = CLI_findCommand(userInput);
            if(!cmd) 
            {
                SF_OSAL_printf("Unknown command\n");
            } else {
                cmd->fn();
                SF_OSAL_printf(">");
            }
        }
        
        userInput = new char[100];
        i = 0;
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
