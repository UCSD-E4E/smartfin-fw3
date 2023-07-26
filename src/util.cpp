#include "Particle.h"
#include "util.hpp"
#include "cli/conio.hpp"

void hexDump(const void *memoryLocation, int buflen) 
{
    const uint8_t *memLoc = (const uint8_t*) memoryLocation;

    const size_t rowLen = 32;

    if (buflen % 4 != 0) {
        SF_OSAL_printf("non-valid length");
        return;
    }

    
    for (int i = 0; i < buflen; i+=rowLen)
    {
        SF_OSAL_printf("%04x: ", i);
        for (uint16_t j = 0; j < rowLen; j+=2)
        {
            SF_OSAL_printf("%02x%02x ", memLoc[i], memLoc[i + 1]);
        }
        SF_OSAL_printf("\n");
    }
}