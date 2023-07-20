#ifndef __CLI_HPP__
#define __CLI_HPP__

#include "task.hpp"


typedef const struct CLI_menu_
{
    const char* cmd;
    const char *const fnName; // menu function name
    void (*fn)(void); // linked menu function
} CLI_menu_t;

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


#endif
