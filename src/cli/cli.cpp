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
#include "imu/newIMU.hpp"
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
#include <cstdlib>
#include <fstream>

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
static void CLI_setWaterSensorWindow(void);

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
    {30, "Dump FLOG", &CLI_displayFLOG, MENU_CMD},
    {31, "Clear FLOG", &CLI_clearFLOG, MENU_CMD},
    {100, "Set State", &CLI_setState, MENU_CMD},
    {101, "Display System State", &CLI_displaySystemState, MENU_CMD},
    {102, "Display NVRAM", &CLI_displayNVRAM, MENU_CMD},
    {103, "Dump System State", &CLI_dumpSystemState, MENU_CMD},
    {200, "Sleep - Set Sleep Behavior", &CLI_sleepSetSleepBehavior, MENU_CMD},
    {201, "Sleep - Get Sleep Behavior", &CLI_sleepGetSleepBehavior, MENU_CMD},
    {300, "Display Reset Reason", &CLI_displayResetReason, MENU_CMD},
    {400, "Set Water Sensor Window", &CLI_setWaterSensorWindow, MENU_CMD},
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

/**
 * @brief MonitorSensors header enum
 *
 */
enum SensorHeader
{
    SensorHeader_Time,
    SensorHeader_AccelX,
    SensorHeader_AccelY,
    SensorHeader_AccelZ,
    SensorHeader_GyroX,
    SensorHeader_GyroY,
    SensorHeader_GyroZ,
    SensorHeader_MagX,
    SensorHeader_MagY,
    SensorHeader_MagZ,
    SensorHeader_Temp,
    SensorHeader_WetDryReading,
    SensorHeader_WetDryStatus,
    SensorHeader_DMPAccelX,
    SensorHeader_DMPAccelY,
    SensorHeader_DMPAccelZ,
    SensorHeader_DMPAccelAcc,
    SensorHeader_DMPGyroX,
    SensorHeader_DMPGyroY,
    SensorHeader_DMPGyroZ,
    SensorHeader_DMPQuat1,
    SensorHeader_DMPQuat2,
    SensorHeader_DMPQuat3,
    SensorHeader_DMPQuat0,
    SensorHeader_DMPQuatAcc,
    SensorHeader_DMPMagX,
    SensorHeader_DMPMagY,
    SensorHeader_DMPMagZ,
    SensorHeader_GpsLock,
    SensorHeader_GpsNSats,
    SensorHeader_GpsLat,
    SensorHeader_GpsLon,
    SensorHeader_GpsAlt,

    SensorHeader_NUMHEADERS
};

/**
 * @brief Monitor sensor table row definition
 *
 */
typedef struct
{
    /**
     * @brief
     * Header index
     *
     */
    enum SensorHeader header_idx;
    /**
     * @brief Column header name
     *
     */
    const char *header;
    /**
     * @brief Active flag
     *
     */
    bool active;
    /**
     * @brief Recorded value to display
     *
     */
    float value;
} CLI_MON_SENSOR_data_t;

/**
 * @brief Monitor sensor table definition
 *
 */
CLI_MON_SENSOR_data_t sensor_headers[SensorHeader_NUMHEADERS + 1] = {
    {SensorHeader_Time, "t", false, NAN},
    {SensorHeader_AccelX, "ax", false, NAN},           // 0
    {SensorHeader_AccelY, "ay", false, NAN},           // 1
    {SensorHeader_AccelZ, "az", false, NAN},           // 2
    {SensorHeader_GyroX, "gx", false, NAN},            // 3
    {SensorHeader_GyroY, "gy", false, NAN},            // 4
    {SensorHeader_GyroZ, "gz", false, NAN},            // 5
    {SensorHeader_MagX, "mx", false, NAN},             // 6
    {SensorHeader_MagX, "my", false, NAN},             // 7
    {SensorHeader_MagX, "mz", false, NAN},             // 8
    {SensorHeader_Temp, "temp", false, NAN},           // 9
    {SensorHeader_WetDryReading, "wd cr", false, NAN}, // 10
    {SensorHeader_WetDryStatus, "wd ls", false, NAN},  // 11
    {SensorHeader_DMPAccelX, "dax", false, NAN},       // 12
    {SensorHeader_DMPAccelY, "day", false, NAN},       // 13
    {SensorHeader_DMPAccelZ, "daz", false, NAN},       // 14
    {SensorHeader_DMPAccelAcc, "a acc", false, NAN},   // 15
    {SensorHeader_DMPGyroX, "dgx", false, NAN},        // 16
    {SensorHeader_DMPGyroY, "dgy", false, NAN},        // 17
    {SensorHeader_DMPGyroZ, "dgz", false, NAN},        // 18
    {SensorHeader_DMPQuat1, "dq1", false, NAN},        // 19
    {SensorHeader_DMPQuat2, "dq2", false, NAN},        // 20
    {SensorHeader_DMPQuat3, "dq3", false, NAN},        // 21
    {SensorHeader_DMPQuat0, "dq0", false, NAN},        // 22
    {SensorHeader_DMPQuatAcc, "dqacc", false, NAN},    // 23
    {SensorHeader_DMPMagX, "dmx", false, NAN},         // 24
    {SensorHeader_DMPMagY, "dmy", false, NAN},         // 25
    {SensorHeader_DMPMagZ, "dmz", false, NAN},         // 26
    {SensorHeader_GpsLock, "lock", false, NAN},        // 27
    {SensorHeader_GpsNSats, "nSats", false, NAN},      // 28
    {SensorHeader_GpsLat, "lat", false, NAN},          // 29
    {SensorHeader_GpsLon, "lon", false, NAN},          // 30
    {SensorHeader_GpsAlt, "alt", false, NAN},          // 31
    {SensorHeader_NUMHEADERS, NULL, false, NAN}};

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
    pSystemDesc->pChargerCheck->stop();
    pSystemDesc->pWaterCheck->stop();
    SF_OSAL_printf(__NL__);

    typedef enum
    {
        TIME,
        ACCEL,
        GYRO,
        MAG,
        TEMP,
        WET_DRY,
        DMP,
        GPS,
        NUM_SENSORS
    } Sensor;
    bool sensors[NUM_SENSORS] = {false};

    LocationPoint point;

    SF_OSAL_printf("Enter delay time (ms): ");
    SF_OSAL_getline(input_buffer, SF_CLI_MAX_CMD_LEN);
    int delayTime = atoi(input_buffer);
    SF_OSAL_printf("Delay set to %d ms" __NL__, delayTime);
    SF_OSAL_printf("a - acceleraction, g - gyroscope, m - magnetometer, t - temp, w - wet/dry, d - "
                   "dmp, G - GPS" __NL__);
    sensors[TIME] = true;
    bool valid = false;
    while (ch != 'x')
    {
        SF_OSAL_printf(
            "Enter which sensors you want to look at (a, g, m, t, w, d, G), x to quit: ");
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
        case 'G':
            sensors[GPS] = true;
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
    sensor_headers[SensorHeader_Time].active = true;
    if (sensors[ACCEL])
    {
        sensor_headers[SensorHeader_AccelX].active = true;
        sensor_headers[SensorHeader_AccelY].active = true;
        sensor_headers[SensorHeader_AccelZ].active = true;
    }
    if (sensors[GYRO])
    {
        sensor_headers[SensorHeader_GyroX].active = true;
        sensor_headers[SensorHeader_GyroY].active = true;
        sensor_headers[SensorHeader_GyroZ].active = true;
    }
    if (sensors[MAG])
    {
        sensor_headers[SensorHeader_MagX].active = true;
        sensor_headers[SensorHeader_MagY].active = true;
        sensor_headers[SensorHeader_MagZ].active = true;
    }
    if (sensors[TEMP])
    {
        sensor_headers[SensorHeader_Temp].active = true;
    }
    if (sensors[WET_DRY])
    {
        sensor_headers[SensorHeader_WetDryStatus].active = true;
        sensor_headers[SensorHeader_WetDryReading].active = true;
    }
    if (sensors[DMP])
    {
        // acc
        sensor_headers[SensorHeader_DMPAccelX].active = true;
        sensor_headers[SensorHeader_DMPAccelY].active = true;
        sensor_headers[SensorHeader_DMPAccelZ].active = true;
        sensor_headers[SensorHeader_DMPAccelAcc].active = true;

        // gyr
        sensor_headers[SensorHeader_DMPGyroX].active = true;
        sensor_headers[SensorHeader_DMPGyroY].active = true;
        sensor_headers[SensorHeader_DMPGyroZ].active = true;

        // quat
        sensor_headers[SensorHeader_DMPQuat0].active = true;
        sensor_headers[SensorHeader_DMPQuat1].active = true;
        sensor_headers[SensorHeader_DMPQuat2].active = true;
        sensor_headers[SensorHeader_DMPQuat3].active = true;
        sensor_headers[SensorHeader_DMPQuatAcc].active = true;

        // mag
        sensor_headers[SensorHeader_DMPMagX].active = true;
        sensor_headers[SensorHeader_DMPMagY].active = true;
        sensor_headers[SensorHeader_DMPMagZ].active = true;
    }
    if (sensors[GPS])
    {
        // gps
        sensor_headers[SensorHeader_GpsLock].active = true;
        sensor_headers[SensorHeader_GpsNSats].active = true;
        sensor_headers[SensorHeader_GpsLat].active = true;
        sensor_headers[SensorHeader_GpsLon].active = true;
        sensor_headers[SensorHeader_GpsAlt].active = true;
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
        sensor_headers[SensorHeader_Time].value = millis();
        if (sensors[ACCEL])
        {
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
            pSystemDesc->pIMU->getAccel_ms2(sensor_headers[SensorHeader_AccelX].value,
                                            sensor_headers[SensorHeader_AccelY].value,
                                            sensor_headers[SensorHeader_AccelZ].value);
#endif
        }
        if (sensors[GYRO])
        {
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
            pSystemDesc->pIMU->getRotVel_dps(sensor_headers[SensorHeader_GyroX].value,
                                             sensor_headers[SensorHeader_GyroY].value,
                                             sensor_headers[SensorHeader_GyroZ].value);
#endif
        }
        if (sensors[MAG])
        {
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
            pSystemDesc->pIMU->getMag_uT(sensor_headers[SensorHeader_MagX].value,
                                         sensor_headers[SensorHeader_MagY].value,
                                         sensor_headers[SensorHeader_MagZ].value);
#endif
        }
        if (sensors[DMP])
        {
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
            pSystemDesc->pIMU->getDmpAccel_ms2(sensor_headers[SensorHeader_DMPAccelX].value,
                                               sensor_headers[SensorHeader_DMPAccelY].value,
                                               sensor_headers[SensorHeader_DMPAccelZ].value);
            pSystemDesc->pIMU->getDmpRotVel_dps(sensor_headers[SensorHeader_DMPGyroX].value,
                                                sensor_headers[SensorHeader_DMPGyroY].value,
                                                sensor_headers[SensorHeader_DMPGyroZ].value);
            pSystemDesc->pIMU->getDmpQuatf(sensor_headers[SensorHeader_DMPQuat0].value,
                                           sensor_headers[SensorHeader_DMPQuat1].value,
                                           sensor_headers[SensorHeader_DMPQuat2].value,
                                           sensor_headers[SensorHeader_DMPQuat3].value,
                                           &sensor_headers[SensorHeader_DMPQuatAcc].value);
            pSystemDesc->pIMU->getDmpMag_uT(sensor_headers[SensorHeader_DMPMagX].value,
                                            sensor_headers[SensorHeader_DMPMagY].value,
                                            sensor_headers[SensorHeader_DMPMagZ].value);
#endif
        }
        if (sensors[TEMP])
        {
            sensor_headers[SensorHeader_Temp].value = pSystemDesc->pTempSensor->getTemp();
        }
        sensor_headers[SensorHeader_WetDryReading].value = NAN;
        sensor_headers[SensorHeader_WetDryStatus].value = NAN;
        if (sensors[WET_DRY])
        {
            pSystemDesc->pWaterSensor->update();
            sensor_headers[SensorHeader_WetDryReading].value =
                pSystemDesc->pWaterSensor->getLastReading();
            sensor_headers[SensorHeader_WetDryStatus].value =
                pSystemDesc->pWaterSensor->getLastStatus();
        }
        if (sensors[GPS])
        {
            pSystemDesc->pLocService->getLocation(point);
            sensor_headers[SensorHeader_GpsLock].value = point.locked;
            sensor_headers[SensorHeader_GpsNSats].value = point.satsInUse;
            sensor_headers[SensorHeader_GpsLat].value = point.latitude;
            sensor_headers[SensorHeader_GpsLon].value = point.longitude;
            sensor_headers[SensorHeader_GpsAlt].value = point.altitude;
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

/**
 * @brief CLI Command to set water sensor window
 *
 * @param
 */
void CLI_setWaterSensorWindow(void)
{
    uint8_t window_length;
    if (!pSystemDesc->pNvram->get(NVRAM::WATER_DETECT_WINDOW_LEN, window_length))
    {
        SF_OSAL_printf("Failed to retrieve window length from NVRAM, defaulting" __NL__);
        window_length = WATER_DETECT_SURF_SESSION_INIT_WINDOW;
    }
    SF_OSAL_printf("Current window length: %hu" __NL__, window_length);
    SF_OSAL_printf("Enter new window length: ");
    if (SF_OSAL_getline(input_buffer, SF_CLI_MAX_CMD_LEN) == 0)
    {
        // empty, no change
        return;
    }
    // non-empty, interpret input.  Auto-detect base
    const long input = strtol(input_buffer, NULL, 0);
    if (errno == ERANGE)
    {
        SF_OSAL_printf("Unable to interpret input" __NL__);
        return;
    }
    if (input > UINT8_MAX || input > WATER_DETECT_ARRAY_SIZE)
    {
        SF_OSAL_printf("Requested window length exceeds %u" __NL__,
                       UINT8_MAX < WATER_DETECT_ARRAY_SIZE ? UINT8_MAX : WATER_DETECT_ARRAY_SIZE);
    }
    if (input < 1)
    {
        SF_OSAL_printf("Negative or zero window values not supported!" __NL__);
    }
    window_length = (uint8_t)input;
    if (!pSystemDesc->pNvram->put(NVRAM::WATER_DETECT_WINDOW_LEN, window_length))
    {
        SF_OSAL_printf("Failed to write value!" __NL__);
    }
    pSystemDesc->pWaterSensor->setWindowSize(window_length);
    pSystemDesc->pWaterSensor->resetArray();
}
