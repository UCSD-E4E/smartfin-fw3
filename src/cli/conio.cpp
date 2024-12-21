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
#if SF_PLATFORM == SF_PLATFORM_GLIBC
#include <pthread.h>
#include <stdio.h>
#include <termios.h>
#elif SF_PLATFORM == SF_PLATFORM_PARTICLE
#include "Particle.h"
#endif

char SF_OSAL_printfBuffer[SF_OSAL_PRINTF_BUFLEN];
#if SF_PLATFORM == SF_PLATFORM_GLIBC
pthread_t read_thread, write_thread;
#define SF_OSAL_READ_BUFLEN 2048
char SF_OSAL_inputBuffer[SF_OSAL_READ_BUFLEN];
std::size_t read_head_idx = 0, read_tail_idx = 0;

void *read_loop(void *_)
{
    while (true)
    {
        SF_OSAL_inputBuffer[read_head_idx % SF_OSAL_READ_BUFLEN] = getchar();
        read_head_idx++;
    }
    return nullptr;
}
#endif

extern "C"
{
    // Determines if key has been pressed
    int SF_OSAL_kbhit(void)
    {
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
        return Serial.available();
#elif SF_PLATFORM == SF_PLATFORM_GLIBC
        return read_head_idx != read_tail_idx;
#endif
    }

    int SF_OSAL_getch(void)
    {
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
        while (Serial.available() == 0)
        {
            delay(1);
        }
        return Serial.read();
#elif SF_PLATFORM == SF_PLATFORM_GLIBC
        char retval = SF_OSAL_inputBuffer[read_tail_idx % SF_OSAL_READ_BUFLEN];
        read_tail_idx++;
        return retval;
#endif
    }

    // Write character
    int SF_OSAL_putch(int ch)
    {
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
        Serial.print((char)ch);
        return ch;
#elif SF_PLATFORM == SF_PLATFORM_GLIBC
        putchar(ch);
        fflush(stdout);
        return ch;
#endif
    }

    int SF_OSAL_getline(char *buffer, int buflen)
    {
        int i = 0;
        char userInput;

#if SF_PLATFORM == SF_PLATFORM_PARTICLE
        while (i < buflen)
        {
            Particle.process();
            if (SF_OSAL_kbhit())
            {
                userInput = SF_OSAL_getch();
                switch (userInput)
                {
                case '\b':
                    i--;
                    SF_OSAL_putch('\b');
                    SF_OSAL_putch(' ');
                    SF_OSAL_putch('\b');
                    break;
                default:
                    buffer[i++] = userInput;
                    SF_OSAL_putch(userInput);
                    break;
                case '\r':
                    buffer[i++] = 0;
                    SF_OSAL_putch('\r');
                    SF_OSAL_putch('\n');
                    return i;
                }
            }
        }
#elif SF_PLATFORM == SF_PLATFORM_GLIBC
        while (i < buflen)
        {
            if (SF_OSAL_kbhit())
            {
                userInput = SF_OSAL_getch();
                switch (userInput)
                {
                case '\b':
                    i--;
                    SF_OSAL_putch('\b');
                    SF_OSAL_putch(' ');
                    SF_OSAL_putch('\b');
                    break;
                default:
                    buffer[i++] = userInput;
                    SF_OSAL_putch(userInput);
                    break;
                case '\r':
                    buffer[i++] = 0;
                    SF_OSAL_putch('\r');
                    SF_OSAL_putch('\n');
                    return i;
                }
            }
        }
#endif
        return i;
    }
    
    // Print char array to terminal
    int SF_OSAL_printf(const char* fmt, ...) {
        va_list vargs;
        int nBytes = 0;
        va_start(vargs, fmt);
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
        nBytes = vsnprintf(SF_OSAL_printfBuffer, SF_OSAL_PRINTF_BUFLEN, fmt, vargs);
        Serial.write(SF_OSAL_printfBuffer);
#elif SF_PLATFORM == SF_PLATFORM_GLIBC
        vprintf(fmt, vargs);
        fflush(stdout);
#endif
        va_end(vargs);
        return nBytes;
    }

    void SF_OSAL_init_conio(void)
    {
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
        Serial.begin(SF_SERIAL_SPEED);
#elif SF_PLATFORM == SF_PLATFORM_GLIBC
        pthread_create(&read_thread, NULL, read_loop, NULL);
        struct termios term;
        tcgetattr(fileno(stdin), &term);

        term.c_lflag &= ~ECHO;
        tcsetattr(fileno(stdin), 0, &term);
#endif
    }

    void SF_OSAL_deinit_conio(void)
    {
#if SF_PLATFORM == SF_PLATFORM_GLIBC
#endif
    }
}