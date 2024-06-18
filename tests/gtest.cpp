/**
 * @file gtest.cpp
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @brief Google Tests for scheduler.cpp
 */
#include <gtest/gtest.h>
#include "scheduler_test_system.hpp"
#include "test_ensembles.hpp"
class SchedulerTest : public ::testing::Test
{
protected:
    //! holds the deployment schedule created in @ref SchedulerTest()
    DeploymentSchedule_t deploymentSchedule[4];
    //! pointer for next event in deploymentSchedule
    DeploymentSchedule_t* nextEvent;
    //! next time any event will be run
    uint32_t nextEventTime;
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
        deploymentSchedule[2] = { nullptr, nullptr, 0, 0, 0, 0, 0, 0, 0, nullptr };
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
        deploymentSchedule[0] = { SS_ensembleAFunc, SS_ensembleAInit, 1, 0, 500, UINT32_MAX, 0, 0, 0, nullptr,200, 'A'};
        deploymentSchedule[1] = { SS_ensembleBFunc, SS_ensembleBInit, 1, 0, 200, UINT32_MAX, 0, 0, 0, nullptr,100, 'B' };
        deploymentSchedule[2] = { SS_ensembleCFunc, SS_ensembleCInit, 1, 0, 800, UINT32_MAX, 0, 0, 0, nullptr,250, 'C' };
    }
    /**
     * @brief setup function that recreates environment for every test
     * 
     */
    void SetUp() override
    {
        nextEvent = nullptr; //ensures that first call to scheduler is correct
        nextEventTime = 0; //time handling
        setTime(0); //!< time handling see @ref tests/scheduler_test_system.cpp
        SCH_initializeSchedule(deploymentSchedule, millis());

    }
    /**
     * @brief Cleans the test envirnoment
     * 
     */
    void TearDown() override
    {
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
        ASSERT_EQ(expectedTaskName, nextEvent->taskName);
        setTime(nextEventTime);
        ASSERT_EQ(expectedStart, nextEventTime);
        addTime(nextEvent->maxDuration);
        addTime(additionalTime);
        ASSERT_EQ(expectedEnd, millis());
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
        ASSERT_EQ(expectedTaskName, nextEvent->taskName);
        setTime(nextEventTime);
        ASSERT_EQ(expectedStart, nextEventTime);
        addTime(nextEvent->maxDuration);
        ASSERT_EQ(expectedEnd, millis());
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
        SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
        setTime(nextEventTime);
        addTime(nextEvent->maxDuration);
    }


};

/**
 * @brief Tests if all deployment schedules are properly initialized with
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
 * @brief Checks that events with no delays are fetched and executed properly.
 *
 */
TEST_F(SchedulerTest, TestIdeal)
{

    for (int i = 0; i < 10; i++)
    {
        SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);


        ASSERT_EQ(nextEvent->taskName, deploymentSchedule[i % 3].taskName);
        if (nextEvent != nullptr)
        {
            setTime(nextEventTime);
            ASSERT_EQ((nextEventTime - nextEvent->ensembleDelay) %
                                        nextEvent->ensembleInterval, 0);
            addTime(nextEvent->maxDuration);
        }
    }
}

/**
 * @brief First task starts with 200ms delay, next tasks run immediately after
 * and the future tasks are still run from their respective ensemble delays
 *
 */

TEST_F(SchedulerTest, DelayRunImmediately)
{

    deploymentSchedule[0].ensembleDelay = 0;
    deploymentSchedule[1].ensembleDelay = 500;
    deploymentSchedule[2].ensembleDelay = 800;

    SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    runAndCheckEventWithDelay('A', 0, 600, 200);

    SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    runAndCheckEvent('B', 600, 800);

    SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    runAndCheckEvent('C', 800, 1400);
    ASSERT_EQ(deploymentSchedule[0].ensembleDelay, 0);
    ASSERT_EQ(deploymentSchedule[1].ensembleDelay, 600);
    ASSERT_EQ(deploymentSchedule[2].ensembleDelay, 800);
    //verify no cascades
    for (int i = 3; i < 10; i++)
    {
        SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
        ASSERT_EQ(nextEvent->taskName, deploymentSchedule[i % 3].taskName);
        setTime(nextEventTime + nextEvent->maxDuration);
    }
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

    SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    runAndCheckEventWithDelay('C',600,2100,900);
   


    SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    runAndCheckEvent('B',2400,2600);

    SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);//C runs
    setTime(nextEventTime);
    addTime(nextEvent->maxDuration);



    SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    runAndCheckEvent('A', 3200, 3600);

    SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    ASSERT_EQ('A', nextEvent->taskName);
    setTime(nextEventTime);
    ASSERT_EQ(4000, nextEventTime);

    for (int i = 1; i < 10; i++)
    {
        SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
        ASSERT_EQ(nextEvent->taskName, deploymentSchedule[i % 3].taskName);
        setTime(nextEventTime);
        ASSERT_EQ((nextEventTime - nextEvent->ensembleDelay)
            % nextEvent->ensembleInterval, 0);
        addTime(nextEvent->maxDuration);
    }
}

/**
 * @brief Task B runs 900ms longer. Because task C has not been run yet, 
 * its ensemble delay is pushed back and it is run after the second run of B.
 * 
 */
TEST_F(SchedulerTest, DelayAndSkip)
{

    deploymentSchedule[0].ensembleDelay = 0;
    deploymentSchedule[1].ensembleDelay = 500;
    deploymentSchedule[2].ensembleDelay = 800;

    runNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    runNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    addTime(900);       //delay

    SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    runAndCheckEvent('A',2000,2400);

    ASSERT_TRUE(SCH_willOverlap(deploymentSchedule, 2, millis(), millis()
        + 600, millis()));
    runNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);//C runs
    ASSERT_EQ('C', nextEvent->taskName);
    setTime(nextEventTime);
    ASSERT_EQ(2600, nextEventTime);
    addTime(nextEvent->maxDuration);
    for (int i = 0; i < 10; i++)
    {
        SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
        ASSERT_EQ(nextEvent->taskName, deploymentSchedule[i % 3].taskName);
        setTime(nextEventTime);
        ASSERT_EQ((nextEventTime - nextEvent->ensembleDelay)
            % nextEvent->ensembleInterval, 0);
        addTime(nextEvent->maxDuration);
    }
}


/**
 * @brief Ensembles A, B, and C have intervals that dont allow all of 
 * them to be run every interval. C is supposed to run every 250 seconds but 
 * tasks A and B do not leave any idle time for it to run.
 * 
 */
TEST_F(SchedulerTest, ScheduleTooTight)
{
    schedule3();

    std::vector<TestLog> actual, expected;
    while (millis() < 1500)
    {
        runNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
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
    expected.emplace_back('B', 1300, 1400);
    expected.emplace_back('B', 1400, 1500);
    for (int i = 0; i < (expected.size() > actual.size() ?
        actual.size() : expected.size()); i++)
    {
        EXPECT_TRUE(expected[i] == actual[i]);
    }

}

/**
 * @brief Ensembles A, B, and C have intervals that dont allow all of 
 * them to be run every interval. C is able to run very infrequently.
 * 
 */
TEST_F(SchedulerTest, ScheduleVeryTight)
{
    schedule3();
    deploymentSchedule[1].ensembleInterval = 300;
    deploymentSchedule[2].maxDuration = 200;

    std::vector<TestLog> actual, expected;
    while (millis() < 2000)
    {
        runNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
        actual.emplace_back(nextEvent->taskName, nextEventTime, millis());
    }
    expected.emplace_back('A', 0, 200);
    expected.emplace_back('B', 200, 300);
    expected.emplace_back('C', 300, 500);
    expected.emplace_back('A', 500, 700);
    expected.emplace_back('B', 700, 800);
    expected.emplace_back('B', 800, 900);
    expected.emplace_back('A', 1000, 1200);
    expected.emplace_back('B', 1200, 1300);
    expected.emplace_back('B', 1400, 1500);
    expected.emplace_back('A', 1500, 1700);
    expected.emplace_back('B', 1700, 1800);
    expected.emplace_back('C', 1900, 2100);

    for (int i = 0; i < (expected.size() > actual.size() 
                    ? actual.size() : expected.size()); i++)
    {
        EXPECT_TRUE(expected[i] == actual[i]);
    }

}



/**
 * @brief Testing no overlap with A before B
 * 
 * A   ***********
 * B                **********
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
    bool overlap = SCH_willOverlap(deploymentSchedule, i, 0, end, start);
    ASSERT_FALSE(overlap);
}



/**
 * @brief Testing overlap with end of A and start of B
 * 
 * A   ***********
 * B           **********
 */
TEST_F(SchedulerTest, OverlapBefore)
{
    schedule2();

    deploymentSchedule[0].deploymentStartTime = 0;
    deploymentSchedule[0].ensembleDelay = 250;
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
    bool overlap = SCH_willOverlap(deploymentSchedule, i, 0, end, start);
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
    bool overlap = SCH_willOverlap(deploymentSchedule, i, 0, end, start);
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
    deploymentSchedule[0].ensembleDelay = 650;
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
    bool overlap = SCH_willOverlap(deploymentSchedule, i, 0, end, start);
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
    deploymentSchedule[0].ensembleDelay = 0;
    deploymentSchedule[0].maxDuration = 800;
    deploymentSchedule[0].ensembleInterval = 1000;

    
    deploymentSchedule[1].deploymentStartTime = 0;
    deploymentSchedule[1].ensembleDelay = 500;
    deploymentSchedule[1].maxDuration = 200;
    deploymentSchedule[1].ensembleInterval = 1000;


    int i = 1;
    uint32_t start = deploymentSchedule[i].deploymentStartTime +
        deploymentSchedule[i].ensembleDelay;
    uint32_t end = start + deploymentSchedule[i].maxDuration;
    bool overlap = SCH_willOverlap(deploymentSchedule, i, 0, end, start);
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
    deploymentSchedule[0].ensembleDelay = 0;
    deploymentSchedule[0].maxDuration = 500;
    deploymentSchedule[0].ensembleInterval = 1000;

    deploymentSchedule[1].deploymentStartTime = 0;
    deploymentSchedule[1].ensembleDelay = 500;
    deploymentSchedule[1].maxDuration = 200;
    deploymentSchedule[1].ensembleInterval = 1000;


    int i = 1;
    uint32_t start = deploymentSchedule[i].deploymentStartTime +
        deploymentSchedule[i].ensembleDelay;
    uint32_t end = start + deploymentSchedule[i].maxDuration;
    bool overlap = SCH_willOverlap(deploymentSchedule, i, 0, end, start);
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
    bool overlap = SCH_willOverlap(deploymentSchedule, i, 0, end, start);
    ASSERT_FALSE(overlap);
}