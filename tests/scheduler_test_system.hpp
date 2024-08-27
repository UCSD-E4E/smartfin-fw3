/**
 * @file scheduler_test_system.hpp
 * @brief header for helper functions needed for testing
 */
#ifndef __SCHEDULER__TEST__HPP_
#define __SCHEDULER__TEST__HPP_

#include "scheduler.hpp"

#include <stdint.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <utility>
#include <unordered_map>


#ifndef TEST_VERSION
#define TEST_VERSION
#endif


//! time unit required by Particle
typedef std::uint32_t system_tick_t;
void write_shift(std::string ensemble, std::uint32_t idx);


/**
 * @brief prints to stdout
 * 
 * @param buffer destination for formatted string
 * @param fmt format to print
 * @param  variables to print
 * @return number of characters if successful
 * @return ferror if error occurs
 */
int SF_OSAL_sprintf(char* buffer, size_t size, const char* fmt, ...);

static int testTime = 0;//! simulates current time
/**
 * @brief adds time to the current time
 *
 * @param add amount of time to add
 */
void addTime(std::uint32_t add);
/**
 * @brief sets the current time
 *
 * @param set time to set clock to
 */
void setTime(std::uint32_t set);


/**
 * @brief adds time to the current time
 * @note needed by Particle headers, same definition as @ref addTime(std::uint32_t)
 * @param time amount of time to add
 */
void delay(std::uint32_t time);
/**
 * @brief returns the current time in milliseconds
 * @note this is areplacement function for Particle's millis() function
 *
 * @return current time in milliseconds
 */
std::uint32_t millis();








#endif //__SCHEDULER__TEST__HPP_