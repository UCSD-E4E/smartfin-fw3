# Overview
    The following document outlines how to add tests to the fixed_google_tests file. The purpose of fixed_google_test is to ensure the scheduler behaves as expected on a fixed set of test cases. "expected behavior" can be defined as the result of the algorithm outlined in control_flow.uml.

# Adding Tests
    The Scheduler Test class handles all the logic and functions needed to run & create a test. A test case is essentially an instance of the Scheduler Test class. Before every test, the SetUp() function is called, intializes a Deployment Table of 3 tasks with duration of 25 seconds and interval of 75 seconds; sets the clock to 0; and sets the test log to empty. Everytime a task is run, the name of the task and the time it ran is logged in test log. After a test case is done running, the TearDown() function is called, clearing the test log. These functions are called automatically. 

    Every test requires the deployment table to be set up with the desired tasks, the number of times the scheduler should be called (iterations), a vector of expected values of length iterations, and an int array of length iteration called Delay, which specifies how much delay to inject to a duration of a task. Delay[i] corresponds to adding a delay of length delay[i] to the ith  task the scheduler runs. (so the ith task will run for task.duration + delay[i])
    
    IMPORTANT: The creator of the test case must hard code the expected behaviour log with the approriate behaviour given the injected delays. 

    Once this is set up, call run(number of tasks, iterations,  delay, expected). This will call the scheduler's getNextTask function for iterations number of times, logging the task name and when the task ran. This will then compare the result of the scheduler to the expected log provided by the test case and throw an exception if the behaviour is mismatched.

    TEST_F(Scheduler_Test, /*Test Name*/){
        /*Set up Deployment Table */
        /* Set up delay log */
         /* Set up expected log */
        run(numtasks, iterations, delay, expected);

    }
    

# Setting up the Deployment Table 

    The set of tasks to test are stored in a vector of DeploymentTables, called deployment_table. This contains all nesecary info of the tasks. To set up the desired set of tasks to test, the changeDuration and changeInterval function can be used to change the duration and interval respectively of the ith priority task, where the 0th prioirity task is defined as the highest priority task. The functions change3intervals, change2intervals, and duration change the intervals of the first 3 or 2 priority tasks respectively, this is for ease of the test creator. If more than 3 tasks are to be tested, additional tasks can be added using the addTask function, which takes the task name, desired interval & duration of the task. 





