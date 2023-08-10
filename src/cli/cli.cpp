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
#include "menuItems/gpsCommands.hpp"
#include "cliDebug.hpp"
#include "states.hpp"
#include "util.hpp"
#include "product.hpp"

#include "system.hpp"

#include "Particle.h"


#include <fstream>
#include <bits/stdc++.h>

void CLI_displayMenu(void);
void CLI_hexdump(void);

static LEDStatus CLI_ledStatus;


const Menu_t CLI_menu[] =
{
    {1, "display Menu", &CLI_displayMenu},
    {2, "disconnect particle", &CLI_disconnect},
    {3, "connect particle", &CLI_connect},
    {4, "show flog errors", &CLI_displayFLOG},
    {5, "test printf", &CLI_testPrintf},
    {6, "debug menu", &CLI_doDebugMode},
    {7, "hexdump", &CLI_hexdump},
    {8, "gps", &CLI_GPS},
    {9, "sleep", &CLI_doSleep},
    {0, nullptr, nullptr}
};

char userInput[SF_CLI_MAX_CMD_LEN];

STATES_e CLI_nextState;

void CLI::init(void) 
{
    CLI_nextState = STATE_CLI;

    CLI_ledStatus.setColor(CLI_RGB_LED_COLOR);
    CLI_ledStatus.setPattern(CLI_RGB_LED_PATTERN);
    CLI_ledStatus.setPeriod(CLI_RGB_LED_PERIOD);
    CLI_ledStatus.setPriority(CLI_RGB_LED_PRIORITY);
    CLI_ledStatus.setActive();

    pSystemDesc->pChargerCheck->start();


    // While there is an avaliable character typed, get it
    while (kbhit())
    {
        getch();
    }
}


STATES_e CLI::run(void)
{
    Menu_t *cmd;
    uint32_t lastKeyPressTime;

    
    userInput[0] = 0;

    SF_OSAL_printf(__NL__ ">");

    lastKeyPressTime = millis();  


    CLI_nextState = STATE_CLI;

    while (1) 
    {
        if(millis() >= lastKeyPressTime + CLI_NO_INPUT_TIMEOUT_MS) 
        {
            CLI_nextState = STATE_CHARGE;
        }

        if(!pSystemDesc->flags->hasCharger)
        {
            CLI_nextState = STATE_DEEP_SLEEP;
        }

        if(CLI_nextState != STATE_CLI)
        {
            break;
        }

        memset(userInput, 0, SF_CLI_MAX_CMD_LEN);        

        getline(userInput, SF_CLI_MAX_CMD_LEN);

        if (strlen(userInput) != 0) //If there is a command
        {
            SF_OSAL_printf("\r\n");
            cmd = MNU_findCommand(userInput, CLI_menu);
            if (!cmd) 
            {
                SF_OSAL_printf("Unknown command" __NL__);
                MNU_displayMenu(CLI_menu);
                SF_OSAL_printf(__NL__">");
            } 
            else 
            {
                cmd->fn();
                SF_OSAL_printf(__NL__">");
            }
        }
    }

    SF_OSAL_printf("Next State: %d", CLI_nextState);
    return CLI_nextState;
}

void CLI::exit() 
{
    pSystemDesc->pChargerCheck->stop();
}

void CLI_displayMenu(void)
{
    MNU_displayMenu(CLI_menu);
}

void CLI_hexdump(void)
{
    char input_buffer[SF_CLI_MAX_CMD_LEN];
    char* pEndTok;
    const void* pBuffer;
    size_t buffer_length;
    SF_OSAL_printf("Starting address: 0x");
    getline(input_buffer, SF_CLI_MAX_CMD_LEN);
    pBuffer = (const void*)strtol(input_buffer, &pEndTok, 16);
    SF_OSAL_printf("Length: ");
    getline(input_buffer, SF_CLI_MAX_CMD_LEN);
    buffer_length = (size_t) strtol(input_buffer, &pEndTok, 10);
    hexDump(pBuffer, buffer_length);
}