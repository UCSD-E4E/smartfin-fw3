/**
 * Project smartfin-fw3
 * Description: Command Line interface for development, manufacturing tests, and user interaction
 * Author: Emily Thorpe
 * Date: Jul 20 2023
 *
 */

#include "cli.hpp"

#include "Particle.h"
#include "cli/flog.hpp"
#include "cliDebug.hpp"
#include "conio.hpp"
#include "consts.hpp"
#include "debug/recorder_debug.hpp"
#include "debug/session_debug.hpp"
#include "imu/imu.hpp"
#include "menu.hpp"
#include "menuItems/debugCommands.hpp"
#include "menuItems/gpsCommands.hpp"
#include "menuItems/systemCommands.hpp"
#include "product.hpp"
#include "rideTask.hpp"
#include "sleepTask.hpp"
#include "states.hpp"
#include "system.hpp"
#include "util.hpp"
#include "vers.hpp"

#include <bits/stdc++.h>
#include <fstream>

#define NUM_SENSORS 6

void CLI_displayMenu(void);
void CLI_hexdump(void);

static LEDStatus CLI_ledStatus;

static void CLI_setState(void);
static void CLI_displaySystemState(void);
static void CLI_dumpSystemState(void);
static void CLI_displayNVRAM(void);
static void CLI_sleepSetSleepBehavior(void);
static void CLI_sleepGetSleepBehavior(void);
static void CLI_displayResetReason(void);
static void CLI_monitorSensors(void);
static void CLI_doEnsemble(void);

static std::uint8_t packet_buffer[SF_PACKET_SIZE];
static char input_buffer[SF_CLI_MAX_CMD_LEN];
static char packet_name_buffer[particle::protocol::MAX_EVENT_NAME_LENGTH + 1];

const Menu_t CLI_menu[] = {
    {1, "display Menu", &CLI_displayMenu, MENU_CMD},
    {2, "disconnect particle", &CLI_disconnect, MENU_CMD},
    {3, "connect particle", &CLI_connect, MENU_CMD},
    {4, "show flog errors", &CLI_displayFLOG, MENU_CMD},
    {5, "test printf", &CLI_testPrintf, MENU_CMD},
    {6, "debug menu", {.pMenu = CLI_debugMenu}, MENU_SUBMENU},
    {7, "hexdump", &CLI_hexdump, MENU_CMD},
    {8, "gps", &CLI_GPS, MENU_CMD},
    {9, "sleep", &CLI_doSleep, MENU_CMD},
    {10, "Self Identify", &CLI_self_identify, MENU_CMD},
    {11, "check charge ports", &CLI_checkCharging, MENU_CMD},
    {12, "MFG Test", &CLI_doMfgTest, MENU_CMD},
    {13, "upload", &CLI_doUpload, MENU_CMD},
    {14, "Recorder Test Menu", {.pMenu = Recorder_debug_menu}, MENU_SUBMENU},
    {15, "Session Test Menu", {.pMenu = Session_debug_menu}, MENU_SUBMENU},
    {16, "Display all sensors", &CLI_monitorSensors, MENU_CMD},
    {20, "Do Ensemble Function", &CLI_doEnsemble, MENU_CMD},
    {100, "Set State", &CLI_setState, MENU_CMD},
    {101, "Display System State", &CLI_displaySystemState, MENU_CMD},
    {102, "Display NVRAM", &CLI_displayNVRAM, MENU_CMD},
    {103, "Dump System State", &CLI_dumpSystemState, MENU_CMD},
    {200, "Sleep - Set Sleep Behavior", &CLI_sleepSetSleepBehavior, MENU_CMD},
    {201, "Sleep - Get Sleep Behavior", &CLI_sleepGetSleepBehavior, MENU_CMD},
    {300, "Display Reset Reason", &CLI_displayResetReason, MENU_CMD},
    {0, nullptr, nullptr, MENU_NULL}};

STATES_e CLI_nextState;

void CLI::init(void)
{
    VERS_printBanner();

    CLI_nextState = STATE_CHARGE;

    pSystemDesc->pChargerCheck->start();

    CLI_ledStatus.setColor(CLI_RGB_LED_COLOR);
    CLI_ledStatus.setPattern(CLI_RGB_LED_PATTERN);
    CLI_ledStatus.setPeriod(CLI_RGB_LED_PERIOD);
    CLI_ledStatus.setPriority(CLI_RGB_LED_PRIORITY);
    CLI_ledStatus.setActive();

    // While there is an avaliable character typed, get it
    while (SF_OSAL_kbhit())
    {
        SF_OSAL_getch();
    }
}

STATES_e CLI::run(void)
{
    switch (MNU_executeMenu(CLI_menu))
    {
    case -1: // Abort due to USB terminal disconnect
        CLI_nextState = STATE_DEEP_SLEEP;
        break;
    default:
        break;
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
    char *pEndTok;
    const void *pBuffer;
    size_t buffer_length;
    SF_OSAL_printf("Starting address: 0x");
    SF_OSAL_getline(input_buffer, SF_CLI_MAX_CMD_LEN);
    pBuffer = (const void *)strtol(input_buffer, &pEndTok, 16);
    SF_OSAL_printf("Length: ");
    SF_OSAL_getline(input_buffer, SF_CLI_MAX_CMD_LEN);
    buffer_length = (size_t)strtol(input_buffer, &pEndTok, 10);
    hexDump(pBuffer, buffer_length);
}

static void CLI_setState(void)
{
    char *pEndTok;
    STATES_e nextState;

    for (int i = 1; i < STATE_N_STATES; i++)
    {
        SF_OSAL_printf("%3d: %s" __NL__, i, STATES_NAME_TAB[i]);
    }
    SF_OSAL_printf("Enter state to change to: ");
    SF_OSAL_getline(input_buffer, SF_CLI_MAX_CMD_LEN);
    nextState = (STATES_e)strtol(input_buffer, &pEndTok, 10);
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
static void CLI_dumpSystemState(void)
{
    SYS_dumpSys(0);
}

static void CLI_displayNVRAM(void)
{
    NVRAM &instance = NVRAM::getInstance();
    instance.displayNVRAM();
}

static void CLI_sleepSetSleepBehavior(void)
{
    char *pEndTok;
    SleepTask::BOOT_BEHAVIOR_e boot_behavior;
    SF_OSAL_printf("Boot Behavior Code: ");
    SF_OSAL_getline(input_buffer, SF_CLI_MAX_CMD_LEN);
    boot_behavior = (SleepTask::BOOT_BEHAVIOR_e)strtol(input_buffer, &pEndTok, 10);
    SleepTask::setBootBehavior(boot_behavior);
}

static void CLI_sleepGetSleepBehavior(void)
{
    SleepTask::BOOT_BEHAVIOR_e boot_behavior = SleepTask::getBootBehavior();
    SF_OSAL_printf("Boot Behavior: %s" __NL__, SleepTask::strBootBehavior(boot_behavior));
}

void CLI_displayResetReason(void)
{
    uint16_t reset_reason = System.resetReason();
    SF_OSAL_printf("Reset Reason: %hd", reset_reason);
    switch (reset_reason)
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

typedef struct
{
    const char *header;
    bool active;
    float value;
} CLI_MON_SENSOR_data_t;

CLI_MON_SENSOR_data_t sensor_headers[] = {{"ax", false, NAN},    // 0
                                          {"ay", false, NAN},    // 1
                                          {"az", false, NAN},    // 2
                                          {"gx", false, NAN},    // 3
                                          {"gy", false, NAN},    // 4
                                          {"gz", false, NAN},    // 5
                                          {"mx", false, NAN},    // 6
                                          {"my", false, NAN},    // 7
                                          {"mz", false, NAN},    // 8
                                          {"temp", false, NAN},  // 9
                                          {"wd cr", false, NAN}, // 10
                                          {"wd ls", false, NAN}, // 11
                                          {"dax", false, NAN},   // 12
                                          {"day", false, NAN},   // 13
                                          {"daz", false, NAN},   // 14
                                          {"a acc", false, NAN}, // 15
                                          {"dgx", false, NAN},   // 16
                                          {"dgy", false, NAN},   // 17
                                          {"dgz", false, NAN},   // 18
                                          {"dq1", false, NAN},   // 19
                                          {"dq2", false, NAN},   // 20
                                          {"dq3", false, NAN},   // 21
                                          {"dq0", false, NAN},   // 22
                                          {"dqacc", false, NAN}, // 23
                                          {NULL, false, NAN}};

/**
 * @brief CLI Monitor Sensors
 *
 * @todo This function needs to be refactored into a class for unit testing. Suspect bad data
 * acquisition
 *
 *
 */
static void CLI_monitorSensors(void)
{
    char ch = ' ';
    IMU_DMPData_t DMPData = {0};
    pSystemDesc->pChargerCheck->stop();
    pSystemDesc->pWaterCheck->stop();
    setupICM();
    SF_OSAL_printf(__NL__);

    typedef enum
    {
        ACCEL,
        GYRO,
        MAG,
        TEMP,
        WET_DRY,
        DMP
    } Sensor;
    bool sensors[NUM_SENSORS] = {false};

    SF_OSAL_printf("Enter delay time (ms): ");
    SF_OSAL_getline(input_buffer, SF_CLI_MAX_CMD_LEN);
    int delayTime = atoi(input_buffer);
    SF_OSAL_printf("Delay set to %d ms" __NL__, delayTime);
    SF_OSAL_printf("a - acceleraction, g - gyroscope, m - magnetometer, t - temp, w - wet/dry, d - "
                   "dmp" __NL__);
    bool valid = false;
    while (ch != 'x')
    {
        SF_OSAL_printf("Enter which sensors you want to look at (a, g, m, t, w, d), x to quit: ");
        ch = SF_OSAL_getch();
        SF_OSAL_printf("%c", ch);
        SF_OSAL_printf(__NL__);
        switch (ch)
        {
        case 'a':
            sensors[ACCEL] = true;
            valid = true;
            break;
        case 'g':
            sensors[GYRO] = true;
            valid = true;
            break;
        case 'm':
            sensors[MAG] = true;
            valid = true;
            break;
        case 't':
            sensors[TEMP] = true;
            valid = true;
            break;
        case 'w':
            sensors[WET_DRY] = true;
            valid = true;
            break;
        case 'd':
            sensors[DMP] = true;
            valid = true;
            break;
        case 'x':
            break;
        default:
            SF_OSAL_printf("invalid input" __NL__);
        }
    }
    SF_OSAL_printf(__NL__);
    // if no headers, return now
    if (!valid)
    {
        pSystemDesc->pChargerCheck->start();
        pSystemDesc->pWaterCheck->start();
        return;
    }

    for (CLI_MON_SENSOR_data_t *pEntry = sensor_headers; pEntry->header; pEntry++)
    {
        pEntry->active = false;
    }
    if (sensors[ACCEL])
    {
        sensor_headers[0].active = true;
        sensor_headers[1].active = true;
        sensor_headers[2].active = true;
    }
    if (sensors[GYRO])
    {
        sensor_headers[3].active = true;
        sensor_headers[4].active = true;
        sensor_headers[5].active = true;
    }
    if (sensors[MAG])
    {
        sensor_headers[6].active = true;
        sensor_headers[7].active = true;
        sensor_headers[8].active = true;
    }
    if (sensors[TEMP])
    {
        sensor_headers[9].active = true;
    }
    if (sensors[WET_DRY])
    {
        sensor_headers[10].active = true;
        sensor_headers[11].active = true;
    }
    if (sensors[DMP])
    {
        // acc
        sensor_headers[12].active = true;
        sensor_headers[13].active = true;
        sensor_headers[14].active = true;
        sensor_headers[15].active = true;

        // gyr
        sensor_headers[16].active = true;
        sensor_headers[17].active = true;
        sensor_headers[18].active = true;

        // quat
        sensor_headers[19].active = true;
        sensor_headers[20].active = true;
        sensor_headers[21].active = true;
        sensor_headers[22].active = true;
        sensor_headers[23].active = true;
    }
    int count = 0;

    while (1)
    {
        if (SF_OSAL_kbhit())
        {
            ch = SF_OSAL_getch();

            if ('q' == ch)
            {
                break;
            }
        }
        if (sensors[ACCEL])
        {
            if (!getAccelerometer(
                    &sensor_headers[0].value, &sensor_headers[1].value, &sensor_headers[2].value))
            {
                sensor_headers[0].value = NAN;
                sensor_headers[1].value = NAN;
                sensor_headers[2].value = NAN;
            }
        }
        if (sensors[GYRO])
        {
            getGyroscope(
                &sensor_headers[3].value, &sensor_headers[4].value, &sensor_headers[5].value);
        }
        if (sensors[MAG])
        {
            getMagnetometer(
                &sensor_headers[6].value, &sensor_headers[7].value, &sensor_headers[8].value);
        }
        if (sensors[DMP])
        {
            delayTime = 0;
            DMPData.acc[0] = NAN;
            DMPData.acc[1] = NAN;
            DMPData.acc[2] = NAN;
            DMPData.acc_acc = NAN;
            DMPData.gyr[0] = NAN;
            DMPData.gyr[1] = NAN;
            DMPData.gyr[2] = NAN;
            DMPData.mag[0] = NAN;
            DMPData.mag[1] = NAN;
            DMPData.mag[2] = NAN;
            DMPData.quat[0] = NAN;
            DMPData.quat[1] = NAN;
            DMPData.quat[2] = NAN;
            DMPData.quat[3] = NAN;
            DMPData.quat_acc = NAN;
            if (!getDMPData(DMPData))
            {
                SF_OSAL_printf("DMP fail!" __NL__);
                FLOG_AddError(FLOG_ICM_FAIL, 1);
            }
            sensor_headers[12].value = DMPData.acc[0];
            sensor_headers[13].value = DMPData.acc[1];
            sensor_headers[14].value = DMPData.acc[2];

            sensor_headers[15].value = DMPData.acc_acc;

            sensor_headers[16].value = DMPData.gyr[0];
            sensor_headers[17].value = DMPData.gyr[1];
            sensor_headers[18].value = DMPData.gyr[2];

            sensor_headers[19].value = DMPData.quat[0];
            sensor_headers[20].value = DMPData.quat[1];
            sensor_headers[21].value = DMPData.quat[2];
            sensor_headers[22].value = DMPData.quat[3];

            sensor_headers[23].value = DMPData.quat_acc;
        }
        if (sensors[TEMP])
        {
            sensor_headers[9].value = pSystemDesc->pTempSensor->getTemp();
        }
        sensor_headers[10].value = NAN;
        sensor_headers[11].value = NAN;
        if (sensors[WET_DRY])
        {
            pSystemDesc->pWaterSensor->update();
            sensor_headers[10].value = pSystemDesc->pWaterSensor->getLastReading();
            sensor_headers[11].value = pSystemDesc->pWaterSensor->getLastStatus();
        }

        if (count % 10 == 0)
        {
            for (CLI_MON_SENSOR_data_t *it = sensor_headers; it->header; it++)
            {
                if (it->active)
                {
                    SF_OSAL_printf("|   %s    |\t", it->header);
                }
            }
            SF_OSAL_printf(__NL__);
        }
        for (CLI_MON_SENSOR_data_t *it = sensor_headers; it->header; it++)
        {
            if (it->active)
            {
                SF_OSAL_printf(" %8.4f\t", it->value);
            }
        }
        SF_OSAL_printf(__NL__);
        count++;
        delay(delayTime);
    }
    pSystemDesc->pChargerCheck->start();
    pSystemDesc->pWaterCheck->start();
}

static void CLI_doEnsemble(void)
{
    int idx = 0;

    for (; deploymentSchedule[idx].init; idx++)
    {
        SF_OSAL_printf("%3d: %s" __NL__, idx, deploymentSchedule[idx].taskName);
    }
    SF_OSAL_printf("Enter ensemble to run: ");
    SF_OSAL_getline(input_buffer, SF_CLI_MAX_CMD_LEN);
    int user_input = atoi(input_buffer);
    if (user_input < 0 || user_input >= idx)
    {
        SF_OSAL_printf("Invalid index" __NL__);
        return;
    }
    DeploymentSchedule_t &ensemble = deploymentSchedule[user_input];
    SF_OSAL_printf("Running %s" __NL__, ensemble.taskName);

    pSystemDesc->pChargerCheck->stop();
    pSystemDesc->pWaterCheck->stop();
    setupICM();
    pSystemDesc->pRecorder->openSession();
    SYS_dumpSys(2);
    ensemble.init(&ensemble);
    ensemble.measure(&ensemble);
    SYS_dumpSys(2);
    pSystemDesc->pRecorder->closeSession();
    pSystemDesc->pChargerCheck->start();
    pSystemDesc->pWaterCheck->start();
    SF_OSAL_printf("Done" __NL__);
    int nBytes = pSystemDesc->pRecorder->getLastPacket(packet_buffer,
                                                       SF_PACKET_SIZE,
                                                       packet_name_buffer,
                                                       particle::protocol::MAX_EVENT_NAME_LENGTH);
    if (nBytes < 0)
    {
        SF_OSAL_printf("Failed to get last packet: %d" __NL__, nBytes);
        return;
    }
    SF_OSAL_printf("Packet name: %s" __NL__, packet_name_buffer);
    SF_OSAL_printf("Packet size: %d" __NL__, nBytes);
    SF_OSAL_printf("Packet data:" __NL__);
    hexDump(packet_buffer, nBytes);
    pSystemDesc->pRecorder->popLastPacket(nBytes);
}