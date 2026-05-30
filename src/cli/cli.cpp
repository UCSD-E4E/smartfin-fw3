/**
 * Project smartfin-fw3
 * Description: Command Line interface for development, manufacturing tests, and user interaction
 * Author: Emily Thorpe
 * Date: Jul 20 2023
 *
 */

#include "cli.hpp"

#include "Particle.h"
#include "ble/ble_live_stream.hpp"
#include "ble/high_rate_stream.hpp"
#include "ble/sf_ble.hpp"
#include "cellular/sf_cloud.hpp"
#include "cli/flog.hpp"
#include "cliDebug.hpp"
#include "conio.hpp"
#include "consts.hpp"
#if SF_ENABLE_DEBUG_MENUS
#include "debug/recorder_debug.hpp"
#include "debug/session_debug.hpp"
#endif
#include "deploy/ensembleTypes.hpp"
#include "imu/newIMU.hpp"
#include "menu.hpp"
#include "menuItems/debugCommands.hpp"
#if SF_ENABLE_GPS
#include "menuItems/gpsCommands.hpp"
#endif
#include "menuItems/systemCommands.hpp"
#include "product.hpp"
#include "rideTask.hpp"
#include "sleepTask.hpp"
#include "states.hpp"
#include "system.hpp"
#include "util.hpp"
#include "vers.hpp"

#include <atomic>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cstdint>

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
#if SF_ENABLE_DEBUG_MENUS
static void CLI_monitorSensors(void);
#endif
static void CLI_doEnsemble(void);
static void CLI_setWaterSensorWindow(void);
static void CLI_doBleTest(void);

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
#if SF_ENABLE_GPS
    {8, "gps", &CLI_GPS, MENU_CMD},
#endif
    {9, "sleep", &CLI_doSleep, MENU_CMD},
    {10, "Self Identify", &CLI_self_identify, MENU_CMD},
    {11, "check charge ports", &CLI_checkCharging, MENU_CMD},
#if SF_ENABLE_MFG_TEST
    {12, "MFG Test", &CLI_doMfgTest, MENU_CMD},
#endif
    {13, "upload", &CLI_doUpload, MENU_CMD},
#if SF_ENABLE_DEBUG_MENUS
    {14, "Recorder Test Menu", {.pMenu = Recorder_debug_menu}, MENU_SUBMENU},
    {15, "Session Test Menu", {.pMenu = Session_debug_menu}, MENU_SUBMENU},
    {16, "Display all sensors", &CLI_monitorSensors, MENU_CMD},
#endif
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
    {500, "BLE Stream Test", &CLI_doBleTest, MENU_CMD},
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

    //referenced dataupload
    this->initSuccess = 1;
    //RGB connection test
    if (sf::cloud::wait_connect(SF_CELL_SIGNAL_TIMEOUT_MS))
    {
        this->initSuccess = 0;
        //CLI_ledStatus.setColor(RGB_COLOR_YELLOW);
    }
    // else if (sf::cloud::wait_connect(SF_CELL_SIGNAL_TIMEOUT_MS) == 0) {
    //     CLI_ledStatus.setColor(RGB_COLOR_BLUE);
    // } 
    Particle.syncTime();

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
    if (sf::cloud::wait_disconnect(5000))
    {
        FLOG_AddError(FLOG_CELL_DISCONN_FAIL, 0);
    }
    CLI_ledStatus.setActive(false);
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

#if SF_ENABLE_DEBUG_MENUS
/**
 * @brief MonitorSensors header enum
 *
 */
enum SensorHeader
...
        for (CLI_MON_SENSOR_data_t *it = sensor_headers; it->header; it++)
        {
            if (it->active)
            {
                SF_OSAL_printf("| %12.4f ", it->value);
            }
        }
        SF_OSAL_printf("|" __NL__);
        count++;
        delay(delayTime);
    }
    pSystemDesc->pTempSensor->stop();
    pSystemDesc->pChargerCheck->start();
    pSystemDesc->pWaterCheck->start();
}
#endif // SF_ENABLE_DEBUG_MENUS

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

/**
 * @brief BLE live-stream test
 *
 * Initializes SFBLE + TransportService, runs the deployment schedule in a
 * loop until the user presses 'q', then shuts down cleanly.
 *
 * Run this from CLI while tests/ble_reciever.py is open on the host to
 * verify the full BLE data path end-to-end.
 */
static void CLI_doBleTest(void)
{
    struct BleStatus
    {
        std::atomic<bool> connected{false};
        std::atomic<bool> changed{false};
    } bleStatus;

    BleLiveStream &stream = BleLiveStream::getInstance();
    TransportService &transport = TransportService::getInstance();
    SF_OSAL_printf("=== Starting BLE Test ===" __NL__);
    SF_OSAL_printf("Initializing BLE..." __NL__);
    if (!stream.init())
    {
        SF_OSAL_printf("BLE init failed!" __NL__);
        return;
    }
    transport.init();
    transport.start();
    SFBLE::getInstance().setConnectionCallback(
        [](bool connected, void *context)
        {
            BleStatus *status = static_cast<BleStatus *>(context);
            if (status == nullptr)
            {
                return;
            }

            status->connected.store(connected, std::memory_order_release);
            status->changed.store(true, std::memory_order_release);
        },
        &bleStatus);
    SF_OSAL_printf("Advertising. Connect your device, then press 'q' to stop." __NL__ __NL__);

    pSystemDesc->pChargerCheck->stop();
    pSystemDesc->pWaterCheck->stop();
    const bool tempSensorReady = pSystemDesc->pTempSensor->init();
    SF_OSAL_printf("[BLE TEST] temp sensor init: %s" __NL__,
                   tempSensorReady ? "OK" : "FAILED");
    delay(500);

    float tempProbe = pSystemDesc->pTempSensor->getTemp();
    if (std::isfinite(tempProbe))
    {
        SF_OSAL_printf("[BLE TEST] initial temp sample: %.4f C" __NL__, tempProbe);
    }
    else
    {
        SF_OSAL_printf("[BLE TEST] initial temp sample invalid (NaN/Inf)" __NL__);
    }

#if ENABLE_RECORD_SINK
    if (!pSystemDesc->pRecorder->openSession())
    {
        SF_OSAL_printf("Warning: failed to open recorder session" __NL__);
    }
#endif

    Scheduler bleScheduler(deploymentSchedule);
    Ens_setStartTime();
    bleScheduler.initializeScheduler();

    DeploymentSchedule_t *pNextEvent = nullptr;
    uint32_t nextEventTime = 0;
    uint32_t ensCount = 0;
    uint32_t lastStatusMs = millis();
    bool quit = false;

    while (!quit)
    {
        if (bleStatus.changed.exchange(false, std::memory_order_acq_rel))
        {
            const bool connected = bleStatus.connected.load(std::memory_order_acquire);
            SF_OSAL_printf("[BLE EVT] %s" __NL__, connected ? "CONNECTED" : "DISCONNECTED");
        }

        if (SF_OSAL_kbhit())
        {
            char ch = SF_OSAL_getch();
            if (ch == 'q' || ch == 'Q')
            {
                break;
            }
            // Hardware Toggles for Power Testing
            else if (ch == 'G') { CLI_stopGPS(); }
            else if (ch == 'g') { CLI_startGPS(); }
            else if (ch == 'I') { CLI_stopIMU(); }
            else if (ch == 'i') { CLI_startIMU(); }
            else if (ch == 'C') { CLI_stopCellular(); }
            else if (ch == 'c') { CLI_startCellular(); }
            else if (ch == 'S' || ch == 's') {
                SF_OSAL_printf("[CELL STATUS] On: %d | Ready: %d | Particle: %d" __NL__, 
                               Cellular.isOn(), Cellular.ready(), Particle.connected());
            }
        }

        uint32_t now = millis();
        if (now - lastStatusMs >= 2000)
        {
            tempProbe = pSystemDesc->pTempSensor->getTemp();
            if (std::isfinite(tempProbe))
            {
                SF_OSAL_printf("[BLE STATUS] %s | ensembles: %" PRIu32 " | temp: %.4f C" __NL__,
                               stream.isConnected() ? "connected" : "advertising",
                               ensCount,
                               tempProbe);
            }
            else
            {
                SF_OSAL_printf("[BLE STATUS] %s | ensembles: %" PRIu32 " | temp: invalid" __NL__,
                               stream.isConnected() ? "connected" : "advertising",
                               ensCount);
            }
            lastStatusMs = now;
        }

        SCH_error_e ret = bleScheduler.getNextTask(&pNextEvent, &nextEventTime, millis());
        if (ret == TASK_SEARCH_FAIL)
        {
            SF_OSAL_printf("Scheduler error — aborting BLE test" __NL__);
            break;
        }

        while (millis() < nextEventTime && !quit)
        {
            Particle.process();
            if (SF_OSAL_kbhit())
            {
                char ch = SF_OSAL_getch();
                if (ch == 'q' || ch == 'Q')
                {
                    quit = true;
                }
                // Hardware Toggles for Power Testing (inner loop)
                else if (ch == 'G') { CLI_stopGPS(); }
                else if (ch == 'g') { CLI_startGPS(); }
                else if (ch == 'I') { CLI_stopIMU(); }
                else if (ch == 'i') { CLI_startIMU(); }
                else if (ch == 'C') { CLI_stopCellular(); }
                else if (ch == 'c') { CLI_startCellular(); }
                else if (ch == 'S' || ch == 's') {
                    SF_OSAL_printf("[CELL STATUS] On: %d | Ready: %d | Particle: %d" __NL__, 
                                   Cellular.isOn(), Cellular.ready(), Particle.connected());
                }
            }
            delay(1);
        }

        if (quit)
        {
            break;
        }

        pNextEvent->measure(pNextEvent);
        ensCount++;
    }

    SFBLE::getInstance().setConnectionCallback(nullptr, nullptr);
    transport.shutdown();

#if ENABLE_RECORD_SINK
    pSystemDesc->pRecorder->closeSession();
#endif
    pSystemDesc->pTempSensor->stop();
    pSystemDesc->pChargerCheck->start();
    pSystemDesc->pWaterCheck->start();

    SF_OSAL_printf("BLE stream test complete. %" PRIu32 " ensembles run." __NL__, ensCount);
}
