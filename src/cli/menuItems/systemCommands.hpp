#ifndef __SYSCMD_HPP__
#define __SYSCMD_HPP__


/**
 * @brief connect particle board to serial
*/
void CLI_connect(void);
/**
 * @brief disconnect particle board from serial
*/
void CLI_disconnect(void);
/**
 * @brief Display FLOG errors
*/
void CLI_displayFLOG(void);

/**
 * @brief Enter sleep mode
 * 
 */
void CLI_doSleep(void);

void CLI_doUpload(void);

#endif
