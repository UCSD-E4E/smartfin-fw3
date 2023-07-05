#include "cli.hpp"
#include "states.hpp"
#include "Particle.h"


typedef const struct CLI_menu_
{
    String cmd;
    String fnName; // menu function name
    void (*fn)(void); // linked menu function
} CLI_menu_t;

static STATES_e CLI_nextState;

void CLI::init(void) {
    
}