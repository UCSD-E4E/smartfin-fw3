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
    /*Constructor: sets up values*/

    SchedulerFixedTests()
    {
        table_default_interval_A = 75;
        table_default_interval_B = 75;
        table_default_interval_C = 75;
        table_default_duration_A = 25;
        table_default_duration_B = 25;
        table_default_duration_C = 25;
        clock = 0;

        deployment_table = {
                                {   SS_ensembleAFunc, //measure
                                    SS_ensembleAInit, //init
                                    1, // measurementsToAccumulate

                                    table_default_interval_A, //ensembleInterval
                                    table_default_duration_A, //maxDuration

                                    UINT32_MAX, //maxDelay
                                    "A", //taskName
                                    {0} //state
                                    },

        {SS_ensembleBFunc, SS_ensembleBInit, 1,  table_default_interval_B, table_default_duration_B, UINT32_MAX, "B", {0}},
        {SS_ensembleCFunc, SS_ensembleCInit, 1, table_default_interval_C, table_default_duration_C,  UINT32_MAX, "C", {0}},
        {nullptr,           nullptr,          0, 0,                        0,                         0,         nullptr, {0}}
        };
        test_log = {};


    };
    /*every test will require a deployment table, we fill it with defaults. Clock corresponds to millis(), record scheduler output in test log*/
    uint32_t table_default_interval_A;
    uint32_t table_default_interval_B;
    uint32_t table_default_interval_C;
    uint32_t table_default_duration_A;
    uint32_t table_default_duration_B;
    uint32_t table_default_duration_C;
    uint32_t clock;


    std::vector<DeploymentSchedule_> deployment_table;
    std::vector<Log> test_log;



    /*Before every test, set the deployment table to default values, clock to 0, and empty test log*/
    void SetUp() override {
        deployment_table[0].ensembleInterval = table_default_interval_A;
        deployment_table[1].ensembleInterval = table_default_interval_B;
        deployment_table[2].ensembleInterval = table_default_interval_C;

        deployment_table[0].maxDuration = table_default_duration_A;
        deployment_table[1].maxDuration = table_default_duration_B;
        deployment_table[2].maxDuration = table_default_duration_C;

        test_log.clear();
        clock = 0;





    }
    void TearDown() override {
        test_log.clear();

    }

    /*functions to change the intervals or durations of tasks, add a task. changes are made in test function prior to running*/

    void three_task_change_intervals(uint32_t A, uint32_t B, uint32_t C) {
        deployment_table[0].ensembleInterval = A;
        deployment_table[1].ensembleInterval = B;
        deployment_table[2].ensembleInterval = C;
    }


    void two_task_change_intervals(uint32_t A, uint32_t B) {
        deployment_table[0].ensembleInterval = A;
        deployment_table[1].ensembleInterval = B;
    }

    void change_interval(uint32_t task, uint32_t interval) {
        deployment_table[task].ensembleInterval = interval;
    }
    void change_duratiom(uint32_t task, uint32_t duration) {
        deployment_table[task].maxDuration = duration;
    }



    /*clock functions -- updates clock when task is run, tick clock*/
    void update_clock_time(DeploymentSchedule_* task, uint32_t delay) {
        if (task != nullptr)        
{
            clock += task->maxDuration;
        }
        if (delay != 0)
        {
            clock += delay;
        }


    }


    void clock_increment() {
        clock++;
    }
    //! compares the test log with the expected values, as set in the test case
    void compare(std::vector<Log> expected, int iterations) {
        int size = test_log.size() < expected.size() ? 
                            test_log.size():expected.size();
        for (int i = 0; i < iterations; i++)        
{
            EXPECT_TRUE(test_log[i] == expected[i]) << "test log failed:\n"
                << "Expected \t Actual\n"
                << expected[i].getName() << "\t\t" 
                << test_log[i].getName() << "\n"
                << expected[i].getRunTime() << "\t\t" 
                << test_log[i].getRunTime();
        }
    }

    /**
     * @param num_tasks number of tasks
     * creates scheduler based on tasks set in test, compares scheduler
     * behaviour with expected behaviour
    */

    void run(int num_tasks, int iterations, int task_delay, int delay_amount,
                    std::vector<Log>& expected) {



        Scheduler scheduler(deployment_table.data());
        scheduler.initializeScheduler();
        std::uint32_t nextTime = 0;
        std::uint32_t* nextTaskTime = &(nextTime);

        DeploymentSchedule_* nextTask = nullptr;

        for (int i = 0; i <= iterations; i++)
        {
            int delay = 0;
            if (i == task_delay)
            {
                delay = delay_amount;

            }

            scheduler.getNextTask(&nextTask, nextTaskTime, this->clock);
            clock = *nextTaskTime;
            
            test_log.emplace_back(Log(nextTask, clock));
            update_clock_time(nextTask, delay);
            DeploymentSchedule_* t = nextTask;
            std::cout << t->taskName << "time: " << clock << std::endl;

        }
        compare(expected, iterations);

    }

};

TEST_F(SchedulerFixedTests, TestDefault)
{

    std::vector<Log> expected;
    expected.emplace_back("A", 0);
    expected.emplace_back("B", 25);
    expected.emplace_back("C", 50);
    expected.emplace_back("A", 75);
    expected.emplace_back("B", 100);
    expected.emplace_back("C", 125);
    run(3, 6, 0, 0, expected);
}



