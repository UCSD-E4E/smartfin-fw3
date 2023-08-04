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

#include "states.hpp"
#include "util.hpp"
#include "product.hpp"

#include "system.hpp"

#include <fstream>
#include <bits/stdc++.h>

#include "Particle.h"

void CLI_displayMenu(void);
void CLI_hexdump(void);

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
    lastKeyPressTime = millis();  

    SF_OSAL_printf(__NL__ ">");\

    while (1) 
    {
        if(millis() >= lastKeyPressTime + CLI_NO_INPUT_TIMEOUT_MS) 
        {
            CLI_nextState = STATE_CHARGE;
        }

        if(!pSystemDesc->flags->hasCharger)
        {
            return STATE_DEEP_SLEEP;
        }

        memset(userInput, 0, SF_CLI_MAX_CMD_LEN);        

        getline(userInput, SF_CLI_MAX_CMD_LEN);

        if(strlen(userInput) == 0)
        {
            continue; //No command, keep waiting
        }

        //Proccess Command
        SF_OSAL_printf("\r\n");
        cmd = MNU_findCommand(userInput, CLI_menu);

        if (!cmd) 
        {
            SF_OSAL_printf("Unknown command" __NL__);
            MNU_displayMenu(CLI_menu);
        } 
        else 
        {
            cmd->fn();
        }
        
        SF_OSAL_printf(__NL__ ">");
    }

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