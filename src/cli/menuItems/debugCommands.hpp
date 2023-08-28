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

#endif
