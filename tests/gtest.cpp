#include <gtest/gtest.h>
#include "../tests/scheduler_test_system.hpp"
#include "scheduler_functions.hpp"
class SchedulerTest : public ::testing::Test
{
protected:
    DeploymentSchedule_t deploymentSchedule[4];
    DeploymentSchedule_t* nextEvent;
    uint32_t nextEventTime;
    SchedulerTest()
    {
        deploymentSchedule[0] = { SS_ensembleAFunc, SS_ensembleAInit, 1,
                millis(), 2000, UINT32_MAX, 0, 0, 0, nullptr, 400, (char)65 };
        deploymentSchedule[1] = { SS_ensembleBFunc, SS_ensembleBInit, 1,
                millis(), 2000, UINT32_MAX, 0, 0, 0, nullptr, 200, (char)66 };
        deploymentSchedule[2] = { SS_ensembleCFunc, SS_ensembleCInit, 1,
                millis(), 2000, UINT32_MAX, 0, 0, 0, nullptr, 600, (char)67 };
        deploymentSchedule[3] = { nullptr, nullptr, 0, 0, 0, 0, 0, 0, 0,
                nullptr };
    }
    void schedule2()
    {
        deploymentSchedule[2] = { nullptr, nullptr, 0, 0, 0, 0, 0, 0, 0,
            nullptr };
    }
    void schedule3()
    {
        deploymentSchedule[0] = { SS_ensembleAFunc, SS_ensembleAInit, 1,
                millis(), 500, UINT32_MAX, 0, 0, 0, nullptr,200,(char)65 };
        deploymentSchedule[1] = { SS_ensembleBFunc, SS_ensembleBInit, 1,
                millis(), 200, UINT32_MAX, 0, 0, 0, nullptr,100,(char)66 };
        deploymentSchedule[2] = { SS_ensembleCFunc, SS_ensembleCInit, 1,
                millis(), 800, UINT32_MAX, 0, 0, 0, nullptr,250,(char)67 };
    }
    void SetUp() override
    {
        nextEvent = nullptr;
        nextEventTime = 0;
        setTime(0);
        SCH_initializeSchedule(deploymentSchedule, millis());

    }

    void TearDown() override
    {
    }
    void verifyNextEvent(char expectedTaskName, uint32_t expectedStart,
        uint32_t expectedEnd, uint32_t additionalTime)
    {
        ASSERT_EQ(expectedTaskName, nextEvent->taskName);
        setTime(nextEventTime);
        ASSERT_EQ(expectedStart, nextEventTime);
        addTime(nextEvent->maxDuration);
        addTime(additionalTime);
        ASSERT_EQ(expectedEnd, millis());
    }
    void verifyNextEvent(char expectedTaskName, uint32_t expectedStart,
        uint32_t expectedEnd)
    {
        ASSERT_EQ(expectedTaskName, nextEvent->taskName);
        setTime(nextEventTime);
        ASSERT_EQ(expectedStart, nextEventTime);
        addTime(nextEvent->maxDuration);
        ASSERT_EQ(expectedEnd, millis());
    }


};


TEST_F(SchedulerTest, TestInitialization)
{
    ASSERT_NE(nullptr, deploymentSchedule[0].init);
    ASSERT_NE(nullptr, deploymentSchedule[1].init);
    ASSERT_NE(nullptr, deploymentSchedule[2].init);
    ASSERT_EQ(millis(), 0);
}

TEST_F(SchedulerTest, TestIdeal)
{

    for (int i = 0; i < 10; i++)
    {
        SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);


        ASSERT_EQ(nextEvent->taskName, deploymentSchedule[i % 3].taskName);
        if (nextEvent != nullptr)
        {
            setTime(nextEventTime);
            ASSERT_EQ((nextEventTime - nextEvent->ensembleDelay) % nextEvent->ensembleInterval, 0);
            addTime(nextEvent->maxDuration);
        }
    }
}

TEST_F(SchedulerTest, DelayRunImmediately)
{

    deploymentSchedule[0].ensembleDelay = 0;
    deploymentSchedule[1].ensembleDelay = 500;
    deploymentSchedule[2].ensembleDelay = 800;

    SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    verifyNextEvent('A', 0, 700, 200);



    SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    verifyNextEvent('B', 600, 800, 0);

    SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    verifyNextEvent('C', 800, 1400, 0);
    //verify no cascades
    for (int i = 3; i < 10; i++)
    {

        SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
        ASSERT_EQ(nextEvent->taskName, deploymentSchedule[i % 3].taskName);
        setTime(nextEventTime + nextEvent->maxDuration);

    }
}

TEST_F(SchedulerTest, DelayRunDuringIdle)
{

    SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    setTime(nextEventTime);
    addTime(nextEvent->maxDuration);
    SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    setTime(nextEventTime);
    addTime(nextEvent->maxDuration);

    SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    ASSERT_EQ('C', nextEvent->taskName);
    setTime(nextEventTime);
    ASSERT_EQ(600, nextEventTime);
    addTime(nextEvent->maxDuration);
    addTime(900);       //delay


    SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    ASSERT_EQ('B', nextEvent->taskName);
    setTime(nextEventTime);
    ASSERT_EQ(2400, nextEventTime);
    addTime(nextEvent->maxDuration);

    SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);//C runs
    setTime(nextEventTime);
    addTime(nextEvent->maxDuration);



    SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    verifyNextEvent('A', 3200, 3600);

    SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    ASSERT_EQ('A', nextEvent->taskName);
    setTime(nextEventTime);
    ASSERT_EQ(4000, nextEventTime);

    for (int i = 1; i < 10; i++)
    {
        SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
        ASSERT_EQ(nextEvent->taskName, deploymentSchedule[i % 3].taskName);
        setTime(nextEventTime);
        ASSERT_EQ((nextEventTime - nextEvent->ensembleDelay) % nextEvent->ensembleInterval, 0);
        addTime(nextEvent->maxDuration);
    }
}


TEST_F(SchedulerTest, DelayAndSkip)
{

    deploymentSchedule[0].ensembleDelay = 0;
    deploymentSchedule[1].ensembleDelay = 500;
    deploymentSchedule[2].ensembleDelay = 800;

    SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    setTime(nextEventTime);
    addTime(nextEvent->maxDuration);
    SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    setTime(nextEventTime);
    addTime(nextEvent->maxDuration);
    addTime(900);       //delay

    SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    ASSERT_EQ('A', nextEvent->taskName);
    setTime(nextEventTime);
    ASSERT_EQ(2000, nextEventTime);
    addTime(nextEvent->maxDuration);

    ASSERT_TRUE(SCH_willOverlap(deploymentSchedule, 2, millis(), millis() + 600, millis()));
    SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
    setTime(nextEventTime);
    addTime(nextEvent->maxDuration);
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
        ASSERT_EQ((nextEventTime - nextEvent->ensembleDelay) % nextEvent->ensembleInterval, 0);
        addTime(nextEvent->maxDuration);
    }
}



TEST_F(SchedulerTest, OverlapAtBoundary)
{
    schedule2();


    deploymentSchedule[0].deploymentStartTime = 0;
    deploymentSchedule[0].ensembleDelay = 0;
    deploymentSchedule[0].maxDuration = 400;
    deploymentSchedule[1].deploymentStartTime = 0;
    deploymentSchedule[1].ensembleDelay = 400;
    bool overlap = SCH_willOverlap(deploymentSchedule, 0, 0, 400, 0);
    ASSERT_FALSE(overlap);
}

TEST_F(SchedulerTest, ScheduleTooTight)
{
    schedule3();

    std::vector<TestLog> actual, expected;
    while (millis() < 1500)
    {
        SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
        setTime(nextEventTime);
        addTime(nextEvent->maxDuration);
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
    for (int i = 0; i < (expected.size() > actual.size() ? actual.size() : expected.size()); i++)
    {
        EXPECT_TRUE(expected[i] == actual[i]);
    }

}


TEST_F(SchedulerTest, ScheduleVeryTight)
{
    schedule3();
    deploymentSchedule[1].ensembleInterval = 300;
    deploymentSchedule[2].maxDuration = 200;

    std::vector<TestLog> actual, expected;
    while (millis() < 2000)
    {
        SCH_getNextEvent(deploymentSchedule, &nextEvent, &nextEventTime);
        setTime(nextEventTime);
        addTime(nextEvent->maxDuration);
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

    for (int i = 0; i < (expected.size() > actual.size() ? actual.size() : expected.size()); i++)
    {
        EXPECT_TRUE(expected[i] == actual[i]);
    }

}


/*
A   ***********
B                **********
*/
TEST_F(SchedulerTest, NoOverlapBefore)
{
    schedule2();

    deploymentSchedule[0].deploymentStartTime = 0;
    deploymentSchedule[0].ensembleDelay = 0;
    deploymentSchedule[0].maxDuration = 300;
    deploymentSchedule[0].ensembleInterval = 1000;

    // Task B starts at 500 ms
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

/*
A   ***********
B           **********
*/
TEST_F(SchedulerTest, OverlapBefore)
{
    schedule2();

    deploymentSchedule[0].deploymentStartTime = 0;
    deploymentSchedule[0].ensembleDelay = 250;
    deploymentSchedule[0].maxDuration = 300;
    deploymentSchedule[0].ensembleInterval = 1000;

    // Task B starts at 500 ms
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
TEST_F(SchedulerTest, NoOverlapAfter)
{
    schedule2();

    deploymentSchedule[0].deploymentStartTime = 0;
    deploymentSchedule[0].ensembleDelay = 800;
    deploymentSchedule[0].maxDuration = 300;
    deploymentSchedule[0].ensembleInterval = 1000;

    // Task B starts at 500 ms
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
/*
A                   ***********
B           **********
*/
TEST_F(SchedulerTest, OverlapAfter)
{
    schedule2();

    deploymentSchedule[0].deploymentStartTime = 0;
    deploymentSchedule[0].ensembleDelay = 650;
    deploymentSchedule[0].maxDuration = 300;
    deploymentSchedule[0].ensembleInterval = 1000;

    // Task B starts at 500 ms
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
A                   ***********
B           **********
*/
TEST_F(SchedulerTest, InsideOverlap)
{
    schedule2();

    deploymentSchedule[0].deploymentStartTime = 0;
    deploymentSchedule[0].ensembleDelay = 0;
    deploymentSchedule[0].maxDuration = 800;
    deploymentSchedule[0].ensembleInterval = 1000;

    // Task B starts at 500 ms
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
A  *********
B           **********
*/
TEST_F(SchedulerTest, Boundary1)
{
    schedule2();

    deploymentSchedule[0].deploymentStartTime = 0;
    deploymentSchedule[0].ensembleDelay = 0;
    deploymentSchedule[0].maxDuration = 500;
    deploymentSchedule[0].ensembleInterval = 1000;

    // Task B starts at 500 ms
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

/*
A                     *********
B           **********
*/
TEST_F(SchedulerTest, Boundary2)
{
    schedule2();

    deploymentSchedule[0].deploymentStartTime = 0;
    deploymentSchedule[0].ensembleDelay = 700;
    deploymentSchedule[0].maxDuration = 300;
    deploymentSchedule[0].ensembleInterval = 1000;

    // Task B starts at 500 ms
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