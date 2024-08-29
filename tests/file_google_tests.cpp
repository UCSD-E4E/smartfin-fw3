/**
 * @file gtest.cpp
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @brief Google Tests for scheduler.cpp
 */

#include "scheduler_test_system.hpp"
#include "test_ensembles.hpp"
#include "scheduler.hpp"
#include "test_file_system.hpp"

#include <gtest/gtest.h>

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <memory>
#include <dirent.h>
#include <algorithm>
#include <utility>



class SchedulerTestsFromFiles : public ::testing::TestWithParam<std::string>
{
public:
    static void SetUpTestSuite()
    {
        files = std::make_unique<FileWriter>(
            "tests/outputs/expected_file_tests.log",
            "tests/outputs/actual_file_tests.log");
        
    }
    static void TearDownTestSuite()
    {
        files->closeFiles();
    }
protected:
    //! holds the deployment schedule created in @ref SchedulerTest()
    std::vector<DeploymentSchedule_t> deploymentSchedule;
    //! pointer for next event in deploymentSchedule
    DeploymentSchedule_t* nextEvent;
    //! next time any event will be run
    std::uint32_t nextEventTime;
    //! holds actual values for testing
    std::vector<TestLog> actual;
    //! holds expected values for testing
    std::vector<TestLog> expected;

    //! output file for expected values
    std::ofstream expectedFile;
    //! output file for actual values
    std::ofstream actualFile;
    
    //! holds all data from a file
    TestInput input;


    //! filename for writing expected test output
    std::string expectedFileName = "expected_file_tests.log";
    //! filename for writing actual test output
    std::string actualFileName = "actual_file_tests.log";
    //! holds test name across functions
    std::string testName;


    //! for writing test output
    bool useCompareLogs;
    static std::unique_ptr<FileWriter> files;

    std::unique_ptr<Scheduler> scheduler;
    

    
   
   
    
    /**
     * @brief Sets up the test envirnoment before each test
     *
     */
    void SetUp() override
    {
        actual.clear();
        expected.clear();

        const ::testing::TestInfo* const test_info =
            ::testing::UnitTest::GetInstance()->current_test_info();
        
        testName = std::string(test_info->name());
        
        nextEvent = nullptr; // ensures that first call to scheduler is correct
        nextEventTime = 0; // time handling
        
        setTime(0);

    }
    
    /**
     * @brief Cleans the test envirnoment after each test
     *
     */
    void TearDown() override
    {
        
        size_t pos = testName.find_last_of("/");

        files->writeTest(testName.substr(pos+1),expected,actual);
    }
    void runNextEvent()
    {

        scheduler->getNextTask(&nextEvent,
                            &nextEventTime,
                            millis());
        ASSERT_NE(nextEvent, nullptr) << "Scheduler returned nullptr.";
        if (nextEvent == nullptr) return;
        setTime(nextEventTime);
        addTime(nextEvent->maxDuration);
        if (!useCompareLogs)
        {
            expected.emplace_back(nextEvent->taskName, nextEventTime, millis());
            actual.emplace_back(nextEvent->taskName, nextEventTime, millis());
        }
    }
    
    void runTestFile(std::string filename)
    {
        input.clear();
        input.parseInputFile(filename);

        setTime(input.start);
        deploymentSchedule.clear();
        DeploymentSchedule_t e;
        for (size_t i = 0; i < input.ensembles.size(); i++)
        {
            e = { SS_ensembleAFunc, SS_ensembleAInit, 1,
                                        input.ensembles[i].interval,
                                        input.ensembles[i].duration,
                                        input.ensembles[i].delay,
                                        input.ensembles[i].taskName.c_str(),
                                        {0} };
            deploymentSchedule.emplace_back(e);
        }
        e = { nullptr, nullptr, 0, 0, 0, 0, "", {0}};
        deploymentSchedule.emplace_back(e);
        scheduler = std::make_unique<Scheduler>(deploymentSchedule.data());
        scheduler->initializeScheduler();

        while (millis() < input.end)
        {
            scheduler->getNextTask(&nextEvent, &nextEventTime,
                                    millis());
            std::uint32_t afterDelay = input.getDelay(nextEvent->taskName,
                                        nextEvent->state.measurementCount - 1);
            
            
            
            if(input.expectedValues.size() > 0) 
            {
                TestLog exp = input.expectedValues.front();
                runAndCheckEventWithDelays(exp.name, exp.start, exp.end, 
                                                            afterDelay);
                input.expectedValues.erase(input.expectedValues.begin());
                
            }
            else
            {
                return;
            }
        }
    }
    
    /**
     * @brief Run task, update time, and check values
     *
     * This function checks the correct task is being run, and the start time
     * and end time is correct
     *
     *
     * @param expectedTaskName expected task name character
     * @param expectedStart time to check start time with
     * @param expectedEnd time to check end time with
     */
    inline void runAndCheckEvent(std::string expectedTaskName,
                                    std::uint32_t expectedStart,
                                    std::uint32_t expectedEnd)
    {
        runAndCheckEventWithDelays(expectedTaskName, expectedStart, 
                                    expectedEnd, 0);
    }
    void runAndCheckEventWithDelays(std::string expectedTaskName,
                                    std::uint32_t expectedStart,
                                    std::uint32_t expectedEnd,
                                    
                                    std::uint32_t trailingDelay)
    {
        ASSERT_NE(nextEvent, nullptr) << "Scheduler returned nullptr.";
        std::stringstream failMessage;
        failMessage << "runAndCheckEvent failed:\n"
            << "Expected \tActual\n"
            << expectedTaskName << "\t\t" << nextEvent->taskName << "\n"
            << expectedStart << "\t\t" << nextEventTime 
            << "\n"
            << expectedEnd << "\t\t" << nextEventTime + nextEvent->maxDuration
                                        + trailingDelay;
        
    
        
        
        setTime(nextEventTime);
        
        std::uint32_t actualStart = millis();
        

        addTime(nextEvent->maxDuration);
        addTime(trailingDelay);
        std::uint32_t actualEnd = millis();
        
        expected.emplace_back(expectedTaskName, expectedStart, expectedEnd);
        actual.emplace_back(nextEvent->taskName, actualStart, actualEnd);

        ASSERT_EQ(expectedTaskName, nextEvent->taskName) << failMessage.str();
        ASSERT_EQ(expectedStart, actualStart)  << failMessage.str();
        ASSERT_EQ(expectedEnd, actualEnd)  << failMessage.str();
    }
    
    

    
};
std::unique_ptr<FileWriter> SchedulerTestsFromFiles::files;


TEST_P(SchedulerTestsFromFiles, RunTestFile) {
    std::string filename = GetParam();
    
    runTestFile(filename);
    
}

std::vector<std::string> GetFilesInDirectory(const std::string& directory) {
    std::vector<std::string> files;
    DIR* dirp = opendir(directory.c_str());
    struct dirent* dp;
    std::cout << "Directory: " << directory.c_str() << "\n";
    while ((dp = readdir(dirp)) != nullptr) {
        if (dp->d_type == DT_REG) {  
            files.push_back(directory + "/" + std::string(dp->d_name));
        }
    }
    closedir(dirp);
    return files;
}

INSTANTIATE_TEST_SUITE_P(
    Files,
    SchedulerTestsFromFiles,
    ::testing::ValuesIn(GetFilesInDirectory("tests/inputs/")),
    [](const testing::TestParamInfo<SchedulerTestsFromFiles::ParamType>& info) {
        
        std::string name = info.param;
        // Remove directory part for cleaner test names
        size_t last_slash_idx = name.find_last_of("\\/");
        if (std::string::npos != last_slash_idx) 
        {
            name.erase(0, last_slash_idx + 1);
        }
        std::replace(name.begin(), name.end(), '.', '_');
        return name;
    }
);