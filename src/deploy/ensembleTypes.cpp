#include "ensembleTypes.hpp"

system_tick_t sessionStart = 0;
unsigned int Ens_getStartTime(void)
{
    return ((millis() - sessionStart) / 100) & 0x00FFFFFF;
}
void Ens_setStartTime(void)
{
    sessionStart = millis();
}