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
 * @brief Display device ID and firmware version
 * 
 */
void CLI_self_identify(void);


/**
 * @brief Enter sleep mode
 * 
 */
void CLI_doSleep(void);
/**
 * @brief Get dummy data and attempt upload
 * 
*/
void CLI_doUpload(void);

#endif
