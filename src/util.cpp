/**
 * @file util.cpp
 * @author Emily Thorpe (ethorpe@macalster.edu)
 * @brief 
 * @version 0.1
 * @date 2023-07-26
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "util.hpp"

#include "cli/conio.hpp"
#include "consts.hpp"

#include "Particle.h"

/**
 * \brief A macro that stores the the size of each line of output as a constant 16 bytes
 */
#define BYTES_PER_LINE 16

void hexDump(const void *memoryLocation, size_t buflen)
{
    size_t buffer_idx = 0;
    uint8_t byte_buffer[BYTES_PER_LINE + 1];
    const uint8_t* data_window = (const uint8_t*) memoryLocation;
    int bytes_read = 0;
    size_t line_idx = 0;

    for(buffer_idx = 0; buffer_idx < buflen; buffer_idx += bytes_read)
    {
        SF_OSAL_printf("%08x  ", buffer_idx);
        memset(byte_buffer, 0, BYTES_PER_LINE + 1);
        bytes_read = ((buffer_idx + BYTES_PER_LINE) < buflen) ? BYTES_PER_LINE : buflen - buffer_idx;
        memcpy(byte_buffer, data_window + buffer_idx, bytes_read);

        // Print hex
        for (line_idx = 0; line_idx < (size_t) bytes_read; line_idx++)
        {
            SF_OSAL_printf("%02hx ", byte_buffer[line_idx]);
            if (7 == line_idx)
            {
                SF_OSAL_printf(" ");
            }
            // Make the byte printable
            if (!isprint(byte_buffer[line_idx]))
            {
                byte_buffer[line_idx] = (uint8_t)'.';
            }
        }
        // fill line
        for (;line_idx < BYTES_PER_LINE; line_idx++)
        {
            SF_OSAL_printf("   ");
            if (7 == line_idx)
            {
                SF_OSAL_printf(" ");
            }
        }
        SF_OSAL_printf(" |%s|" __NL__, (const char*)byte_buffer);
    }
    SF_OSAL_printf("%08x" __NL__, buffer_idx);
}