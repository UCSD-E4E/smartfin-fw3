/**
 * @file task.hpp
 * @author Charlie Kushulevsky (ckushelevsky@ucsd.edu)
 * @author Nathan Hui (nthui@ucsd.edu)
 * @brief
 * @version 0.1
 * @date 2024-10-31
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef __TASK_HPP__
#define __TASK_HPP__
#include "states.hpp"
/**
 * @brief Abstract Base Class for Tasks
 *
 * This task is the effective abstraction of a state in a state machine.
 *
 */
class Task {
public:
    /**
     * @brief Initializes the task.  This assumes entry from any other state.  
     * If this fails, Task::run must handle switching to the appropriate task 
     * context.
     */
    virtual void init(void) = 0;

    /**
     * @brief State logic body.  This should execute until a state change
     * needs to occur, in which case the state to change to should be returned.
     * Once the state to change to is returned, Task::exit will be called to
     * clean up from this state.
     * @return the state to change to
     */
    virtual STATES_e run(void) = 0;

    /**
     * @brief This should handle cleaning up any resources.  
     * This will execute when Task::run returns.
     */
    virtual void exit(void) = 0;
};
#endif
