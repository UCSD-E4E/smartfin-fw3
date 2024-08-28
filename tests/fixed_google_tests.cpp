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
            //{nullptr,           nullptr,          0, 0,                       0,                        0,         nullptr, {0}}
        };
        test_log = {};


    };
    
    uint32_t table_default_interval;
   
    uint32_t table_default_duration;
   
    
    uint32_t clock;


    std::vector<DeploymentSchedule_> deployment_table;
    std::vector<Log> test_log;



    /**
 * @brief SetUp, runs before every test. sets table back to defaults, clock back
 * to 0, and ensures test log is empty
    */
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
    /**
 * @brief TearDown, runs after every test. Clears test log. 
    */
    void TearDown() override {
        test_log.clear();

    }

    /**
     * @param A_interval desired interval of task A, highest priority task
     * @param B_interval desired interval of task B
     * @param C_interval desired interval of task C, lowest prioirity task
     * Updates the intervals of 3 tasks to desired intervals for a test case. 
     * If it is desired to change interval from default, call this function
     * before calling "run()"
    */

    void three_task_change_intervals(std::uint32_t A_interval, std::uint32_t B_interval, std::uint32_t C_interval) {
        deployment_table[0].ensembleInterval = A_interval;
        deployment_table[1].ensembleInterval = B_interval;
        deployment_table[2].ensembleInterval = C_interval;
    }
    
    /**
     * @param A_interval desired interval of task A, highest priority task
     * @param B_interval desired interval of task B
     * Updates the intervals of 2 tasks to desired intervals for a test case. 
     * If it is desired to change interval from default, call this function
     * before calling "run()"
    */

    void two_task_change_intervals(std::uint32_t A_interval, std::uint32_t B_interval) {
        deployment_table[0].ensembleInterval = A_interval;
        deployment_table[1].ensembleInterval = B_interval;
    }

    /**
     * @param task priority of task whose interval is desired to change. 
     * 0=highest priority. 
     * @param interval desired interval of task 
     * Updates the interval of the specified task
    */

    void change_interval(std::uint32_t task, std::uint32_t interval) {
        deployment_table[task].ensembleInterval = interval;
    }
    /**
     * @param task priority of task whose duration is desired to change. 
     * 0=highest priority. 
     * @param duration desired length of task 
     * Updates the duration of the specified task
    */

    void change_duratiom(std::uint32_t task, std::uint32_t duration) {
        deployment_table[task].maxDuration = duration;
    }
     /**
     * @param task_name name of task. 
     * @param duration desired length of task 
     * @param interval desired interval of task
     * Adds a task with task_name and specificed duration & interval as lowest 
     * priority task in deployment table.
    */

    void add_task(const char * task_name, std::uint32_t duration, std::uint32_t interval){

        DeploymentSchedule_ task=  {SS_ensembleCFunc, SS_ensembleCInit, 1, 
        interval, duration, UINT32_MAX, task_name, {0}};
        deployment_table.emplace_back(task);


    }
   



    /**
     * @param task task that is running at current time
     * @param delay delay added to task duration, task runs over expected length
     * updates clock when a task runs by the task's max duration and any delay
     * if present
    */
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


   
     /**
     * @param expected takes expected behavior of scheduler, consisting of 
     * taskname & when it runs
     * @param iterations number of scheduler calls
     * compares scheduler behaviour with expected behviour for iteration number 
     *of calls to the scheduler
    */
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
     * @param task_delay array of delays to add a task that runs
     * @param expected takes expected behavior of scheduler, consisting of 
     * taskname & when it runs
     * creates scheduler based on tasks set in test, compares scheduler
     * behaviour with expected behaviour
    */

    void run(int num_tasks, int iterations, int* task_delay,
                    std::vector<Log>& expected) {
        
        DeploymentSchedule_t table[num_tasks+1];
        if(num_tasks>deployment_table.size()){
            return;
        }
        for(int i=0; i<num_tasks; i++){
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
            delay=task_delay[i];

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
    int Delay[6]={0,0,0,0,0,0};
    run(3, 6, Delay, expected);
}

TEST_F(SchedulerFixedTests, TestDefaultWithDelays)
{
    //test A is on time even with delays
    int Delay[6]={0, 25, 0, 0, 25, 0};
     std::vector<Log> expected;
    expected.emplace_back("A", 0);
    expected.emplace_back("B", 25);
    expected.emplace_back("A", 75);
    expected.emplace_back("B", 100);
    expected.emplace_back("C", 125);
    expected.emplace_back("A", 175);
     run(3, 6, Delay, expected);
}
    
    




