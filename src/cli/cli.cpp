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
#include "vers.hpp"
#include "product.hpp"
#include "sleepTask.hpp"
#include "system.hpp"

#include "system.hpp"

#include "system.hpp"


#include <fstream>
#include <bits/stdc++.h>

#include "Particle.h"

void CLI_displayMenu(void);
void CLI_manageInput(char* inputBuffer);
void CLI_hexdump(void);

static LEDStatus CLI_ledStatus;


static void CLI_setState(void);
static void CLI_displaySystemState(void);
static void CLI_displayNVRAM(void);
static void CLI_sleepSetSleepBehavior(void);
static void CLI_sleepGetSleepBehavior(void);
static void CLI_displayResetReason(void);

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
    {10, "Self Identify", &CLI_self_identify},
    {11, "check charge ports", &CLI_checkCharging},
    {100, "Set State", &CLI_setState},
    {101, "Display System State", &CLI_displaySystemState},
    {102, "Display NVRAM", &CLI_displayNVRAM},
    {200, "Sleep - Set Sleep Behavior", &CLI_sleepSetSleepBehavior},
    {201, "Sleep - Get Sleep Behavior", &CLI_sleepGetSleepBehavior},
    {300, "Display Reset Reason", &CLI_displayResetReason},
    {0, nullptr, nullptr}
};

char userInput[SF_CLI_MAX_CMD_LEN];

STATES_e CLI_nextState;

void CLI::init(void) 
{
    VERS_printBanner();

    CLI_nextState = STATE_CLI;

    pSystemDesc->pChargerCheck->start();

    CLI_ledStatus.setColor(CLI_RGB_LED_COLOR);
    CLI_ledStatus.setPattern(CLI_RGB_LED_PATTERN);
    CLI_ledStatus.setPeriod(CLI_RGB_LED_PERIOD);
    CLI_ledStatus.setPriority(CLI_RGB_LED_PRIORITY);
    CLI_ledStatus.setActive();

    // While there is an avaliable character typed, get it
    while (kbhit())
    {
        getch();
    }
}


STATES_e CLI::run(void)
{
    uint32_t lastKeyPressTime;
    userInput[0] = 0;
    size_t inputIdx = 0;

    lastKeyPressTime = millis();  

    SF_OSAL_printf(__NL__ ">");

    CLI_nextState = STATE_CLI;

    memset(userInput, 0, SF_CLI_MAX_CMD_LEN);        
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

        
        if (kbhit())
        {
            char userChar = getch();
            lastKeyPressTime = millis();
            switch (userChar)
            {
                case '\b':
                    inputIdx = (inputIdx <= 0) ? 0 : inputIdx - 1;
                    SF_OSAL_printf("\b \b");
                    break;
                case '\r':
                case '\n':
                    userInput[inputIdx++] = 0;
                    SF_OSAL_printf(__NL__);
                    CLI_manageInput(userInput);
                    SF_OSAL_printf(__NL__ ">");
                    inputIdx = 0;
                    memset(userInput, 0, SF_CLI_MAX_CMD_LEN);        
                    break;
                default:
                    userInput[inputIdx++] = userChar;
                    putch(userChar);
                    break;
            }
        }
    }

    SF_OSAL_printf("Next State: %d", CLI_nextState);
    return CLI_nextState;
}

void CLI_manageInput(char* inputBuffer)
{
    Menu_t *cmd;
    cmd = MNU_findCommand(inputBuffer, CLI_menu);
    
    if (!cmd) 
    {
        SF_OSAL_printf("Unknown command" __NL__);
        MNU_displayMenu(CLI_menu);
    } 
    else 
    {
        cmd->fn();
    }
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

static void CLI_setState(void)
{
    char input_buffer[SF_CLI_MAX_CMD_LEN];
    char* pEndTok;
    STATES_e nextState;
    
    for (int i = 1; i < STATE_N_STATES; i++)
    {
        SF_OSAL_printf("%3d: %s" __NL__, i, STATES_NAME_TAB[i]);
    }
    SF_OSAL_printf("Enter state to change to: ");
    getline(input_buffer, SF_CLI_MAX_CMD_LEN);
    nextState = (STATES_e) strtol(input_buffer, &pEndTok, 10);
    if (nextState == 0)
    {
        SF_OSAL_printf("Invalid state" __NL__);
        return;
    }
    CLI_nextState = nextState;
    SF_OSAL_printf("Switching to %s" __NL__, STATES_NAME_TAB[nextState]);
    return;
}

static void CLI_displaySystemState(void)
{
    SYS_displaySys();
}

static void CLI_displayNVRAM(void)
{
    NVRAM& instance = NVRAM::getInstance();
    instance.displayNVRAM();
}

static void CLI_sleepSetSleepBehavior(void)
{
    char input_buffer[SF_CLI_MAX_CMD_LEN];
    char* pEndTok;
    SleepTask::BOOT_BEHAVIOR_e boot_behavior;
    SF_OSAL_printf("Boot Behavior Code: ");
    getline(input_buffer, SF_CLI_MAX_CMD_LEN);
    boot_behavior = (SleepTask::BOOT_BEHAVIOR_e) strtol(input_buffer, &pEndTok, 10);
    SleepTask::setBootBehavior(boot_behavior);
}

static void CLI_sleepGetSleepBehavior(void)
{
    SleepTask::BOOT_BEHAVIOR_e boot_behavior =  SleepTask::getBootBehavior();
    SF_OSAL_printf("Boot Behavior: %s" __NL__, SleepTask::strBootBehavior(boot_behavior));
}

void CLI_displayResetReason(void)
{
    uint16_t reset_reason = System.resetReason();
    SF_OSAL_printf("Reset Reason: %hd", reset_reason);
    switch(reset_reason)
    {
        case RESET_REASON_PIN_RESET:
            SF_OSAL_printf("nRESET Assertion");
            break;
        case RESET_REASON_POWER_MANAGEMENT:
            SF_OSAL_printf("Low Power Management Reset");
            break;
        case RESET_REASON_POWER_DOWN:
            SF_OSAL_printf("Power-down Reset");
            break;
        case RESET_REASON_POWER_BROWNOUT:
            SF_OSAL_printf("Brownout Reset");
            break;
        case RESET_REASON_WATCHDOG:
            SF_OSAL_printf("Watchdog Reset");
            break;
        case RESET_REASON_UPDATE:
            SF_OSAL_printf("FW Update Success");
            break;
        case RESET_REASON_UPDATE_TIMEOUT:
            SF_OSAL_printf("FW Update Timeout");
            break;
        case RESET_REASON_FACTORY_RESET:
            SF_OSAL_printf("Factory Reset");
            break;
        case RESET_REASON_SAFE_MODE:
            SF_OSAL_printf("Safe Mode");
            break;
        case RESET_REASON_DFU_MODE:
            SF_OSAL_printf("DFU mode");
            break;
        case RESET_REASON_PANIC:
            SF_OSAL_printf("System Panic");
            break;
        case RESET_REASON_USER:
            SF_OSAL_printf("User Reset");
            break;
        case RESET_REASON_NONE:
            SF_OSAL_printf("No info available");
            break;
        case RESET_REASON_UNKNOWN:
        default:
            SF_OSAL_printf("Unknown Reset");
            break;
    }
    SF_OSAL_printf(__NL__);
}