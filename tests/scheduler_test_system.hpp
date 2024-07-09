/**
 * @file scheduler_test_system.hpp
 * @brief header for helper functions needed for testing
 */
#ifndef __SCHEDULER__TEST__HPP_
#define __SCHEDULER__TEST__HPP_
#include <stdint.h>
#include <vector>
#include <string>
#include <iostream>

typedef uint32_t system_tick_t;//!< time unit required by Particle

static int testTime = 0;//!< simulates current time
/**
 * @brief adds time to the current time
 *
 * @param add amount of time to add
 */
void addTime(uint32_t add);
/**
 * @brief sets the current time
 *
 * @param set time to set clock to
 */
void setTime(uint32_t set);


/**
 * @brief adds time to the current time
 * @note needed by Particle headers, same definition as @ref addTime(uint32_t)
 * @param time amount of time to add
 */
void delay(uint32_t time);
/**
 * @brief returns the current time in milliseconds
 * @note this is areplacement function for Particle's millis() function
 *
 * @return current time in milliseconds
 */
uint32_t millis();
/**
 * @class TestLog
 * @brief helper for @ref gtest.cpp, compares scheduler output with values
 * 
 */
struct TestLog
{
    char name; //!< name of the task
    uint32_t start; //!< expected start time
    uint32_t end; //!< expected end time
    /**
     * @brief Construct a new Test Log object
     * 
     * @param name task name
     * @param start expected start time
     * @param end expected end time
     */
    TestLog(char name, uint32_t start, uint32_t end) : name(name), 
                                    start(start), end(end){}
    /**
     * @brief comparator for google tests EXPECT_TRUE
     * 
     * @param rhs other TestLog to compare to
     * @return true if the same
     * @return false otherwise
     */
    bool operator==(const TestLog& rhs);
};

std::ostream& operator<<(std::ostream &strm, const TestLog &value);

#endif //__SCHEDULER__TEST__HPP_