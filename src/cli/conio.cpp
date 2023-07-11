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
        Serial.print((char) ch);
        return ch;
    }
    
    // Print char array to terminal
    int SF_OSAL_printf(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        
        int printed_chars = 0;

        while (*fmt != '\0') {
            if (*fmt == '%') {
                // Handle format specifier
                fmt++;

                // Process the format specifier based on its type
                switch (*fmt) {
                    case 'd': {
                        int arg = va_arg(args, int);
                        printed_chars += Serial.write(arg);
                        break;
                    }
                    case 'f': {
                        double arg = va_arg(args, double);
                        printed_chars += Serial.write(arg);
                        break;
                    }
                    case 's': {
                        char* arg = va_arg(args, char*);
                        printed_chars += Serial.write(arg);
                        break;
                    }
                    // Add more format specifiers as needed

                    default:
                        // Invalid format specifier, handle error or ignore it
                        break;
                }
            } else {
                // Handle regular character
                printed_chars += Serial.write(*fmt);
            }

            fmt++;
        }

        va_end(args);
        
        return 1;
    }
}