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
static void CLI_displayNVRAM(void);
static void CLI_sleepSetSleepBehavior(void);
static void CLI_sleepGetSleepBehavior(void);
static void CLI_displayResetReason(void);
static void CLI_monitorSensors(void);
static void CLI_doEnsemble(void);

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
    {200, "Sleep - Set Sleep Behavior", &CLI_sleepSetSleepBehavior, MENU_CMD},
    {201, "Sleep - Get Sleep Behavior", &CLI_sleepGetSleepBehavior, MENU_CMD},
    {300, "Display Reset Reason", &CLI_displayResetReason, MENU_CMD},
    {0, nullptr, nullptr, MENU_NULL}};

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
    while (SF_OSAL_kbhit())
    {
        SF_OSAL_getch();
    }
}

STATES_e CLI::run(void)
{
    MNU_executeMenu(CLI_menu);
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
    char input_buffer[SF_CLI_MAX_CMD_LEN];
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

static void CLI_displayNVRAM(void)
{
    NVRAM &instance = NVRAM::getInstance();
    instance.displayNVRAM();
}

static void CLI_sleepSetSleepBehavior(void)
{
    char input_buffer[SF_CLI_MAX_CMD_LEN];
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
    char ch;
    float accelData[3] = {0, 0, 0};
    // float accelDMPData[4] = {0, 0, 0, 0};
    float gyroData[3] = {0, 0, 0};
    // float gyroDMPData[3] = {0, 0, 0};
    float magData[3] = {0, 0, 0};
    // double quatData[5] = {0, 0, 0, 0, 0};
    IMU_DMPData_t DMPData = {0};
    float tmpData = 0;
    float wdCR = 0;
    float wdLS = 0;
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
    char dt[SF_CLI_MAX_CMD_LEN];
    SF_OSAL_getline(dt, SF_CLI_MAX_CMD_LEN);
    int delayTime = atoi(dt);
    SF_OSAL_printf("Delay set to %d ms" __NL__, delayTime);
    SF_OSAL_printf("a - acceleraction, g - gyroscope, m - magnetometer, t - temp, w - wet/dry, d - "
                   "dmp" __NL__);
    bool valid = true;
    while (valid)
    {
        SF_OSAL_printf("Enter which sensors you want to look at (a, g, m, t, w, d), x to quit: ");
        ch = SF_OSAL_getch();
        SF_OSAL_printf("%c", ch);
        SF_OSAL_printf(__NL__);
        switch (ch)
        {
        case 'a':
            sensors[ACCEL] = true;
            break;
        case 'g':
            sensors[GYRO] = true;
            break;
        case 'm':
            sensors[MAG] = true;
            break;
        case 't':
            sensors[TEMP] = true;
            break;
        case 'w':
            sensors[WET_DRY] = true;
            break;
        case 'd':
            sensors[DMP] = true;
            break;
        case 'x':
            valid = false;
            break;
        default:
            SF_OSAL_printf("invalid input" __NL__);
        }
    }
    SF_OSAL_printf(__NL__);
    std::vector<std::string> headers;
    if (sensors[ACCEL])
    {
        headers.push_back("ax");
        headers.push_back("ay");
        headers.push_back("az");
    }
    if (sensors[GYRO])
    {
        headers.push_back("gx");
        headers.push_back("gy");
        headers.push_back("gz");
    }
    if (sensors[MAG])
    {
        headers.push_back("mx");
        headers.push_back("my");
        headers.push_back("mz");
    }
    if (sensors[TEMP])
    {
        headers.push_back("temp");
    }
    if (sensors[WET_DRY])
    {
        headers.push_back("wd lr");
        headers.push_back("wd ls");
    }
    if (sensors[DMP])
    {
        // headers.push_back("dq1");
        // headers.push_back("dq2");
        // headers.push_back("dq3");
        // headers.push_back("dq0");
        // headers.push_back("dqacc");
        headers.push_back("dax");
        headers.push_back("day");
        headers.push_back("daz");
        headers.push_back("a acc");
        // headers.push_back("dcx");
        // headers.push_back("dcy");
        // headers.push_back("dcz");
    }

    // if no headers, return now
    if (headers.size() == 0)
    {
        return;
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
            if (!getAccelerometer(accelData, accelData + 1, accelData + 2))
            {
                accelData[0] = nanf("");
            }
        }
        if (sensors[GYRO])
        {
            getGyroscope(gyroData, gyroData + 1, gyroData + 2);
        }
        if (sensors[MAG])
        {
            getMagnetometer(magData, magData + 1, magData + 2);
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
            // if (!getDMPAccelerometer(accelDMPData, accelDMPData + 1, accelDMPData + 2))
            // {
            //     accelDMPData[0] = nanf("");
            //     accelDMPData[1] = nanf("");
            //     accelDMPData[2] = nanf("");
            // }
            // if (!getDMPAccelerometerAcc(accelDMPData + 3))
            // {
            //     accelDMPData[3] = nanf("");
            // }
            // getDMPGyroscope(gyroDMPData, gyroDMPData + 1, gyroDMPData + 2);
            // getDMPQuaternion(quatData, quatData + 1, quatData + 2, quatData + 3, quatData + 4);
        }
        if (sensors[TEMP])
        {
            tmpData = pSystemDesc->pTempSensor->getTemp();
        }
        if (sensors[WET_DRY])
        {
            wdCR = pSystemDesc->pWaterSensor->getCurrentReading();
            wdLS = pSystemDesc->pWaterSensor->getLastStatus();
        }

        std::map<std::string, float> sensorData = {{"ax", accelData[0]},
                                                   {"ay", accelData[1]},
                                                   {"az", accelData[2]},
                                                   {"gx", gyroData[0]},
                                                   {"gy", gyroData[1]},
                                                   {"gz", gyroData[2]},
                                                   {"mx", magData[0]},
                                                   {"my", magData[1]},
                                                   {"mz", magData[2]},
                                                   {"temp", tmpData},
                                                   {"wd cr", wdCR},
                                                   {"wd ls", wdLS},

                                                   {"dax", DMPData.acc[0]},
                                                   {"day", DMPData.acc[1]},
                                                   {"daz", DMPData.acc[2]},
                                                   {"a acc", DMPData.acc_acc},
                                                   {"dgx", DMPData.gyr[0]},
                                                   {"dgy", DMPData.gyr[1]},
                                                   {"dgz", DMPData.gyr[2]},
                                                   {"dq1", DMPData.quat[0]},
                                                   {"dq2", DMPData.quat[1]},
                                                   {"dq3", DMPData.quat[2]},
                                                   {"dq0", DMPData.quat[3]},
                                                   {"dqacc", DMPData.quat_acc}

        };
        if (count % 10 == 0)
        {
            for (const auto &header : headers)
            {
                SF_OSAL_printf("|   %s    |\t", header.c_str());
            }
            SF_OSAL_printf(__NL__);
        }
        for (const auto &header : headers)
        {
            SF_OSAL_printf(" %8.4f\t", sensorData.at(header));
        }
        SF_OSAL_printf(__NL__);
        count++;
        delay(delayTime);
    }
}

static void CLI_doEnsemble(void)
{
    char input_buffer[SF_CLI_MAX_CMD_LEN];
    std::uint8_t packet_buffer[SF_PACKET_SIZE];
    char packet_name_buffer[particle::protocol::MAX_EVENT_NAME_LENGTH + 1];
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

    pSystemDesc->pRecorder->openSession();
    ensemble.init(&ensemble);
    ensemble.measure(&ensemble);
    pSystemDesc->pRecorder->closeSession();
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