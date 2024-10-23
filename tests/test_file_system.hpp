#ifndef __TEST_FILE_SYSTEM__
#define __TEST_FILE_SYSTEM__

#include "scheduler.hpp"

#include <string>
#include <vector>
#include <stdint.h>
#include <unordered_map>


class EnsembleInput
{
public:
    EnsembleInput(std::string taskName,std::uint32_t interval, 
    std::uint32_t duration, std::uint32_t delay);
    std::string taskName; //!< name of the task
    std::uint32_t interval;
    std::uint32_t duration;
    std::uint32_t delay;
};


/**
 * @class TestLog
 * @brief helper for @ref gtest.cpp, compares scheduler output with values
 * 
 */
struct TestLog
{
    std::string  name; //!< name of the task
    std::uint32_t start; //!< expected start time
    std::uint32_t end; //!< expected end time
    /**
     * @brief Construct a new Test Log object
     * 
     * @param name task name
     * @param start expected start time
     * @param end expected end time
     */
    TestLog(std::string name, std::uint32_t start, std::uint32_t end) : name(name), 
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
    const char* taskName;
    std::uint32_t runTime;
public:

    Log(DeploymentSchedule_ *task, std::uint32_t time);
    Log(const char* name, std::uint32_t time);

   friend bool operator==(const Log& a, const Log& b);

   const char* getName(void);
   std::uint32_t getRunTime(void);

};
class TestInput
{
public:
    std::uint32_t start;
    std::uint32_t end;
    std::vector<EnsembleInput> ensembles;
    std::vector<TestLog> expectedValues;
    std::unordered_map<std::string, 
                        std::unordered_map<std::uint32_t,std::uint32_t>> delays;
    std::vector<std::pair<std::string,std::uint32_t>> resets;

    std::uint32_t getDelay(std::string name, std::uint32_t iteration);

    void parseInputFile(std::string filename);

    void clear();
};

std::ostream& operator<<(std::ostream &strm, const TestLog &value);




#endif //__TEST_FILE_SYSTEM__