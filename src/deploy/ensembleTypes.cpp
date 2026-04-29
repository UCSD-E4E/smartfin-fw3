#include "ensembleTypes.hpp"

#include "platform/hal.hpp"
SF_HAL::tick_t sessionStart = 0;
unsigned int Ens_getStartTime(void)
{
    return ((SF_HAL::millis() - sessionStart) / 100) & 0x000FFFFF;
}
void Ens_setStartTime(void)
{
    sessionStart = SF_HAL::millis();
}
