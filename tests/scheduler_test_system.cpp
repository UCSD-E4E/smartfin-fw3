/**
 * @file scheduler_test_system.cpp
 * @brief implements functions for testing declared in @ref scheduler_test_system.hpp
 * 
 */
#include "scheduler_test_system.hpp"
#include "conio.hpp"
#include <chrono>

#include <stdarg.h>
#include <string>
#include <cstdio>

/**
* @brief comparator for google tests EXPECT_TRUE
* 
* @param rhs other TestLog to compare to
* @return true if the same
* @return false otherwise
*/
bool TestLog::operator==(const TestLog& rhs)
{
    return  (name == rhs.name) &&
        (start == rhs.start) &&
        (end == rhs.end);
};
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
uint32_t millis()
{
    return testTime;
}
/**
 * @brief adds time to the current time
 *
 * @param add amount of time to add
 */
void addTime(uint32_t add)
{
    testTime += add;
}
/**
 * @brief sets the current time
 *
 * @param set time to set clock to
 */
void setTime(uint32_t set)
{
    testTime = set;
}

/**
 * @brief adds time to the current time
 * @note needed by Particle headers, same definition as @ref addTime(uint32_t)
 *
 * @param time amount of time to add
 */
void delay(uint32_t time)
{
    addTime(time);
}