#ifndef __UTIL_HPP__
#define __UTIL_HPP__
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Hexdump function, sends hex data of memory to serial
 * 
 * @param memoryLocation Memory location to check
 * @param buflen Length of memory buffer
 */
void hexDump(const void *memoryLocation, size_t buflen);

#define N_TO_B_ENDIAN_2(x)  ((((uint16_t)(x) & 0x00FF) << 8) | (((uint16_t)(x) & 0xFF00) >> 8))
#define N_TO_B_ENDIAN_4(x)  ((((uint32_t)(x) & 0x000000FF) << 24) | (((uint32_t)(x) & 0x0000FF00) << 8) | (((uint32_t)(x) & 0x00FF0000) >> 8) | (((uint32_t)(x) & 0xFF000000) >> 24))

#define B_TO_N_ENDIAN_2(x)  N_TO_B_ENDIAN_2(x)
#define B_TO_N_ENDIAN_4(x)  N_TO_B_ENDIAN_4(x)

namespace SF::utils
{
    /**
     * @brief Generates a uniform random number in the given range
     *
     * @param min Minimum value (inclusive)
     * @param max Maximum value (exclusive)
     * @return int random integer
     */
    int random(int min, int max);
}; // namespace SF::utils
#endif
