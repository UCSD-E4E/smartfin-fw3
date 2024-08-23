/**
 * @file gtest.cpp
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @brief Google Tests for scheduler.cpp
 */

#include "scheduler_test_system.hpp"
#include "scheduler.hpp"
#include "test_ensembles.hpp"
#include "cli/flog.hpp"

#include <gtest/gtest.h>

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <memory>



class SchedulerFixedTests : public ::testing::Test
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
        files = std::make_unique<FileWriter>("tests/outputs/expected_fixed_tests.log", 
                    "tests/outputs/actual_fixed_tests.log");
        FLOG_Initialize();
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
    SchedulerFixedTests()
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
                            
            {SS_ensembleBFunc, SS_ensembleBInit, 1, 0, 2000, 200, UINT32_MAX, "B", {0}},
            {SS_ensembleCFunc, SS_ensembleCInit, 1, 0, 2000, 600, UINT32_MAX, "C", {0}},
            { nullptr,          nullptr,          0, 0, 0,    0,   0,          "", {0}}
        };
        
        
        scheduler = std::make_unique<Scheduler>(deploymentSchedule.data());
        scheduler->initializeScheduler();
    }
    /**
    * @brief Modifies the constructed scheduler
    *  Creates a deployment schedule with one ensemble:
    *  Ensemble A runs every 2000ms with a max duration of 400ms
    */
    void schedule2()
    {
        deploymentSchedule[2] = { nullptr, nullptr, 0, 0, 0, 0, 0, "\0",{0} };
        scheduler = std::make_unique<Scheduler>(deploymentSchedule.data());
        scheduler->initializeScheduler();

        
    }
    /**
    * @brief Modifies the constructed scheduler
    * Creates a deployment schedule with three ensembles:
    * Ensemble A runs every 500ms with a max duration of 200ms
    * Ensemble B runs every 200ms with a max duration of 100ms
    * Ensemble C runs every 800ms with a max duration of 250ms
    */
    void schedule3()
    {
        deploymentSchedule[0] = { SS_ensembleAFunc, SS_ensembleAInit, 1, 0, 500, 200, UINT32_MAX, "A", {0} };
        deploymentSchedule[1] = { SS_ensembleBFunc, SS_ensembleBInit, 1, 0, 200, 100, UINT32_MAX, "B", {0} };
        deploymentSchedule[2] = { SS_ensembleCFunc, SS_ensembleCInit, 1, 0, 800, 250, UINT32_MAX, "C", {0} };
        scheduler = std::make_unique<Scheduler>(deploymentSchedule.data());
        scheduler->initializeScheduler();
    }
    /**
    * @brief Modifies the constructed scheduler
    * Creates a deployment schedule with three ensembles:
    * Ensemble A runs every 200ms with a max duration of 150ms
    */
    void schedule4()
    {
        deploymentSchedule[0] = { SS_ensembleAFunc, SS_ensembleAInit, 1, 0, 200, 150, UINT32_MAX, "A", {0} };
        deploymentSchedule[1] = { nullptr,          nullptr,          0, 0, 0,    0,   0,        "", {0} };
        scheduler = std::make_unique<Scheduler>(deploymentSchedule.data());
        scheduler->initializeScheduler(); 
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
    /**
     * @brief Runs the next event and updates clock
     *
     * @param ScheduleTable_t contains ensemble table
     * @param p_nextEvent  pointer to next event
     * @param p_nextTime pointer to next time to run
     */
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
            appendExpectedFile(nextEvent->taskName, nextEventTime, millis());
            appendActualFile(nextEvent->taskName, nextEventTime, millis());
        }
    }
    void singleEventStart()
    {
        schedule4();
        // ideal behaivor
        scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
        runAndCheckEvent("A", 0, 150);
        scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
        runAndCheckEvent("A", 200, 350);
        scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
        runAndCheckEvent("A", 400, 550);
    }
    void singleEventEndCheck()
    {
        schedule4();
        // ideal behaivor
        scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
        runAndCheckEvent("A", 0, 150);
        scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
        runAndCheckEvent("A", 200, 350);
        scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
        runAndCheckEvent("A", 400, 550);
    }
    void checkIdeal(uint32_t end)
    {
        useCompareLogs = true;
        if (end > 10000)
        {
            end = 10000;
        }
        std::vector<TestLog> expectedIdeal;
        expectedIdeal.emplace_back("A", 0, 400);
        expectedIdeal.emplace_back("B", 400, 600);
        expectedIdeal.emplace_back("C", 600, 1200);

        expectedIdeal.emplace_back("A", 2000, 2400);
        expectedIdeal.emplace_back("B", 2400, 2600);
        expectedIdeal.emplace_back("C", 2600, 3200);

        expectedIdeal.emplace_back("A", 4000, 4400);
        expectedIdeal.emplace_back("B", 4400, 4600);
        expectedIdeal.emplace_back("C", 4600, 5200);

        expectedIdeal.emplace_back("A", 6000, 6400);
        expectedIdeal.emplace_back("B", 6400, 6600);
        expectedIdeal.emplace_back("C", 6600, 7200);

        expectedIdeal.emplace_back("A", 8000, 8400);
        expectedIdeal.emplace_back("B", 8400, 8600);
        expectedIdeal.emplace_back("C", 8600, 9200);

        while (millis() > expectedIdeal.front().start)
        {
            expectedIdeal.erase(expectedIdeal.begin());
        }
        for (int i = 0; i < expectedIdeal.size(); i++)
        {
            expected.emplace_back(expectedIdeal[i]);
        }
        while (millis() < end)
        {
            runNextEvent();
            if (nextEvent == nullptr) return;
            actual.emplace_back(nextEvent->taskName, nextEventTime, millis());
        }
        compareLogs();
    }
    void compareLogs()
    {
        for (int i = 0; i < (expected.size() > actual.size() ?
            actual.size() : expected.size()); i++)
        {

            EXPECT_TRUE(expected[i] == actual[i])
                << "test log failed:\n"
                << "Expected \t Actual\n"
                << expected[i].name << "\t\t" << actual[i].name << "\n"
                << expected[i].start << "\t\t" << actual[i].start << "\n"
                << expected[i].end << "\t\t" << actual[i].end;
        }
    }
    
    
    
};
std::unique_ptr<FileWriter> SchedulerFixedTests::files;

/**
 * @brief Tests initialization of deployment schedule
 *
 * Tests if all deployment schedules are properly initialized with
 * non-null init functions and the system time is correctly reset to zero.
 */
TEST_F(SchedulerFixedTests, TestInitialization)
{
    ASSERT_NE(nullptr, deploymentSchedule[0].init);
    ASSERT_NE(nullptr, deploymentSchedule[1].init);
    ASSERT_NE(nullptr, deploymentSchedule[2].init);
    ASSERT_EQ(millis(), 0);
}


/**
 * @brief Checks ideal behaivor for single task
 */
TEST_F(SchedulerFixedTests, SingleEventIdeal)
{
    schedule4();
    // ideal behaivor
    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    
    runAndCheckEvent("A", 0, 150);
    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent("A", 200, 350);
    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent("A", 400, 550);
}
/**
 * @brief Boundary delay case for single event schedule
 *
 * task end delayed to when second event is supposed to start, so schedule
 * unchanged
 */
TEST_F(SchedulerFixedTests, SingleEvent_DelayToSecondStart)
{
    singleEventStart();
    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    runAndCheckEventWithDelays("A", 600, 800, 0, 50);
    //  last task ended at 800ms, should not impact next task
    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent("A", 800, 950);
    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent("A", 1000, 1150);

}
/**
* @brief Seccond iteration starts late with enough time to run

* task end delayed past when second event is supposed to start, but leaves
* enough time to run before third task
*/
TEST_F(SchedulerFixedTests, SingleEvent_SecondDelayedNoImpact)
{
    singleEventStart();
    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    runAndCheckEventWithDelays("A", 600, 810, 0, 60);
    //  last task ended at 810 ms, this task should be run immediately
    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent("A", 810, 960);
    //  task not delayed, should run as normal
    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent("A", 1000, 1150);
}
/**
* @brief Seccond iteration starts late with enough time to run

* task end delayed past when second event start, but leaves
* enough time to run and end when third task starts
*/
TEST_F(SchedulerFixedTests, SingleEvent_SecondEndAtThirdStart)
{
    singleEventStart();
    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    runAndCheckEventWithDelays("A", 600, 850, 0, 100);
    //  last task ended at 2050 ms, this task should be run immediately
    //  as it will now end when the next task is supposed to start
    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent("A", 850, 1000);
    //  task not delayed, should run as normal
    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent("A", 1000, 1150);


}
/**
 * @brief Seccond iteration starts late without enough time to run
 *
* task end delayed past when second event is supposed to start, second
* event would now overlap with third run time, so second event is skipped
*/
TEST_F(SchedulerFixedTests, SingleEvent_SecondEndAfterThirdStart)
{

    singleEventStart();
    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    runAndCheckEventWithDelays("A", 600, 860, 0, 110);
    //  second event skipped running third task
    //  task not delayed, should run as normal
    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent("A", 1000, 1150);


}

/**
* @brief Seccond iteration ends when third iteration was supposed to end time
* task end delayed past when second event is supposed to start, second
* event would now start when third run would start,
* so second event is skipped
*/
TEST_F(SchedulerFixedTests, SingleEvent_ExactOverlap)
{
    singleEventStart();
   
    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    runAndCheckEventWithDelays("A", 600, 1000, 0, 250);
    //  second event skipped running third task
    //  task not delayed, should run as normal
    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent("A", 1000, 1150);
    //  check only two runs
    


}
/**
 * @brief Seccond iteration ends after third iteration start time
 *
* task end delayed past ends 50 ms after third task should run
* second task skipped, third task still runs
*/
TEST_F(SchedulerFixedTests, SingleEvent_DelayPastThirdStart)
{

    singleEventStart();
    
    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    runAndCheckEventWithDelays("A", 600, 1050, 0, 300);
    //  second event skipped running third task
    //  task not delayed, should run as normal
    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent("A", 1050, 1200);
    //  check only two runs
    


}
/**
* task end delayed past ends 60 ms after third task should run
* second task skipped, third task will be skipped as well
*/
TEST_F(SchedulerFixedTests, SingleEvent_SkipSecondAndThird)
{

    singleEventStart();
   
    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    runAndCheckEventWithDelays("A", 600, 1060, 0, 310);
    //  second event skipped running third task
    //  task not delayed, should run as normal
    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent("A", 1200, 1350);
    //  check only two runs
    
}


/**
 * @brief Checks that events with no delays are fetched and executed properly.
 *
 * This test  runs through expected behaivor with no delays and enough
 * time for all the functions to run by themselves in order.
 *
 */
TEST_F(SchedulerFixedTests, TestIdeal)
{
    checkIdeal(8000);
}



/**
 * @brief Task C runs 900ms long, delaying second run of task A. Task A would
 * now delay Task B so A is run during idle time after task C is run again.
 *
 */
TEST_F(SchedulerFixedTests, DelayRunDuringIdle)
{

    runNextEvent();

    runNextEvent();

    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    runAndCheckEventWithDelays("C", 600, 2100, 0, 900);



    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent("A", 2100, 2500);
    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent("B", 2500, 2700);
    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent("C", 2700, 3300);



    checkIdeal(10000);
}

/**
 * @brief Task B runs 900ms longer. Because task C has not been run yet,
 *  it is run after the second run of B.
 *
 */
TEST_F(SchedulerFixedTests, DelayAndSkip)
{

    runNextEvent();
    runNextEvent();
    runNextEvent();

    runNextEvent();
    runNextEvent();
    addTime(900); //delay
    scheduler->getNextTask(&nextEvent,
                                &nextEventTime, millis());
    runAndCheckEvent("A", 4000, 4400);
    scheduler->getNextTask(&nextEvent,
                                &nextEventTime, millis());
    runAndCheckEvent("B", 4400, 4600);


    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());//C runs
    runAndCheckEvent("C", 4600, 5200);
    scheduler->getNextTask(&nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent("A", 6000, 6400);

    //  check for cascades
    checkIdeal(10000);
    FLOG_DisplayLog();


}


/**
 * @brief Ensembles A, B, and C have intervals that dont allow all of
 * them to be run every interval. C is supposed to run every 250 seconds but
 * tasks A and B do not leave any idle time for it to run.
 */
TEST_F(SchedulerFixedTests, ScheduleTooTight)
{
    schedule3();
    useCompareLogs = true;

    while (millis() < 1500)
    {
        runNextEvent();
        ASSERT_NE(nextEvent, nullptr) << "Scheduler returned nullptr.";
        if (nextEvent == nullptr) return;
        actual.emplace_back(nextEvent->taskName, nextEventTime, millis());
    }
    expected.emplace_back("A", 0, 200);
    expected.emplace_back("B", 200, 300);
    expected.emplace_back("B", 400, 500);
    expected.emplace_back("A", 500, 700);
    expected.emplace_back("B", 700, 800);
    expected.emplace_back("B", 800, 900);
    expected.emplace_back("A", 1000, 1200);
    expected.emplace_back("B", 1200, 1300);
    expected.emplace_back("B", 1400, 1500);
    compareLogs();

}






