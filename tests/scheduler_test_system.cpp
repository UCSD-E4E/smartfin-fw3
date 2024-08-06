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
std::ostream& operator<<(std::ostream &strm, const TestLog &value) {
    return strm << "\"" << value.name << "|"
                << value.start << "|" << value.end << "\"";
}

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

FileWriter::FileWriter(std::string expectedFileName, std::string actualFileName)
{
    this->firstTest = true;
    this->expectedFileName = expectedFileName;
    this->actualFileName = actualFileName;
    std::ofstream expectedFile(expectedFileName,std::ios::out | 
    std::ios::trunc);

    std::ofstream actualFile(actualFileName,std::ios::out |
        std::ios::trunc);
    
    if (actualFile.is_open() && expectedFile.is_open())
    {
        actualFile << "{";
        expectedFile << "{";
    }
    actualFile.close();
    expectedFile.close();
}
void FileWriter::writeTest(std::string testName, 
    std::vector<TestLog> expected,std::vector<TestLog> actual)
{
    std::ofstream actualFile(actualFileName, std::ios::out | 
                                std::ios::app);
    std::ofstream expectedFile(expectedFileName, std::ios::out |
        std::ios::app);
    
    if (firstTest == false)
    {
        expectedFile << ",";
        actualFile << ",";
    }
    else
    {
        firstTest = false;
    }
    expectedFile << "\n\t\"" << testName << "\": [";
    if (!expected.empty())
    {


        for (int i = 0; i < expected.size() - 1; i++)
        {
            expectedFile << expected[i] << ", ";
        }
        expectedFile << expected.back();
    }
    expectedFile << "]";

    expectedFile.close();


    actualFile << "\n\t\"" << testName << "\": [";
    if (!actual.empty())
    {


        for (int i = 0; i < actual.size() - 1; i++)
        {
            actualFile << actual[i] << ", ";
        }
        actualFile << actual.back();
    }
    actualFile << "]";
    actualFile.close();
}
void FileWriter::closeFiles()
{
    std::ofstream actualFile(actualFileName, std::ios::out | 
                                std::ios::app);
    std::ofstream expectedFile(expectedFileName, std::ios::out |
        std::ios::app);

    if (actualFile.is_open() && expectedFile.is_open())
    {
        actualFile << "\n}";
        expectedFile << "\n}";
    }

    actualFile.close();
    expectedFile.close();
}
EnsembleInput::EnsembleInput(std::string taskName,uint32_t interval, uint32_t duration, uint32_t delay) : taskName(taskName), interval(interval), duration(duration), delay(delay){};

void TestInput::clear()
{
    ensembles.clear();
    expectedValues.clear();
    delays.clear();
    resets.clear();
    
}