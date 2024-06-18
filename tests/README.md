# Overview
The folder `tests` contains tests monatomic tests for the scheduler in the `src` directory. There is a bash script to manage the tests at `tests/run_test.sh`. 

# File Descriptions
## `tests/run_test.sh` 
This should be used to run all test cases. ithout any inputs the script runs two cases, a scheduler with no delays, and a scheduler with delays that have added that demostrate the three different cases for late ensembles. If you want to use custom delays, you can pass them as arguments at the end of run_test.sh. If you do so, it must be preceded by the number of ensembles to run with `-n <ensembles to run>` and optionally a title for the gantt chart with `-t <title>`. The gantt chartt is saved to `tests/outputs`.


##  `scheduler_test.cpp`
This is equivelant to the `src/rideTask.cpp` in the main project. This is where the ensembles are defined and where the scheduler is called from.

## `tests/scheduler_test_system.hpp` and `tests/scheduler_test_system.cpp`
This is where functions that are necessary for the scheduler to run are redefined from the particle library.

# Test case
There are 3 ensembles: A, B, and C that run every 2000 seconds. A takes 400 ms to run, B takes 200ms, C takes 600ms to run.

Ideal case (0ms-2000ms):
A (0ms-400ms)
B (500ms-700ms)
C (800ms-1400ms)

Delay case 1 (2000ms-6000ms):  task A takes 200 ms longer than expected so task B that was supposed to run at 2500 ms is run immediately as it will not overlap with the next task (task C)
A (2000ms-2600ms)
B (2600ms-2800ms)
C (2800ms-3400ms)

Delay case 2 (4000ms-8000ms):  task C that starts at 4800 ms takes 800 ms longer so the task A that was supposed to run at 6000 ms is run at next available idle time as it would conflict with task B at 6500ms.
A (4000ms-4600ms)
B (4500ms-4700ms)
C (4800ms-6200ms)

B (6500ms-6700ms)
C (6800ms-7400ms)
A (7400ms-7800ms)

Delay case 3 (8000ms-12000ms):  task B that starts at 8500 ms takes 800 ms longer so the task C that was supposed to run at 8700 ms is not run as there is no idle time before task C is scheduled to run next.
A(8000ms-8400ms)
B(8500ms-9500ms)
A(10000ms-10400ms)
B(10500ms-10700ms)
C(10800ms-11400ms)