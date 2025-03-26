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
#include "conioHistory.hpp"
#include <ncurses.h>
#include <pthread.h>
#elif SF_PLATFORM == SF_PLATFORM_PARTICLE
#include "Particle.h"
#endif

char SF_OSAL_printfBuffer[SF_OSAL_PRINTF_BUFLEN];
#if SF_PLATFORM == SF_PLATFORM_GLIBC
pthread_t read_thread, write_thread;
#define SF_OSAL_READ_BUFLEN 2048
char SF_OSAL_inputBuffer[SF_OSAL_READ_BUFLEN];
std::size_t read_head_idx = 0, read_tail_idx = 0;
pthread_mutex_t read_mutex;
std::string file_buf;
std::size_t wind_h, wind_w;

void *read_loop(void *_)
{
    int ch;
    while (true)
    {
        ch = wgetch(stdscr);
        pthread_mutex_lock(&read_mutex);
        SF_OSAL_inputBuffer[read_head_idx % SF_OSAL_READ_BUFLEN] = ch;
        read_head_idx++;
        pthread_mutex_unlock(&read_mutex);
    }
    return nullptr;
}
#endif

extern "C"
{
    // Determines if key has been pressed
    int SF_OSAL_kbhit(void)
    {
        int rv;
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
        return Serial.available();
#elif SF_PLATFORM == SF_PLATFORM_GLIBC
        pthread_mutex_lock(&read_mutex);
        rv = (read_head_idx != read_tail_idx);
        pthread_mutex_unlock(&read_mutex);
        return rv;
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
        pthread_mutex_lock(&read_mutex);
        char retval = SF_OSAL_inputBuffer[read_tail_idx % SF_OSAL_READ_BUFLEN];
        read_tail_idx++;
        pthread_mutex_unlock(&read_mutex);
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
        waddch(stdscr, ch);
        wrefresh(stdscr);
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
                    if (i > 0)
                    {
                        i--;
                        SF_OSAL_putch('\b');
                        SF_OSAL_putch(' ');
                        SF_OSAL_putch('\b');
                    }
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
        file_buf = "";
        while (i < buflen)
        {
            if (SF_OSAL_kbhit())
            {
                userInput = SF_OSAL_getch();

                // Check only for UP and DOWN scroll (^[A and ^[B) first
                if (userInput == 27)
                {
                    while (!SF_OSAL_kbhit())
                    {
                    } // Wait for next byte
                    char inp = SF_OSAL_getch();
                    if (inp != '[')
                        break;
                    while (!SF_OSAL_kbhit())
                    {
                    }
                    inp = SF_OSAL_getch();

                    if (bottom_idx <= wind_h) // No need to scroll
                    {
                        break;
                    }
                    switch (inp)
                    {
                        case 'A':
                        {
                            size_t top_idx = cur_bottom - wind_h + 1;
                            if (top_idx == 0) // Already at top
                            {
                                break;
                            }
                            wscrl(stdscr, -1);
                            curs_set(0);
                            move(0, 0);
                            wrefresh(stdscr);
                            cur_bottom--;

                            char *line = retrieve_line(--top_idx);
                            if (!line)
                            {
                                break;
                            }
                            wprintw(stdscr, "%s", line);
                            wrefresh(stdscr);
                            free(line);
                            break;
                        }
                        case 'B':
                        {
                            if (cur_bottom == bottom_idx) // Already at bottom
                            {
                                break;
                            }
                            wscrl(stdscr, 1);
                            curs_set(0);
                            move(wind_h - 1, 0);
                            wrefresh(stdscr);
                            cur_bottom++;

                            char *line = retrieve_line(cur_bottom);
                            if (!line)
                            {
                                break;
                            }
                            wprintw(stdscr, "%s", line);
                            wrefresh(stdscr);
                            free(line);
                            break;
                        }
                    }
                }
                else
                {
                    switch (userInput)
                    {
                        case 127:
                        case '\b':
                            if (i > 0)
                            {
                                i--;
                                SF_OSAL_putch('\b');
                                SF_OSAL_putch(' ');
                                SF_OSAL_putch('\b');
                                file_buf.pop_back();
                            }
                            break;
                        default:
                            buffer[i++] = userInput;
                            SF_OSAL_putch(userInput);
                            file_buf += userInput;
                            break;
                        case '\n':
                            buffer[i++] = 0;
                            SF_OSAL_putch('\n');
                            write_line(file_buf, true);
                            return i;
                    }
                }
            }
        }
        // Exceeded buffer is automatically entered as command
        SF_OSAL_putch('\n');
        write_line(file_buf, true);
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
        int size = vsnprintf(nullptr, 0, fmt, vargs);
        va_end(vargs);

        va_start(vargs, fmt);
        std::string formatted(size + 1, '\0');
        vsnprintf(&formatted[0], size + 1, fmt, vargs);
        va_end(vargs);

        formatted.pop_back(); // remove the null terminator
        wprintw(stdscr, "%s", formatted.c_str());

        size_t start = 0, end;
        while ((end = formatted.find('\n', start)) != std::string::npos)
        {
            write_line(formatted.substr(start, end - start), true); // Extract line
            start = end + 1; // Move past '\n'
        }
        write_line(formatted.substr(start), false);
        wrefresh(stdscr);
#endif
        va_end(vargs);
        return nBytes;
    }

    void SF_OSAL_init_conio(void)
    {
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
        Serial.begin(SF_SERIAL_SPEED);
#elif SF_PLATFORM == SF_PLATFORM_GLIBC
        initscr();
        noecho();
        scrollok(stdscr, TRUE);
        immedok(stdscr, TRUE);

        init_file_mapping();
        getmaxyx(stdscr, wind_h, wind_w);

        pthread_create(&read_thread, NULL, read_loop, NULL);
        pthread_detach(read_thread);
#endif
    }

    void SF_OSAL_deinit_conio(void)
    {
#if SF_PLATFORM == SF_PLATFORM_GLIBC
        write_line(file_buf, false);
        deinit_file_mapping();
        endwin();
#endif
    }
}