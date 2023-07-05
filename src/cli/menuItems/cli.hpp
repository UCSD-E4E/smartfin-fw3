#ifndef __CLI_HPP__
#define __CLI_HPP__

#include "task.hpp"

class CLI : public Task {
    public:
    void init(void);
    STATES_e run(void);
    void exit(void);
};

#endif