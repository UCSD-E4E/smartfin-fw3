#ifndef __DEBUG_RECORDER_H__
#define __DEBUG_RECORDER_H__
#include "cli/menu.hpp"

/**
 * @brief Contains tests for various recorder functions
 * @details Functions able to be tested:
 * - if recorder has data
 * - if we can retrieve the number of files in filesystem\n
 * - if we can retrieve the last packet in memory\n
 * - if we can open & close a recording session\n
 * - if we can input bytes into data buffer\n
 * - if we can create session & store in session of byte size inputted by user\n
 * - if we can set start time of a session\n
 * - if we can pop the last packet in memory\n
  */
extern const Menu_t Recorder_debug_menu[];
#endif