#include "ensembleTypes.hpp"

unsigned int Ens_getStartTime(system_tick_t sessionStart)
{
    return ((millis() - sessionStart) / 100) & 0x00FFFFFF;
}