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



class SchedulerTest : public ::testing::Test
{
protected:
    //! holds the deployment schedule created in @ref SchedulerTest()
    DeploymentSchedule_t deploymentSchedule[4];
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
    std::string expectedFileName = "expected.log";
    //! filename for writing actual test output
    std::string actualFileName = "actual.log";
    //! holds test name across functions
    std::string testName;


    //! for writing test output
    bool useCompareLogs;

    static void SetUpTestSuite()
    {

        std::ofstream actualFile("actual.log",
                    std::ios::out | std::ios::trunc);

        std::ofstream expectedFile("expected.log",
                    std::ios::out);

        if (actualFile.is_open() && expectedFile.is_open())
        {
            actualFile << "{";
            expectedFile << "{";
        }
        actualFile.close();
        expectedFile.close();
    }
    static void TearDownTestSuite()
    {
        std::ofstream actualFile("actual.log", std::ios::out | std::ios::app);
        std::ofstream expectedFile("expected.log", std::ios::out |
            std::ios::app);

        if (actualFile.is_open() && expectedFile.is_open())
        {
            actualFile << "\n}";
            expectedFile << "\n}";
        }

        actualFile.close();
        expectedFile.close();
    }

    /**
    * @brief Construct a new Scheduler Test object
    *
    * Creates a deployment schedule with three ensembles:
    * Ensemble A runs every 2000ms with a max duration of 400ms
    * Ensemble B runs every 2000ms with a max duration of 200ms
    * Ensemble C runs every 2000ms with a max duration of 600ms
    */
    SchedulerTest()
    {
        deploymentSchedule[0] = { SS_ensembleAFunc, SS_ensembleAInit, 1, 0, 2000, UINT32_MAX, 0, 0, 0, nullptr, 400, 'A' };
        deploymentSchedule[1] = { SS_ensembleBFunc, SS_ensembleBInit, 1, 0, 2000, UINT32_MAX, 0, 0, 0, nullptr, 200, 'B' };
        deploymentSchedule[2] = { SS_ensembleCFunc, SS_ensembleCInit, 1, 0, 2000, UINT32_MAX, 0, 0, 0, nullptr, 600, 'C' };
        deploymentSchedule[3] = { nullptr,          nullptr, 0,       0, 0, 0,    0,          0, 0, nullptr,    0,   '\0' };
    }
    /**
    * @brief Modifies the constructed scheduler
    *  Creates a deployment schedule with one ensemble:
    *  Ensemble A runs every 2000ms with a max duration of 400ms
    */
    void schedule2()
    {
        deploymentSchedule[2] = {nullptr, nullptr, 0, 0, 0, 0, 0, 0, 0, nullptr};
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
        deploymentSchedule[0] = { SS_ensembleAFunc, SS_ensembleAInit, 1, 0, 500, UINT32_MAX, 0, 0, 0, nullptr, 200, 'A' };
        deploymentSchedule[1] = { SS_ensembleBFunc, SS_ensembleBInit, 1, 0, 200, UINT32_MAX, 0, 0, 0, nullptr, 100, 'B' };
        deploymentSchedule[2] = { SS_ensembleCFunc, SS_ensembleCInit, 1, 0, 800, UINT32_MAX, 0, 0, 0, nullptr, 250, 'C' };
    }
    /**
    * @brief Modifies the constructed scheduler
    * Creates a deployment schedule with three ensembles:
    * Ensemble A runs every 200ms with a max duration of 150ms
    */
    void schedule4()
    {
        deploymentSchedule[0] = { SS_ensembleAFunc, SS_ensembleAInit, 1, 0, 200, UINT32_MAX, 0, 0, 0, nullptr, 150, 'A' };
        deploymentSchedule[1] = { nullptr,          nullptr, 0,       0, 0, 0,   0,          0, 0, nullptr, 0,   '\0' };
    }
    
    /**
     * @brief Add expected log to vector
     * @param task the name of the task being run
     * @param start the start time
     * @param end the end tim
     */
    inline void appendExpectedFile(char task, uint32_t start, uint32_t end)
    {
        expected.emplace_back(task, start, end);
    }
    /**
     * @brief Add actual log to vector
     * @param task the name of the task being run
     * @param start the start time
     * @param end the end tim
     */
    inline void appendActualFile(char task, uint32_t start, uint32_t end)
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

        nextEvent = nullptr; //ensures that first call to scheduler is correct
        nextEventTime = 0; //time handling
        setTime(0); //!< time handling see @ref tests/scheduler_test_system.cpp
        SCH_initializeSchedule(deploymentSchedule, millis());
        useCompareLogs = false;
        expectedFile.open(expectedFileName, std::ios::out | std::ios::app);
        actualFile.open(actualFileName, std::ios::out | std::ios::app);

    }
    /**
     * @brief Cleans the test envirnoment after each test
     *
     */
    void TearDown() override
    {

        static bool firstTest = true;
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
        if (!expected.empty())
        {


            for (int i = 0; i < expected.size() - 1; i++)
            {
                actualFile << expected[i] << ", ";
            }
            actualFile << expected.back();
        }
        actualFile << "]";
        actualFile.close();

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
    void runAndCheckEventWithDelay(char expectedTaskName,
                                    uint32_t expectedStart,
                                    uint32_t expectedEnd,
                                    uint32_t additionalTime)
    {

        ASSERT_NE(nextEvent, nullptr) << "Scheduler returned nullptr.";


        ASSERT_EQ(expectedTaskName, nextEvent->taskName)
            << "runAndCheckEvent failed:\n"
            << "Expected \t Actual\n"
            << expectedTaskName << "\t\t" << nextEvent->taskName << "\n"
            << expectedStart << "\t\t" << nextEventTime << "\n"
            << expectedEnd << "\t\t" << nextEventTime + nextEvent->maxDuration;
        setTime(nextEventTime);
        ASSERT_EQ(expectedStart, nextEventTime)
            << "runAndCheckEvent failed:\n"
            << "Expected \t Actual\n"
            << expectedTaskName << "\t\t" << nextEvent->taskName << "\n"
            << expectedStart << "\t\t" << nextEventTime << "\n"
            << expectedEnd << "\t\t" << nextEventTime + nextEvent->maxDuration;
        addTime(nextEvent->maxDuration);
        addTime(additionalTime);
        ASSERT_EQ(expectedEnd, millis())
            << "runAndCheckEvent failed:\n"
            << "Expected \t Actual\n"
            << expectedTaskName << "\t\t" << nextEvent->taskName << "\n"
            << expectedStart << "\t\t" << nextEventTime << "\n"
            << expectedEnd << "\t\t" << nextEventTime + nextEvent->maxDuration;



    }

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
    void runAndCheckEvent(char expectedTaskName,
                                    uint32_t expectedStart,
                                    uint32_t expectedEnd)
    {
        ASSERT_NE(nextEvent, nullptr) << "Scheduler returned nullptr.";


        ASSERT_EQ(expectedTaskName, nextEvent->taskName)
            << "runAndCheckEvent failed:\n"
            << "Expected \t Actual\n"
            << expectedTaskName << "\t\t" << nextEvent->taskName << "\n"
            << expectedStart << "\t\t" << nextEventTime << "\n"
            << expectedEnd << "\t\t" << nextEventTime + nextEvent->maxDuration;
        setTime(nextEventTime);
        ASSERT_EQ(expectedStart, nextEventTime)
            << "runAndCheckEvent failed:\n"
            << "Expected \t Actual\n"
            << expectedTaskName << "\t\t" << nextEvent->taskName << "\n"
            << expectedStart << "\t\t" << nextEventTime << "\n"
            << expectedEnd << "\t\t" << nextEventTime + nextEvent->maxDuration;
        addTime(nextEvent->maxDuration);
        ASSERT_EQ(expectedEnd, millis())
            << "runAndCheckEvent failed:\n"
            << "Expected \t Actual\n"
            << expectedTaskName << "\t\t" << nextEvent->taskName << "\n"
            << expectedStart << "\t\t" << nextEventTime << "\n"
            << expectedEnd << "\t\t" << nextEventTime + nextEvent->maxDuration;
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
    void runNextEvent(DeploymentSchedule_t* ScheduleTable_t,
            DeploymentSchedule_t** p_nextEvent,
            system_tick_t* p_nextTime)
    {

        SCH_getNextEvent(deploymentSchedule,
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

    void checkIdeal(uint32_t end)
    {
        useCompareLogs = true;
        if (end > 10000)
        {
            end = 10000;
        }
        std::vector<TestLog> expectedIdeal;
        expectedIdeal.emplace_back('A', 0, 400);
        expectedIdeal.emplace_back('B', 400, 600);
        expectedIdeal.emplace_back('C', 600, 1200);

        expectedIdeal.emplace_back('A', 2000, 2400);
        expectedIdeal.emplace_back('B', 2400, 2600);
        expectedIdeal.emplace_back('C', 2600, 3200);

        expectedIdeal.emplace_back('A', 4000, 4400);
        expectedIdeal.emplace_back('B', 4400, 4600);
        expectedIdeal.emplace_back('C', 4600, 5200);

        expectedIdeal.emplace_back('A', 6000, 6400);
        expectedIdeal.emplace_back('B', 6400, 6600);
        expectedIdeal.emplace_back('C', 6600, 7200);

        expectedIdeal.emplace_back('A', 8000, 8400);
        expectedIdeal.emplace_back('B', 8400, 8600);
        expectedIdeal.emplace_back('C', 8600, 9200);

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
            runNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
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
    void singleEventStart()
    {
        schedule4();
        //! ideal behaivor
        SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
        runAndCheckEvent('A', 0, 150);
        SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
        runAndCheckEvent('A', 200, 350);
        SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
        runAndCheckEvent('A', 400, 550);
    }
    void singleEventEndCheck()
    {
        schedule4();
        //! ideal behaivor
        SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
        runAndCheckEvent('A', 0, 150);
        SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
        runAndCheckEvent('A', 200, 350);
        SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
        runAndCheckEvent('A', 400, 550);
    }
};


/**
 * @brief Tests initialization of deployment schedule
 *
 * Tests if all deployment schedules are properly initialized with
 * non-null init functions and the system time is correctly reset to zero.
 */
TEST_F(SchedulerTest, TestInitialization)
{
    ASSERT_NE(nullptr, deploymentSchedule[0].init);
    ASSERT_NE(nullptr, deploymentSchedule[1].init);
    ASSERT_NE(nullptr, deploymentSchedule[2].init);
    ASSERT_EQ(millis(), 0);
}


/**
 * @brief Checks ideal behaivor for single task
 */
TEST_F(SchedulerTest, SingleEventIdeal)
{
    schedule4();
    //! ideal behaivor
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('A', 0, 150);
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('A', 200, 350);
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('A', 400, 550);
}
/**
 * @brief Boundary delay case for single event schedule
 *
 * task end delayed to when second event is supposed to start, so schedule
 * unchanged
 */
TEST_F(SchedulerTest, SingleEvent_DelayToSecondStart)
{
    singleEventStart();
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEventWithDelay('A', 600, 800, 50);
    //! last task ended at 800ms, should not impact next task
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('A', 800, 950);
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('A', 1000, 1150);

}
/**
* @brief Seccond iteration starts late with enough time to run

* task end delayed past when second event is supposed to start, but leaves
* enough time to run before third task
*/
TEST_F(SchedulerTest, SingleEvent_SecondDelayedNoImpact)
{
    singleEventStart();
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEventWithDelay('A', 600, 810, 60);
    //! last task ended at 810 ms, this task should be run immediately
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('A', 810, 960);
    //! task not delayed, should run as normal
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('A', 1000, 1150);


}
/**
* @brief Seccond iteration starts late with enough time to run

* task end delayed past when second event start, but leaves
* enough time to run and end when third task starts
*/
TEST_F(SchedulerTest, SingleEvent_SecondEndAtThirdStart)
{
    singleEventStart();
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEventWithDelay('A', 600, 850, 100);
    //! last task ended at 2050 ms, this task should be run immediately
    //! as it will now end when the next task is supposed to start
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('A', 850, 1000);
    //! task not delayed, should run as normal
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('A', 1000, 1150);


}
/**
 * @brief Seccond iteration starts late without enough time to run
 *
* task end delayed past when second event is supposed to start, second
* event would now overlap with third run time, so second event is skipped
*/
TEST_F(SchedulerTest, SingleEvent_SecondEndAfterThirdStart)
{

    singleEventStart();
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEventWithDelay('A', 600, 860, 110);
    //! second event skipped running third task
    //! task not delayed, should run as normal
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('A', 1000, 1150);


}

/**
* @brief Seccond iteration ends when third iteration woas supposed to end time
* task end delayed past when second event is supposed to start, second
* event would now start when third run would start,
* so second event is skipped
*/
TEST_F(SchedulerTest, SingleEvent_ExactOverlap)
{
    singleEventStart();
    uint32_t count = deploymentSchedule[0].measurementCount;
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEventWithDelay('A', 600, 1000, 250);
    //! second event skipped running third task
    //! task not delayed, should run as normal
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('A', 1000, 1150);
    //! check only two runs
    ASSERT_EQ(deploymentSchedule[0].measurementCount - count, 2);


}
/**
 * @brief Seccond iteration ends after third iteration start time
 *
* task end delayed past ends 50 ms after third task should run
* second task skipped, third task still runs
*/
TEST_F(SchedulerTest, SingleEvent_DelayPastThirdStart)
{

    singleEventStart();
    uint32_t count = deploymentSchedule[0].measurementCount;
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEventWithDelay('A', 600, 1050, 300);
    //! second event skipped running third task
    //! task not delayed, should run as normal
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('A', 1050, 1200);
    //! check only two runs
    ASSERT_EQ(deploymentSchedule[0].measurementCount - count, 2);


}
/**
* task end delayed past ends 60 ms after third task should run
* second task skipped, third task will be skipped as well
*/
TEST_F(SchedulerTest, SingleEvent_SkipSecondAndThird)
{

    singleEventStart();
    uint32_t count = deploymentSchedule[0].measurementCount;
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEventWithDelay('A', 600, 1060, 310);
    //! second event skipped running third task
    //! task not delayed, should run as normal
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('A', 1200, 1350);
    //! check only two runs
    ASSERT_EQ(deploymentSchedule[0].measurementCount - count, 2);
}


/**
 * @brief Checks that events with no delays are fetched and executed properly.
 *
 * This test  runs through expected behaivor with no delays and enough
 * time for all the functions to run by themselves in order.
 *
 */
TEST_F(SchedulerTest, TestIdeal)
{
    checkIdeal(8000);
}

/**
 * @brief First task starts with 200ms delay, next tasks run immediately after
 * and the future tasks are still run from their respective ensemble delays
 * without changing those ensemble delays
 *
 */
TEST_F(SchedulerTest, DelayRunImmediately)
{

    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEventWithDelay('A', 0, 600, 200);

    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('B', 600, 800);

    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('C', 800, 1400);
    //! ensure ensemble delays unchanged
    ASSERT_EQ(deploymentSchedule[0].ensembleDelay, 0);
    ASSERT_EQ(deploymentSchedule[1].ensembleDelay, 0);
    ASSERT_EQ(deploymentSchedule[2].ensembleDelay, 0);

    checkIdeal(10000);
}

/**
 * @brief Task C runs 900ms long, delaying second run of task A. Task A would
 * now delay Task B so A is run during idle time after task C is run again.
 *
 */
TEST_F(SchedulerTest, DelayRunDuringIdle)
{

    runNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);

    runNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);

    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEventWithDelay('C', 600, 2100, 900);



    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('A', 2100, 2500);
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('B', 2500, 2700);
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('C', 2700, 3300);



    checkIdeal(10000);
}

/**
 * @brief Task B runs 900ms longer. Because task C has not been run yet,
 *  it is run after the second run of B.
 *
 */
TEST_F(SchedulerTest, DelayAndSkip)
{

    runNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    runNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    addTime(900); //delay

    ASSERT_TRUE(SCH_willOverlap(deploymentSchedule, 2, millis(), millis()));
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('A', 2000, 2400);
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('B', 2400, 2600);


    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());//C runs
    runAndCheckEvent('C', 2600, 3200);
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('A', 4000, 4400);

    //! check for cascades
    checkIdeal(10000);


}


/**
 * @brief Ensembles A, B, and C have intervals that dont allow all of
 * them to be run every interval. C is supposed to run every 250 seconds but
 * tasks A and B do not leave any idle time for it to run.
 */
TEST_F(SchedulerTest, ScheduleTooTight)
{
    schedule3();
    useCompareLogs = true;

    while (millis() < 1500)
    {
        runNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
        ASSERT_NE(nextEvent, nullptr) << "Scheduler returned nullptr.";
        if (nextEvent == nullptr) return;
        actual.emplace_back(nextEvent->taskName, nextEventTime, millis());
    }
    expected.emplace_back('A', 0, 200);
    expected.emplace_back('B', 200, 300);
    expected.emplace_back('B', 400, 500);
    expected.emplace_back('A', 500, 700);
    expected.emplace_back('B', 700, 800);
    expected.emplace_back('B', 800, 900);
    expected.emplace_back('A', 1000, 1200);
    expected.emplace_back('B', 1200, 1300);
    expected.emplace_back('B', 1400, 1500);
    compareLogs();

}






/**
 * @brief Testing back-to-back scheduling without overlap
 *
 * A  *********
 * B            **********
 * C                      **********
 */
TEST_F(SchedulerTest, BackToBackScheduling)
{


    deploymentSchedule[0].deploymentStartTime = 0;
    deploymentSchedule[0].ensembleDelay = 0;
    deploymentSchedule[0].maxDuration = 500;
    deploymentSchedule[0].ensembleInterval = 1500;

    deploymentSchedule[1].deploymentStartTime = 0;
    deploymentSchedule[1].ensembleDelay = 500;
    deploymentSchedule[1].maxDuration = 500;
    deploymentSchedule[1].ensembleInterval = 1500;

    deploymentSchedule[2].ensembleInterval = 1500;
    deploymentSchedule[2].maxDuration = 500;

    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('A', 0, 500);

    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('B', 500, 1000);

    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('C', 1000, 1500);
}

/**
 * @brief Testing idle periods where no tasks run
 *
 * A  *********
 * B               **********
 *
 */
TEST_F(SchedulerTest, IdlePeriod)
{
    schedule2();

    deploymentSchedule[0].deploymentStartTime = 0;
    deploymentSchedule[0].ensembleDelay = 0;
    deploymentSchedule[0].maxDuration = 500;
    deploymentSchedule[0].ensembleInterval = 2000;

    deploymentSchedule[1].deploymentStartTime = 0;
    deploymentSchedule[1].ensembleDelay = 1000;
    deploymentSchedule[1].maxDuration = 500;
    deploymentSchedule[1].ensembleInterval = 2000;

    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('A', 0, 500);

    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('B', 1000, 1500);

    // Ensure that no event is scheduled during the idle period
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    ASSERT_EQ(nextEventTime, 2000);
}

/**
 * @brief Testing when maximum durations span multiple intervals
 *
*/
TEST_F(SchedulerTest, MaximumDurationSpan)
{
    schedule2();

    deploymentSchedule[0].deploymentStartTime = 0;
    deploymentSchedule[0].ensembleDelay = 0;
    deploymentSchedule[0].maxDuration = 1500;
    deploymentSchedule[0].ensembleInterval = 3000;

    deploymentSchedule[1].deploymentStartTime = 0;
    deploymentSchedule[1].ensembleDelay = 0;
    deploymentSchedule[1].maxDuration = 100;
    deploymentSchedule[1].ensembleInterval = 500;

    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('A', 0, 1500);

    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('B', 1500, 1600);
}


/**
 * @brief Testing tasks with large intervals and small durations
 *
 */
TEST_F(SchedulerTest, LargeIntervalsSmallDurations)
{
    schedule2();

    deploymentSchedule[0].deploymentStartTime = 0;
    deploymentSchedule[0].ensembleDelay = 0;
    deploymentSchedule[0].maxDuration = 100;
    deploymentSchedule[0].ensembleInterval = 1000;

    deploymentSchedule[1].deploymentStartTime = 0;
    deploymentSchedule[1].ensembleDelay = 800;
    deploymentSchedule[1].maxDuration = 100;
    deploymentSchedule[1].ensembleInterval = 1000;

    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('A', 0, 100);

    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    ASSERT_EQ(nextEventTime, 800);
    runAndCheckEvent('B', 800, 900);
}


/**
 * @brief Testing no overlap with A before B
 *
 * A   ***********
 * B                  **********
 */
TEST_F(SchedulerTest, NoOverlapBefore)
{
    schedule2();

    deploymentSchedule[0].deploymentStartTime = 0;
    deploymentSchedule[0].ensembleDelay = 0;
    deploymentSchedule[0].maxDuration = 300;
    deploymentSchedule[0].ensembleInterval = 1000;

    deploymentSchedule[1].deploymentStartTime = 0;
    deploymentSchedule[1].ensembleDelay = 500;
    deploymentSchedule[1].maxDuration = 200;
    deploymentSchedule[1].ensembleInterval = 1000;

    int i = 1;
    uint32_t start = deploymentSchedule[i].deploymentStartTime +
        deploymentSchedule[i].ensembleDelay;
    uint32_t end = start + deploymentSchedule[i].maxDuration;
    bool overlap = SCH_willOverlap(deploymentSchedule, i, 0, start);
    ASSERT_FALSE(overlap);
}



/**
 * @brief Testing overlap with end of A and start of B
 *
 * A   ***********
 * B          ***********
 */
TEST_F(SchedulerTest, OverlapBefore)
{
    schedule2();

    deploymentSchedule[0].deploymentStartTime = 0;
    deploymentSchedule[0].nextRunTime = 250;
    deploymentSchedule[0].maxDuration = 300;
    deploymentSchedule[0].ensembleInterval = 1000;


    deploymentSchedule[1].deploymentStartTime = 0;
    deploymentSchedule[1].nextRunTime = 500;
    deploymentSchedule[1].maxDuration = 200;
    deploymentSchedule[1].ensembleInterval = 1000;


    int i = 1;
    uint32_t start = deploymentSchedule[i].nextRunTime;
    uint32_t end = start + deploymentSchedule[i].maxDuration;
    bool overlap = SCH_willOverlap(deploymentSchedule, i, 0, start);
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
TEST_F(SchedulerTest, NoOverlapAfter)
{
    schedule2();

    deploymentSchedule[0].deploymentStartTime = 0;
    deploymentSchedule[0].ensembleDelay = 800;
    deploymentSchedule[0].maxDuration = 300;
    deploymentSchedule[0].ensembleInterval = 1000;

    deploymentSchedule[1].deploymentStartTime = 0;
    deploymentSchedule[1].ensembleDelay = 500;
    deploymentSchedule[1].maxDuration = 200;
    deploymentSchedule[1].ensembleInterval = 1000;

    int i = 1;
    uint32_t start = deploymentSchedule[i].deploymentStartTime +
        deploymentSchedule[i].ensembleDelay;
    uint32_t end = start + deploymentSchedule[i].maxDuration;
    bool overlap = SCH_willOverlap(deploymentSchedule, i, 0, start);
    ASSERT_FALSE(overlap);
}

/**
 * @brief Testing overlap with end of B and start of A
 *
 * A                   ***********
 * B           **********
 */
TEST_F(SchedulerTest, OverlapAfter)
{
    schedule2();

    deploymentSchedule[0].deploymentStartTime = 0;
    deploymentSchedule[0].nextRunTime = 650;
    deploymentSchedule[0].maxDuration = 300;
    deploymentSchedule[0].ensembleInterval = 1000;


    deploymentSchedule[1].deploymentStartTime = 0;
    deploymentSchedule[1].nextRunTime = 500;
    deploymentSchedule[1].maxDuration = 200;
    deploymentSchedule[1].ensembleInterval = 1000;


    int i = 1;
    uint32_t start = deploymentSchedule[i].nextRunTime;
    uint32_t end = start + deploymentSchedule[i].maxDuration;
    bool overlap = SCH_willOverlap(deploymentSchedule, i, 0, start);
    ASSERT_TRUE(overlap);
}


/**
 * @brief Testing for overlap when B starts after A starts and before A ends
 *
 * A           *****************
 * B              **********
 *
 */
TEST_F(SchedulerTest, InsideOverlap)
{
    schedule2();

    deploymentSchedule[0].deploymentStartTime = 0;
    deploymentSchedule[0].nextRunTime = 0;
    deploymentSchedule[0].maxDuration = 800;
    deploymentSchedule[0].ensembleInterval = 1000;


    deploymentSchedule[1].deploymentStartTime = 0;
    deploymentSchedule[1].nextRunTime = 500;
    deploymentSchedule[1].maxDuration = 200;
    deploymentSchedule[1].ensembleInterval = 1000;


    int i = 1;
    uint32_t start = deploymentSchedule[i].nextRunTime;
    uint32_t end = start + deploymentSchedule[i].maxDuration;
    bool overlap = SCH_willOverlap(deploymentSchedule, i, 0, start);
    ASSERT_TRUE(overlap);
}


/**
 * @brief Testing no overlap edge case when B starts as A ends
 *
 * A  *********
 * B           **********
 */
TEST_F(SchedulerTest, Boundary1)
{
    schedule2();

    deploymentSchedule[0].deploymentStartTime = 0;
    deploymentSchedule[0].nextRunTime = 0;
    deploymentSchedule[0].maxDuration = 500;
    deploymentSchedule[0].ensembleInterval = 1000;

    deploymentSchedule[1].deploymentStartTime = 0;
    deploymentSchedule[1].nextRunTime = 500;
    deploymentSchedule[1].maxDuration = 200;
    deploymentSchedule[1].ensembleInterval = 1000;


    int i = 1;
    uint32_t start = deploymentSchedule[i].nextRunTime;
    uint32_t end = start + deploymentSchedule[i].maxDuration;
    bool overlap = SCH_willOverlap(deploymentSchedule, i, 0, start);
    ASSERT_FALSE(overlap);
}


/**
 * @brief Testing no overlap edge case when A starts as B ends
 *
 * A                     *********
 * B           **********
 */
TEST_F(SchedulerTest, Boundary2)
{
    schedule2();

    deploymentSchedule[0].deploymentStartTime = 0;
    deploymentSchedule[0].ensembleDelay = 700;
    deploymentSchedule[0].maxDuration = 300;
    deploymentSchedule[0].ensembleInterval = 1000;


    deploymentSchedule[1].deploymentStartTime = 0;
    deploymentSchedule[1].ensembleDelay = 500;
    deploymentSchedule[1].maxDuration = 200;
    deploymentSchedule[1].ensembleInterval = 1000;


    int i = 1;
    uint32_t start = deploymentSchedule[i].deploymentStartTime +
        deploymentSchedule[i].ensembleDelay;
    uint32_t end = start + deploymentSchedule[i].maxDuration;
    bool overlap = SCH_willOverlap(deploymentSchedule, i, 0, start);
    ASSERT_FALSE(overlap);
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('B', 500, 700);
    SCH_getNextEvent(deploymentSchedule, &nextEvent,
                            &nextEventTime, millis());
    runAndCheckEvent('A', 700, 1000);
}