/**
 * @file cliDebug.cpp
 * @author Emily Thorpe (ethorpe@macalster.edu)
 * @brief 
 * @version 0.1
 * @date 2023-07-26
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "cliDebug.hpp"

#include "Particle.h"
#include "cli.hpp"
#include "conio.hpp"
#include "consts.hpp"
#include "menu.hpp"
#include "menuItems/debugCommands.hpp"
#include "product.hpp"
#include "system.hpp"
#include "util.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>

/**
 * @brief Command line debugging menu
 */
const Menu_t CLI_debugMenu[] = {{1, "Display Fault Log", &CLI_displayFLOG, MENU_CMD},
                                {2, "Clear Fault Log", &CLI_clearFLOG, MENU_CMD},
                                {3, "Restart System", &CLI_restart, MENU_CMD},
#if SF_ENABLE_MFG_TEST
                                {4, "MFG Test", &CLI_doMfgTest, MENU_CMD},
#endif
#if SF_ENABLE_DEBUG_MENUS
                                {5, "Wet/Dry Monitor", &CLI_monitorWetDry, MENU_CMD},
                                {7, "Monitor Temperature", &CLI_monitorTempSensor, MENU_CMD},
#endif
                                {8, "reinitialize system", &SYS_initSys, MENU_CMD},
#if SF_ENABLE_DEBUG_MENUS
                                {9, "create test file", &CLI_createTestFile, MENU_CMD},
                                {10, "test has data", &CLI_testHasData, MENU_CMD},
                                {11, "test num files", &CLI_testGetNumFiles, MENU_CMD},
                                {12, "wipe file system", &CLI_wipeFileSystem, MENU_CMD},
                                {13, "File CLI", &CLI_fileCLI, MENU_CMD},
                                {14, "Init Cloud Counters", &CLI_initCloudCounters, MENU_CMD},
                                {15, "Dump IMU Registers", &CLI_dumpIMURegs, MENU_CMD},
#endif
                                {16, "Stop GPS", &CLI_stopGPS, MENU_CMD},
                                {17, "Start GPS", &CLI_startGPS, MENU_CMD},
                                {18, "Stop IMU", &CLI_stopIMU, MENU_CMD},
                                {19, "Start IMU", &CLI_startIMU, MENU_CMD},
                                {20, "Stop Cellular", &CLI_stopCellular, MENU_CMD},
                                {21, "Start Cellular", &CLI_startCellular, MENU_CMD},
                                {22, "Stop BLE", &CLI_stopBLE, MENU_CMD},
                                {23, "Start BLE", &CLI_startBLE, MENU_CMD},
                                {0, nullptr, nullptr, MENU_NULL}};
