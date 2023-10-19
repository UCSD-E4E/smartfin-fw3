#ifndef __DEBUGCMD_HPP__
#define __DEBUGCMD_HPP__

/**
 * @brief Restarts the system
 */
void CLI_restart(void);
/**
 * @brief Displays FLOG errors
 * 
 */
void CLI_displayFLOG(void);
/**
 * @brief Clears FLOG errors
 * 
 */
void CLI_clearFLOG(void);
/**
 * @brief Tests printF functionality
 * 
 */
void CLI_testPrintf(void);
/**
 * @brief Creates test file in filesystem
*/
void CLI_createTestFile(void);
/**
 * @brief Get number of files
*/
void CLI_testGetNumFiles(void);
/**
 * @brief tests if data exists
*/
void CLI_testHasData(void);
/**
 * @brief Delete all files
*/
void CLI_wipeFileSystem(void);
/*
 * @brief Monitors temperature sensor
 * 
 */
void CLI_monitorTempSensor(void);
/**
 * @brief Monitor IMU (gyro and cellerular )
*/
void CLI_monitorIMU(void);
/**
 * @brief check charge ports
*/
void CLI_checkCharging(void);
/**
 * @brief monitor wet/dry sensor
*/
void CLI_monitorWetDry(void);
/**
 * @brief run manufacturing test
*/
void CLI_doMfgTest(void);

/**
 * @brief File CLI
 * 
 */
void CLI_fileCLI(void);
#endif
