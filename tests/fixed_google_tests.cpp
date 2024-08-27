/**
 * @file gtest.cpp
 * @author Antara Chugh (antarachugh@g.ucla.edu), Charlie Kushelevsky (ckushelevsky@ucsd.edu) 
 * @brief Google Tests for scheduler.cpp
 */

#include "scheduler_test_system.hpp"
#include "scheduler.hpp"
#include "test_ensembles.hpp"
#include "cli/flog.hpp"
#include "test_file_system.hpp"

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
/**
  * @brief constructor for tests, intializes default values
 */
    SchedulerFixedTests()
    {
        table_default_interval = 75;
        table_default_duration = 25;
        clock = 0;

        deployment_table = {
            //EnsembleFunc,    init,           meas, ensembleInterval,        maxDuration,              maxDelay, taskName, state
            {SS_ensembleAFunc, SS_ensembleAInit, 1, table_default_interval, table_default_duration, UINT32_MAX, "A", {0}},
            {SS_ensembleBFunc, SS_ensembleBInit, 1, table_default_interval, table_default_duration, UINT32_MAX, "B", {0}},
            {SS_ensembleCFunc, SS_ensembleCInit, 1, table_default_interval, table_default_duration, UINT32_MAX, "C", {0}},
            {nullptr,           nullptr,          0, 0,                       0,                        0,         nullptr, {0}}
        };
        test_log = {};


    };
    
    /*Every test will require a deployment table, we fill it with defaults. 
    Clock corresponds to millis(), record scheduler output in test log*/
    uint32_t table_default_interval;
   
    uint32_t table_default_duration;
   
    
    uint32_t clock;


    std::vector<DeploymentSchedule_> deployment_table;
    std::vector<Log> test_log;



    /*Before every test, set the deployment table to 
    default values, clock to 0, and empty test log*/
    void SetUp() override {
        deployment_table[0].ensembleInterval = table_default_interval;
        deployment_table[1].ensembleInterval = table_default_interval;
        deployment_table[2].ensembleInterval = table_default_interval;

        deployment_table[0].maxDuration = table_default_duration;
        deployment_table[1].maxDuration = table_default_duration;
        deployment_table[2].maxDuration = table_default_duration;

        test_log.clear();
        clock = 0;





    }
    void TearDown() override {
        test_log.clear();

    }

    /*functions to change the intervals or durations of tasks, add a task. changes are made in test function prior to running*/

    void three_task_change_intervals(std::uint32_t A, std::uint32_t B, std::uint32_t C) {
        deployment_table[0].ensembleInterval = A;
        deployment_table[1].ensembleInterval = B;
        deployment_table[2].ensembleInterval = C;
    }


    void two_task_change_intervals(std::uint32_t A, std::uint32_t B) {
        deployment_table[0].ensembleInterval = A;
        deployment_table[1].ensembleInterval = B;
    }

    void change_interval(std::uint32_t task, std::uint32_t interval) {
        deployment_table[task].ensembleInterval = interval;
    }
    void change_duratiom(std::uint32_t task, std::uint32_t duration) {
        deployment_table[task].maxDuration = duration;
    }
   



    /*clock functions -- updates clock when task is run, tick clock*/
    void update_clock_time(DeploymentSchedule_* task, std::uint32_t delay) {
        if (task != nullptr)
        {
            this->clock += task->maxDuration;
        }
        if (delay != 0)
        {
            this->clock += delay;
        }
    }


    void clock_increment() {
        this->clock++;
    }
    //! compares the test log with the expected values, as set in the test case
    void compare(std::vector<Log>& expected, int iterations) {
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
     * @param iterations number of scheduler calls
     * @param task_delay task_delay
     * creates scheduler based on tasks set in test, compares scheduler
     * behaviour with expected behaviour
    */

    void run(int num_tasks, int iterations, int task_delay, int delay_amount,
                    std::vector<Log>& expected) {
        
        DeploymentSchedule_t table[num_tasks+1];
        for(int i=0; i<deployment_table.size()-1; i++){
            table[i]=deployment_table[i];
        }
        table[num_tasks]= {nullptr,           nullptr,          0, 0,                       0,                        0,         nullptr, {0}};
        Scheduler scheduler(table);
        scheduler.initializeScheduler();
        std::uint32_t nextTime = 0;
        std::uint32_t* nextTaskTime = &(nextTime);

        DeploymentSchedule_* nextTask = nullptr;

        for (int i = 0; i <iterations; i++)
        {
            int delay = 0;
            if (i == task_delay)
            {
                delay = delay_amount;

            }

            scheduler.getNextTask(&nextTask, nextTaskTime, this->clock);
            this->clock = *nextTaskTime;

            test_log.emplace_back(Log(nextTask, clock));
            update_clock_time(nextTask, delay);
            DeploymentSchedule_* t = nextTask;
            
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

TEST_F(SchedulerFixedTests, TestDefaultWithDelays)
{
    //test A is on time even with delays
    int Delay[6]={0, 25, 0, 100, 0, 0};
    
    
}



