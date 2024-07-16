/**
 * @file gtest.cpp
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @brief Google Tests for scheduler.cpp
 */
#include <gtest/gtest.h>
#include "scheduler_test_system.hpp"
#include "test_ensembles.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <memory>
#include <dirent.h>
#include <algorithm>

class SchedulerTestsFromFiles : public ::testing::TestWithParam<std::string>
{
public:
    static void SetUpTestSuite()
    {
        files = std::make_unique<FileWriter>("expected_file_tests.log",
                "actual_file_tests.log");
        
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
    uint32_t nextEventTime;
    //! holds actual values for testing
    std::vector<TestLog> actual;
    //! holds expected values for testing
    std::vector<TestLog> expected;

    //! output file for expected values
    std::ofstream expectedFile;
    //! output file for actual values
    std::ofstream actualFile;
    
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

    

    /**
    * @brief Construct a new Scheduler Test object
    *
    * Creates a deployment schedule with three ensembles:
    * Ensemble A runs every 2000ms with a max duration of 400ms
    * Ensemble B runs every 2000ms with a max duration of 200ms
    * Ensemble C runs every 2000ms with a max duration of 600ms
    */
   
    /**
     * @brief Add expected log to vector
     * @param task the name of the task being run
     * @param start the start time
     * @param end the end tim
     */
    inline void appendExpectedFile(std::string task, 
                                    uint32_t start, 
                                    uint32_t end)
    {
        expected.emplace_back(task, start, end);
    }
    /**
     * @brief Add actual log to vector
     * @param task the name of the task being run
     * @param start the start time
     * @param end the end tim
     */
    inline void appendActualFile(std::string task, 
                                    uint32_t start, 
                                    uint32_t end)
    {
        actual.emplace_back(task, start, end);
    }
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

    }
    
    /**
     * @brief Cleans the test envirnoment after each test
     *
     */
    void TearDown() override
    {

        files->writeTest(testName,expected,actual);
    }
    void runNextEvent()
    {

        SCH_getNextEvent(deploymentSchedule.data(),
                            &nextEvent,
                            &nextEventTime,
                            millis());
        ASSERT_NE(nextEvent, nullptr) << "Scheduler returned nullptr.";
        if (nextEvent == nullptr) return;
        setTime(nextEventTime);
        addTime(nextEvent->maxDuration);
        if (!useCompareLogs)
        {
            appendExpectedFile(nextEvent->taskName, nextEventTime, millis());
            appendActualFile(nextEvent->taskName, nextEventTime, millis());
        }
    }
    
    void runTestFile(std::string filename)
    {
        input = parseInputFile(filename);
        std::cout << input.serialize() << "\n";
        setTime(input.start); 
        deploymentSchedule.clear();
        DeploymentSchedule_t e;
        for(size_t i = 0; i < input.ensembles.size(); i++)
        {
            e = { SS_ensembleAFunc, SS_ensembleAInit, 1, 0, 
                                                input.ensembles[i].interval,
                                                input.ensembles[i].duration, 
                                                input.ensembles[i].delay, 
                                                input.ensembles[i].taskName.c_str(), 
                                                {0} };
            deploymentSchedule.emplace_back(e);
        }
        e = { nullptr, nullptr, 0, 0, 0, 0, 0, "",{0} };
        deploymentSchedule.emplace_back(e);
        SCH_initializeSchedule(deploymentSchedule.data(), millis());
        
        while(millis() < input.end)
        {
            SCH_getNextEvent(deploymentSchedule.data(), 
                                    &nextEvent,&nextEventTime,
                                    millis());
            uint32_t beforeDelay = 0;
            uint32_t afterDelay = 0;
            for(size_t i = 0; i < input.delays.size(); i++)
            {
                if(!strcmp(nextEvent->taskName, 
                            input.delays[i].taskName.c_str()) &&
                        (nextEvent->state.measurementCount == 
                        input.delays[i].iteration))
                {
                    if (input.delays[i].isBefore)
                    {
                        beforeDelay = input.delays[i].delay;
                    }
                    else
                    {
                        afterDelay = input.delays[i].delay;
                    }
                    input.delays.erase(input.delays.begin() + i);
                    if ((beforeDelay != 0) && (afterDelay != 0))
                        break;
                }
            }
            ASSERT_TRUE(input.expectedValues.size() > 0) 
                << "Not enough expected values were provided!";
            if (input.expectedValues.size() > 0) 
            {
                TestLog exp = input.expectedValues.front();
                runAndCheckEventWithDelays(exp.name, exp.start, exp.end, 
                                        beforeDelay,afterDelay);
                input.expectedValues.erase(input.expectedValues.begin());
            }
            else
            {
                runNextEvent();
            }   

        }
        
    }
    TestInput parseInputFile(std::string filename)
    {
        std::ifstream inputFile(filename);
        std::ifstream file(filename);
        std::string line;
        std::string currentSection;
        int start = 0;
        int end;
        
        std::vector<TestLog> expectedValues;
        std::vector<Delay> delays;
        TestInput out;
        while (getline(file, line)) {
            
            size_t commentPos = line.find('#');
            if (commentPos != std::string::npos) {
                line = line.substr(0, commentPos);
            }

            // Remove leading and trailing whitespace
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);

            if (line.empty()) continue;  

            if (line == "START") {
                currentSection = "START";
                continue;
            } else if (line == "END") {
                currentSection = "END";
                continue;
            } else if (line == "ENSEMBLES") {
                currentSection = "ENSEMBLES";
                continue;
            } else if (line == "DELAYS") {
                currentSection = "DELAYS";
                continue; 
            } else if (line == "EXPECTED") {
                currentSection = "EXPECTED";
                continue;
            }

            std::istringstream iss(line);
            if (currentSection == "START") {
                iss >> out.start;
            } else if (currentSection == "END") {
                iss >> out.end;
            } 
            else if (currentSection == "ENSEMBLES") {
                //A|2000|200
                //B|2000|400|1000 
                std::string name;
                
                uint32_t interval, duration, maxDelay;
                std::getline(iss, name, '|');
                std::cout << "name: " << name <<"\n";
                iss >> interval;
                std::cout << "interval: " << interval <<"\n";
                
                iss.ignore(1, '|');
                iss >> duration;
                std::cout << "duration: " << duration <<"\n";
                char checkChar = iss.peek();
                if (checkChar == '|') {
                    iss.ignore(1, '|');
                    iss >> maxDelay;
                }
                else
                {
                    maxDelay = interval * 2;
                }
                EnsembleInput ensembleInput(name, interval, duration, maxDelay);
                out.ensembles.emplace_back(ensembleInput);
                
            } 
            else if (currentSection == "DELAYS") {
                Delay d;
                std::string delayName;
                std::getline(iss, delayName, '|');
                d.taskName = delayName.c_str();
                iss >> d.iteration;
                iss.ignore(1, '|');
                iss >> d.delay;
                d.isBefore = false;
                char checkChar = iss.peek();
                if (checkChar == '|') {
                    iss.ignore(1, '|');
                    std::string position;
                    iss >> position;
                    if (position == "before") d.isBefore = true;
                }
                out.delays.emplace_back(d);
            } else if (currentSection == "EXPECTED") {
                
                std::string expectedTaskName;
                uint32_t expectedStart, exepectedEnd;
                std::getline(iss, expectedTaskName, '|');
                
                iss >> expectedStart;
                iss.ignore(1, '|');
                iss >> exepectedEnd;
                TestLog exp(expectedTaskName.c_str(), expectedStart,
                                                        exepectedEnd);
                out.expectedValues.push_back(exp);
                for (const auto& ensemble : out.expectedValues) {
                    std::cout << "Expected Name: " << ensemble.name << ", start: " << ensemble.start << ", end: " << ensemble.end  << "\n";
                } 
                std::cout << "\n";
            }
        }
        
        for (const auto& ensemble : out.ensembles) {
            std::cout << "Task Name: " << ensemble.taskName << ", Interval: " << ensemble.interval << ", Duration: " << ensemble.duration << ", Delay: " << ensemble.delay << "\n";
        } 
        for (const auto& ensemble : out.expectedValues) {
            std::cout << "Expected Name: " << ensemble.name << ", start: " << ensemble.start << ", end: " << ensemble.end  << "\n";
        } 
        
        return out;
    }

    
    /**
     * @brief Run task and update time with delay and check values
     *
     * This function checks the correct task is being run, and the start time
     * and end time is correct. The delay is added after the task is run.
     *
     *
     * @param expectedTaskName expected task name character
     * @param expectedStart time to check start time with
     * @param expectedEnd time to check end time with
     * @param additionalTime the amount of delay to add
     */
    
    /**
     * @brief Run task and update time and check values
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
                                    uint32_t expectedStart,
                                    uint32_t expectedEnd)
    {
        runAndCheckEventWithDelays(expectedTaskName, expectedStart, 
                                    expectedEnd, 0, 0);
    }
    void runAndCheckEventWithDelays(std::string expectedTaskName,
                                    uint32_t expectedStart,
                                    uint32_t expectedEnd,
                                    uint32_t preceedingDelay,
                                    uint32_t trailingDelay)
    {
        ASSERT_NE(nextEvent, nullptr) << "Scheduler returned nullptr.";
        std::stringstream failMessage;
        failMessage << "runAndCheckEvent failed:\n"
            << "Expected \tActual\n"
            << expectedTaskName << "\t\t" << nextEvent->taskName << "\n"
            << expectedStart << "\t\t" << nextEventTime + preceedingDelay 
            << "\n"
            << expectedEnd << "\t\t" << nextEventTime + nextEvent->maxDuration;
        
    
        
        
        setTime(nextEventTime);
        addTime(preceedingDelay);
        uint32_t actualStart = millis();
        

        addTime(nextEvent->maxDuration);
        addTime(trailingDelay);
        uint32_t actualEnd = millis();
        
        appendExpectedFile(expectedTaskName, expectedStart, expectedEnd);
        appendActualFile(nextEvent->taskName, nextEventTime, millis());

        ASSERT_EQ(expectedTaskName, nextEvent->taskName) << failMessage.str();
        ASSERT_EQ(expectedStart, actualStart)  << failMessage.str();
        ASSERT_EQ(expectedEnd, actualEnd)  << failMessage.str();
    }
    /**
     * @brief Runs the next event and updates clock
     *
     * @param ScheduleTable_t contains ensemble table
     * @param p_nextEvent  pointer to next event
     * @param p_nextTime pointer to next time to run
     */
    

    
};
std::unique_ptr<FileWriter> SchedulerTestsFromFiles::files;


TEST_P(SchedulerTestsFromFiles, ReadsFileCorrectly) {
    std::string filename = GetParam();
    
    runTestFile(filename);
    
}

std::vector<std::string> GetFilesInDirectory(const std::string& directory) {
    std::vector<std::string> files;
    DIR* dirp = opendir(directory.c_str());
    struct dirent* dp;
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
    ::testing::ValuesIn(GetFilesInDirectory("inputs/")),
    [](const testing::TestParamInfo<SchedulerTestsFromFiles::ParamType>& info) {
        
        std::string name = info.param;
        // Remove directory part for cleaner test names
        size_t last_slash_idx = name.find_last_of("\\/");
        if (std::string::npos != last_slash_idx) {
            name.erase(0, last_slash_idx + 1);
        }
    
        std::replace(name.begin(), name.end(), '.', '_');
        return name;
    }
);