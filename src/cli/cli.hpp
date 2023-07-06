#ifndef __CLI_HPP__
#define __CLI_HPP__

#include "task.hpp"


typedef const struct CLI_menu_
{
    char* cmd;
    const char *const fnName; // menu function name
    void (*fn)(void); // linked menu function
} CLI_menu_t;

class CLI : public Task {
    public:
    void init(void);
    STATES_e run(void);
    void exit(void);
    CLI_menu_t* CLI_findCommand(char *cmd);
};


#endif