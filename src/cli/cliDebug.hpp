#ifndef __CLIDEBUG_HPP__
#define __CLIDEBUG_HPP__
#include "cli/menu.hpp"

/**
 * @brief Main debug loop
 * Manages user input, runs commands, and displays menu
 */
void CLI_doDebugMode(void);
extern const Menu_t CLI_debugMenu[];
#endif