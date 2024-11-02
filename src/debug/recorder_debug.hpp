#ifndef __DEBUG_RECORDER_H__
#define __DEBUG_RECORDER_H__
#include "cli/menu.hpp"

/**
 * @brief Contains tests for various recorder functions
 * @details Functions able to be tested:
 * - if recorder has data
 * - if we can retrieve the number of files in filesystem
 * - if we can retrieve the last packet in memory
 * - if we can open & close a recording session
 * - if we can input bytes into data buffer
 * - if we can create session & store in session of byte size inputted by user
 * - if we can set start time of a session
 * - if we can pop the last packet in memory
 */
extern const Menu_t Recorder_debug_menu[];
#endif