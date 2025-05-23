#ifndef __TASK_HPP__
#define __TASK_HPP__
#include "states.hpp"
/**
 * @brief Task interface which defines methods for basic structure of tasks to be implemented by other task classes.
 * 
 * specifies methods to initialize, run, and exit a task.
 * 
 */
class Task{
    public:
    /**
     * Initializes the task.  This assumes entry from any other state.  If this
     * fails, Task::run must handle switching to the appropriate task context.
     */
    virtual void init(void) = 0;

    /**
     * This should be the task body.  This should execute until a state change
     * needs to occur, in which case the state to change to should be returned.
     * Once the state to change to is returned, Task::exit will be called to
     * clean up from this state.
     * 
     * @returns STATES_e state to change to
     */
    virtual STATES_e run(void) = 0;

    /**
     * This should handle cleaning up any resources.  This will execute when
     * Task::run returns.
     */
    virtual void exit(void) = 0;
};
#endif
