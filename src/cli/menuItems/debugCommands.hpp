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

void CLI_createTestFile(void);

void CLI_testHasData(void);
#endif
