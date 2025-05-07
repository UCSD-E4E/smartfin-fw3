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

#include "conio.hpp"
#include "cli.hpp"
#include "menu.hpp"

#include "menuItems/debugCommands.hpp"

#include "product.hpp"
#include "util.hpp"
#include "menu.hpp"

#include "system.hpp"

#include "consts.hpp"

#include <cmath>
#include <cstdlib>
#include <cstdio>


#include "Particle.h"

/**
 * @brief Command line debugging menu
 */
const Menu_t CLI_debugMenu[] = {{1, "Display Fault Log", &CLI_displayFLOG, MENU_CMD},
                                {2, "Clear Fault Log", &CLI_clearFLOG, MENU_CMD},
                                {3, "Restart System", &CLI_restart, MENU_CMD},
                                {4, "MFG Test", &CLI_doMfgTest, MENU_CMD},
                                {5, "Wet/Dry Monitor", &CLI_monitorWetDry, MENU_CMD},
                                {6, "Monitor IMU", &CLI_monitorIMU, MENU_CMD},
                                {7, "Monitor Temperature", &CLI_monitorTempSensor, MENU_CMD},
                                {8, "reinitialize system", &SYS_initSys, MENU_CMD},
                                {9, "create test file", &CLI_createTestFile, MENU_CMD},
                                {10, "test has data", &CLI_testHasData, MENU_CMD},
                                {11, "test num files", &CLI_testGetNumFiles, MENU_CMD},
                                {12, "wipe file system", &CLI_wipeFileSystem, MENU_CMD},
                                {13, "File CLI", &CLI_fileCLI, MENU_CMD},
                                {14, "Init Cloud Counters", &CLI_initCloudCounters, MENU_CMD},
                                {0, nullptr, nullptr, MENU_NULL}};
