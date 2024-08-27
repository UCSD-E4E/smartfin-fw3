/**
 * @file scheduler_test_system.cpp
 * @brief implements functions for testing declared in @ref
 * scheduler_test_system.hpp
 *
 */
#include "scheduler_test_system.hpp"
#include "cli/conio.hpp"
#include "cli/flog.hpp"

#include <chrono>
#include <stdarg.h>
#include <string>
#include <cstdio>

 

int SF_OSAL_sprintf(char* buffer, size_t size, const char* fmt, ...)
{
    va_list vargs;
    va_start(vargs, fmt);
    int nBytes = vsnprintf(buffer, size, fmt, vargs);
    va_end(vargs);
    return nBytes;
}

/**
 * @brief prints to stdout
 *
 *
 * @param fmt format to print
 * @param  variables to print
 * @return number of characters if successful
 * @return ferror if error occurs
 */
int SF_OSAL_printf(const char* fmt, ...)
{
    va_list vargs;
    va_start(vargs, fmt);
    int nBytes = vprintf(fmt, vargs);
    va_end(vargs);
    return nBytes;
}
/**
 * @brief returns the current time in milliseconds
 * @note this is areplacement function for Particle's millis() function
 *
 * @return current time in milliseconds
 */
std::uint32_t millis()
{
    return testTime;
}
/**
 * @brief adds time to the current time
 *
 * @param add amount of time to add
 */
void addTime(std::uint32_t add)
{
    testTime += add;
}
/**
 * @brief sets the current time
 *
 * @param set time to set clock to
 */
void setTime(std::uint32_t set)
{
    testTime = set;
}

/**
 * @brief adds time to the current time
 * @note needed by Particle headers, same definition as @ref addTime(std::uint32_t)
 *
 * @param time amount of time to add
 */
void delay(std::uint32_t time)
{
    addTime(time);
}


