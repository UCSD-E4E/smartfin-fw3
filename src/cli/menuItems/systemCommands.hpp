#ifndef __SYSCMD_HPP__
#define __SYSCMD_HPP__

/**
 * @brief Display list of commands
 * 
 */
void CMD_displayMenu(void);
/**
 * @brief Connect the particle board to serial
 * 
 */
void CLI_connect(void);
/**
 * @brief Disconnect particle board from serial
 * 
 */
void CLI_disconnect(void);

#endif
