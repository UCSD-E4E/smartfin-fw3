#include "conio.hpp"

#include "Particle.h"

#include <cstdio>


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

    // Write character on screen
    int putch(int ch)
    {
        char outputBuf[2] = {(char)ch, 0};
        // Serial.print(outputBuf);
        Serial.write((char) ch);
        return ch;
    }
    



}