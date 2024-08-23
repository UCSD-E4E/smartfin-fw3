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





//! time unit required by Particle
typedef uint32_t system_tick_t;


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
    std::string  name; //!< name of the task
    uint32_t start; //!< expected start time
    uint32_t end; //!< expected end time
    /**
     * @brief Construct a new Test Log object
     * 
     * @param name task name
     * @param start expected start time
     * @param end expected end time
     */
    TestLog(std::string name, uint32_t start, uint32_t end) : name(name), 
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


struct EnsembleInput
{
    EnsembleInput(std::string taskName,uint32_t interval, uint32_t duration, 
                                                                uint32_t delay);
    std::string taskName; //!< name of the task
    uint32_t interval;
    uint32_t duration;
    uint32_t delay;
};
struct TestInput
{
    uint32_t start;
    uint32_t end;
    std::vector<EnsembleInput> ensembles;
    std::vector<TestLog> expectedValues;
    std::unordered_map<std::string, 
                        std::unordered_map<uint32_t,uint32_t>> delays;
    std::vector<std::pair<std::string,uint32_t>> resets;

    uint32_t getDelay(std::string name, uint32_t iteration);

    void clear();
    std::string serialize() const {
        std::stringstream ss;
        ss << "TestInput: { Start: " << start << ", End: " << end << "\n";

        ss << "  Ensembles: [\n";
        for (const auto& ensemble : ensembles) {
            ss << "    { TaskName: " << ensemble.taskName
               << ", Interval: " << ensemble.interval
               << ", Duration: " << ensemble.duration
               << ", Delay: " << ensemble.delay << " }\n";
        }
        ss << "  ]\n";

        ss << "  ExpectedValues: [\n";
        for (const auto& exp : expectedValues) {
            ss << "    { Name: " << exp.name
               << ", Start: " << exp.start
               << ", End: " << exp.end << " }\n";
        }
        ss << "  ]\n";

        

        ss << "}";
        return ss.str();
    }
    

};
std::ostream& operator<<(std::ostream &strm, const TestLog &value);

struct FileWriter
{
    std::string expectedFileName;
    std::string actualFileName;
    bool firstTest;
    FileWriter(std::string expectedFileName, std::string actualFileName);
    void writeTest(std::string testName, 
            std::vector<TestLog> expected, std::vector<TestLog> actual);
    void closeFiles();
};
class Log{
    std::string taskName;
    uint32_t runTime;
public:

    Log(DeploymentSchedule_ **task, uint32_t time);
    Log(std::string name, uint32_t time);

   friend bool operator==(const Log& a, const Log& b);

   std::string getName();
   uint32_t getRunTime();

};
#endif //__SCHEDULER__TEST__HPP_