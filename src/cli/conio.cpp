/**
 * Project smartfin-fw3
 * @file conio.hpp
 * Description: Particle serial input and output for command line use
 * @author Emily B. Thorpe <ethorpe@macalester.edu>
 * @date Jul 20 2023
 */

#include "conio.hpp"

#include "product.hpp"

#include <string>
#include <cstdio>

#include "Particle.h"


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
        while (Serial.available() == 0)
        {
            delay(1);
        }
        return Serial.read();
    }

    // Write character
    int putch(int ch)
    {
        Serial.print((char) ch);
        return ch;
    }

    int getline(char* buffer, int buflen)
    {
        int i = 0;
        char userInput;

        while (i < buflen)
        {
            Particle.process();
            if (kbhit())
            {
                userInput = getch();
                switch(userInput)
                {
                    case '\b':
                        i--;
                        putch('\b');
                        putch(' ');
                        putch('\b');
                        break;
                    default:
                        buffer[i++] = userInput;
                        putch(userInput);
                        break;
                    case '\r':
                        buffer[i++] = 0;
                        putch('\r');
                        putch('\n');
                        return i;
                }
            }
        }
        return i;
    }
    
    // Print char array to terminal
    int SF_OSAL_printf(const char* fmt, ...) {
        va_list vargs;
        int nBytes = 0;
        va_start(vargs, fmt);
        nBytes = vsnprintf(SF_OSAL_printfBuffer, SF_OSAL_PRINTF_BUFLEN, fmt, vargs);
        va_end(vargs);
        Serial.write(SF_OSAL_printfBuffer);
        return nBytes;
    }
}