#ifndef __CLI_HPP__
#define __CLI_HPP__

#include "task.hpp"


#define CLI_RGB_LED_COLOR       SF_CLI_RGB_LED_COLOR
#define CLI_RGB_LED_PATTERN     SF_CLI_RGB_LED_PATTERN
#define CLI_RGB_LED_PERIOD      SF_CLI_RGB_LED_PERIOD
#define CLI_RGB_LED_PRIORITY    SF_CLI_RGB_LED_PRIORITY


/**
 * @brief CLI Menu Entry
 * 
 */
typedef const struct CLI_menu_
{
    /**
     * @brief Command ID
     * 
     */
    const int cmd;
    /**
     * @brief Command Description/Name
     * 
     */
    const char *const fnName;
    /**
     * @brief Command function
     * 
     */
    void (*fn)(void);
} CLI_menu_t;

/**
 * @brief CLI Task
 * 
 */
class CLI : public Task 
{
    public:
        /** Initialization function for CLI task
        */
        void init(void);
        /** CLI main 'thread', run repeatadly during task
         * Manages user input and runs commands
         * @return next state 
        */
        STATES_e run(void);
        /** Exit function for CLI task
        */
        void exit(void);
        /** Finds the corresponding command (the function to run) from the command name
         * @param cmd string command to find 
         * @return command function if found, nullptr otherwise
        */
        CLI_menu_t* CLI_findCommand(const char *cmd);
};

extern STATES_e CLI_nextState;

/**
 * @brief State to go to after CLI
 * Used for changing state within CLI
 * 
 */
extern STATES_e CLI_nextState;

#endif
