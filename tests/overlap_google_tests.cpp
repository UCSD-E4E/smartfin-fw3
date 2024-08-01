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

class SchedulerOverlapTests : public ::testing::Test
{
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


    //! filename for writing expected test output
    std::string expectedFileName;
    //! filename for writing actual test output
    std::string actualFileName;
    //! holds test name across functions
    std::string testName;
    std::unique_ptr<Scheduler> scheduler;


    //! for writing test output
    bool useCompareLogs;

    static std::unique_ptr<FileWriter> files;

    static void SetUpTestSuite()
    {
        files = std::make_unique<FileWriter>("tests/outputs/expected_overlap_tests.log", 
                    "tests/outputs/actual_overlap_tests.log");
    }
    static void TearDownTestSuite()
    {
        files->closeFiles();
    }

    /**
    * @brief Construct a new Scheduler Test object
    *
    * Creates a deployment schedule with three ensembles:
    * Ensemble A runs every 2000ms with a max duration of 400ms
    * Ensemble B runs every 2000ms with a max duration of 200ms
    * Ensemble C runs every 2000ms with a max duration of 600ms
    */
    SchedulerOverlapTests()
    {   
        deploymentSchedule = {
                                {   SS_ensembleAFunc, //measure
                                    SS_ensembleAInit, //init
                                    1, // measurementsToAccumulate
                                    0, // ensembleDelay 
                                    2000, //ensembleInterval
                                    400, //maxDuration
                                    UINT32_MAX, //maxDelay
                                    "A", //taskName
                                    {0} //state
                                    },
                            
        {SS_ensembleBFunc, SS_ensembleBInit, 1, 0, 2000, 200, UINT32_MAX, "B",{0}},
        { nullptr,          nullptr,          0, 0, 0,    0,   0,          "", {0}}
        };
    }
   
    
    
    
    

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
        setTime(0); // time handling see @ref tests/scheduler_test_system.cpp
        scheduler->initializeScheduler();
        useCompareLogs = false;
        

    }
    
    /**
     * @brief Cleans the test envirnoment after each test
     *
     */
    void TearDown() override
    {
        files->writeTest(testName,expected,actual);
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
            << "Expected \t Actual\n"
            << expectedTaskName << "\t\t" << nextEvent->taskName << "\n"
            << expectedStart << "\t\t" << nextEventTime + preceedingDelay 
            << "\n"
            << expectedEnd << "\t\t" << nextEventTime + nextEvent->maxDuration;

        ASSERT_EQ(expectedTaskName, nextEvent->taskName) << failMessage.str();
            
        setTime(nextEventTime);
        addTime(preceedingDelay);
        ASSERT_EQ(expectedStart, millis())  << failMessage.str();
        addTime(nextEvent->maxDuration);
        addTime(trailingDelay);
        ASSERT_EQ(expectedEnd, millis())  << failMessage.str();
        appendExpectedFile(expectedTaskName, expectedStart, expectedEnd);
        appendActualFile(nextEvent->taskName, nextEventTime, millis());
    }
    
    
};
std::unique_ptr<FileWriter> SchedulerOverlapTests::files;
/**
 * @brief Testing no overlap with A before B
 *
 * A   ***********
 * B                  **********
 */
TEST_F(SchedulerOverlapTests, NoOverlapBefore)
{
    

    deploymentSchedule[0].state.deploymentStartTime = 0;
    deploymentSchedule[0].ensembleDelay = 0;
    deploymentSchedule[0].maxDuration = 300;
    deploymentSchedule[0].ensembleInterval = 1000;

    deploymentSchedule[1].state.deploymentStartTime = 0;
    deploymentSchedule[1].ensembleDelay = 500;
    deploymentSchedule[1].maxDuration = 200;
    deploymentSchedule[1].ensembleInterval = 1000;

    int i = 1;
    uint32_t start = deploymentSchedule[i].state.deploymentStartTime +
        deploymentSchedule[i].ensembleDelay;
    uint32_t end = start + deploymentSchedule[i].maxDuration;
    bool overlap = scheduler->willOverlap(i, 0, start);
    ASSERT_FALSE(overlap);
}



/**
 * @brief Testing overlap with end of A and start of B
 *
 * A   ***********
 * B          ***********
 */
TEST_F(SchedulerOverlapTests, OverlapBefore)
{
    

    deploymentSchedule[0].state.deploymentStartTime = 0;
    deploymentSchedule[0].state.nextRunTime = 250;
    deploymentSchedule[0].maxDuration = 300;
    deploymentSchedule[0].ensembleInterval = 1000;


    deploymentSchedule[1].state.deploymentStartTime = 0;
    deploymentSchedule[1].state.nextRunTime = 500;
    deploymentSchedule[1].maxDuration = 200;
    deploymentSchedule[1].ensembleInterval = 1000;


    int i = 1;
    uint32_t start = deploymentSchedule[i].state.nextRunTime;
    uint32_t end = start + deploymentSchedule[i].maxDuration;
    bool overlap = scheduler->willOverlap(i, 0, start);
    ASSERT_TRUE(overlap);
}


/*
A                             ***********
B                 **********
*/
/**
 * @brief Testing no overlap with A after B
 *
 */
TEST_F(SchedulerOverlapTests, NoOverlapAfter)
{

    deploymentSchedule[0].state.deploymentStartTime = 0;
    deploymentSchedule[0].ensembleDelay = 800;
    deploymentSchedule[0].maxDuration = 300;
    deploymentSchedule[0].ensembleInterval = 1000;

    deploymentSchedule[1].state.deploymentStartTime = 0;
    deploymentSchedule[1].ensembleDelay = 500;
    deploymentSchedule[1].maxDuration = 200;
    deploymentSchedule[1].ensembleInterval = 1000;

    int i = 1;
    uint32_t start = deploymentSchedule[i].state.deploymentStartTime +
        deploymentSchedule[i].ensembleDelay;
    uint32_t end = start + deploymentSchedule[i].maxDuration;
    bool overlap = scheduler->willOverlap(i, 0, start);
    ASSERT_FALSE(overlap);
}

/**
 * @brief Testing overlap with end of B and start of A
 *
 * A                   ***********
 * B           **********
 */
TEST_F(SchedulerOverlapTests, OverlapAfter)
{

    deploymentSchedule[0].state.deploymentStartTime = 0;
    deploymentSchedule[0].state.nextRunTime = 650;
    deploymentSchedule[0].maxDuration = 300;
    deploymentSchedule[0].ensembleInterval = 1000;


    deploymentSchedule[1].state.deploymentStartTime = 0;
    deploymentSchedule[1].state.nextRunTime = 500;
    deploymentSchedule[1].maxDuration = 200;
    deploymentSchedule[1].ensembleInterval = 1000;


    int i = 1;
    uint32_t start = deploymentSchedule[i].state.nextRunTime;
    uint32_t end = start + deploymentSchedule[i].maxDuration;
    bool overlap = scheduler->willOverlap(i, 0, start);
    ASSERT_TRUE(overlap);
}


/**
 * @brief Testing for overlap when B starts after A starts and before A ends
 *
 * A           *****************
 * B              **********
 *
 */
TEST_F(SchedulerOverlapTests, InsideOverlap)
{

    deploymentSchedule[0].state.deploymentStartTime = 0;
    deploymentSchedule[0].state.nextRunTime = 0;
    deploymentSchedule[0].maxDuration = 800;
    deploymentSchedule[0].ensembleInterval = 1000;


    deploymentSchedule[1].state.deploymentStartTime = 0;
    deploymentSchedule[1].state.nextRunTime = 500;
    deploymentSchedule[1].maxDuration = 200;
    deploymentSchedule[1].ensembleInterval = 1000;


    int i = 1;
    uint32_t start = deploymentSchedule[i].state.nextRunTime;
    uint32_t end = start + deploymentSchedule[i].maxDuration;
    bool overlap = scheduler->willOverlap(i, 0, start);
    ASSERT_TRUE(overlap);
}


/**
 * @brief Testing no overlap edge case when B starts as A ends
 *
 * A  *********
 * B           **********
 */
TEST_F(SchedulerOverlapTests, Boundary1)
{
    deploymentSchedule[0].state.deploymentStartTime = 0;
    deploymentSchedule[0].state.nextRunTime = 0;
    deploymentSchedule[0].maxDuration = 500;
    deploymentSchedule[0].ensembleInterval = 1000;

    deploymentSchedule[1].state.deploymentStartTime = 0;
    deploymentSchedule[1].state.nextRunTime = 500;
    deploymentSchedule[1].maxDuration = 200;
    deploymentSchedule[1].ensembleInterval = 1000;


    int i = 1;
    uint32_t start = deploymentSchedule[i].state.nextRunTime;
    uint32_t end = start + deploymentSchedule[i].maxDuration;
    bool overlap = scheduler->willOverlap(i, 0, start);
    ASSERT_FALSE(overlap);
}


/**
 * @brief Testing no overlap edge case when A starts as B ends
 *
 * A                     *********
 * B           **********
 */
TEST_F(SchedulerOverlapTests, Boundary2)
{

    deploymentSchedule[0].state.deploymentStartTime = 0;
    deploymentSchedule[0].ensembleDelay = 700;
    deploymentSchedule[0].maxDuration = 300;
    deploymentSchedule[0].ensembleInterval = 1000;


    deploymentSchedule[1].state.deploymentStartTime = 0;
    deploymentSchedule[1].ensembleDelay = 500;
    deploymentSchedule[1].maxDuration = 200;
    deploymentSchedule[1].ensembleInterval = 1000;


    int i = 1;
    uint32_t start = deploymentSchedule[i].state.deploymentStartTime +
        deploymentSchedule[i].ensembleDelay;
    uint32_t end = start + deploymentSchedule[i].maxDuration;
    bool overlap = scheduler->willOverlap(i, 0, start);
    ASSERT_FALSE(overlap);
    scheduler->getNextTask(nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent("B", 500, 700);
    scheduler->getNextTask(nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent("A", 700, 1000);
}

