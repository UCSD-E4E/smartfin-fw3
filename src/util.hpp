#ifndef __UTIL_HPP__
#define __UTIL_HPP__

/**
 * @brief Hexdump function, sends hex data of memory to serial
 * 
 * @param memoryLocation Memory location to check
 * @param buflen Length of memory buffer
 */
void hexDump(const void *memoryLocation, int buflen);


#endif
