/**
 * conio.cpp
 * 
 * Particle serial input and output for command line use
 * 
*/


#include "conio.hpp"

#include "Particle.h"

#include <cstdio>

char SF_OSAL_printfBuffer[SF_OSAL_PRINTF_BUFLEN];



extern "C"
{
    // Determines if key has been pressed
    int kbhit(void) 
    {
        return Serial.available();
    }

    // Get pressed key
    int getch(void)
    {
        while(Serial.available() == 0)
        {
            delay(1);
        }
        return Serial.read();
    }

    // Write character
    int putch(int ch)
    {
        char outputBuf[2] = {(char)ch, 0};
        // Serial.print(outputBuf);
        Serial.write((char) ch);
        return ch;
    }
    
    // Print char array to terminal
    int SF_OSAL_printf(const char* fmt, ...)
    {
        va_list vargs;
        int nBytes = 0;
        va_start(vargs, fmt);
        nBytes = vsnprintf(SF_OSAL_printfBuffer, SF_OSAL_PRINTF_BUFLEN, fmt, vargs);
        va_end(vargs);
        Serial.write(SF_OSAL_printfBuffer);
        return nBytes;
    }
}