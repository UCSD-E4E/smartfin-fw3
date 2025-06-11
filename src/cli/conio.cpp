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
#include <cstring>
#elif SF_PLATFORM == SF_PLATFORM_PARTICLE
#include "Particle.h"
#include "system.hpp"
#endif

char SF_OSAL_printfBuffer[SF_OSAL_PRINTF_BUFLEN];
#if SF_PLATFORM == SF_PLATFORM_GLIBC
/**
 * @brief Simplify syntax for conioHistory singleton
 */
#define conioHistory conioHistory::getInstance()
/**
 * @brief Separate thread for reading in keyboard input
 * 
 */
pthread_t read_thread;
/**
 * @brief Length for read buffer
 * 
 */
#define SF_OSAL_READ_BUFLEN 2048
/**
 * @brief Buffer for keyboard input
 * 
 */
int SF_OSAL_inputBuffer[SF_OSAL_READ_BUFLEN];
/**
 * @brief Head points to last keyboard inputted. Tail points to last input read by the program.
 * 
 */
std::size_t read_head_idx = 0, read_tail_idx = 0;
/**
 * @brief Mutex used for functions accessing the inputBuffer
 * 
 */
pthread_mutex_t read_mutex;
/**
 * @brief Buffer used for writing to log file in SF_OSAL_getline function
 * 
 */
std::string file_buf;
/**
 * @brief Dimensions of the ncurses CLI window
 * 
 */
std::size_t wind_h, wind_w;
/**
 * @brief Flag to allow deinit_conio to distinguish if SF_OSAL_getline has already written file_buf into log file
 * 
 */
bool buf_written = false;
/**
 * @brief Allows conio to know where in the log file that it may need to write to.
 * 
 */
std::size_t offset;

void *read_loop(void *_)
{
    int ch;
    while (true)
    {
        ch = wgetch(stdscr);
        // Check for any invalid inputs
        if (ch != '\b' && ch != KEY_BACKSPACE && ch != '\n' && ch != '\r' && (ch < ' ' || ch > '\x7F'))
        {
            // Scroll inputs only matter when CLI window is active
            if (conioHistory.get_display() && conioHistory.is_active())
            {
                // Check only for UP and DOWN scroll
                switch (ch)
                {
                case KEY_UP:
                {
                    std::size_t top_idx = conioHistory.get_cur_bottom_display() - wind_h + 1;
                    if (top_idx == 0) // Already at top
                    {
                        break;
                    }
                    if (conioHistory.get_cur_bottom_display() == conioHistory.get_bottom_display())
                    {
                        // Save anything already written
                        conioHistory.overwrite_last_line_at(file_buf.c_str(), file_buf.size(), offset, false);
                        buf_written = true;
                    }
                    wscrl(stdscr, -1);
                    curs_set(0);
                    move(0, 0);
                    wrefresh(stdscr);
                    conioHistory.set_cur_bottom_display(conioHistory.get_cur_bottom_display() - 1);

                    char *line = conioHistory.retrieve_display_line(--top_idx);
                    if (!line)
                    {
                        break;
                    }
                    wprintw(stdscr, "%s", line);
                    wrefresh(stdscr);
                    free(line);
                    break;
                }
                case KEY_DOWN:
                {
                    if (conioHistory.get_cur_bottom_display() ==
                        conioHistory.get_bottom_display()) // Already at bottom
                    {
                        break;
                    }
                    wscrl(stdscr, 1);
                    move(wind_h - 1, 0);
                    wrefresh(stdscr);
                    conioHistory.set_cur_bottom_display(conioHistory.get_cur_bottom_display() + 1);

                    char *line = conioHistory.retrieve_display_line(conioHistory.get_cur_bottom_display());
                    if (!line)
                    {
                        break;
                    }
                    wprintw(stdscr, "%s", line);
                    if (conioHistory.get_cur_bottom_display() == conioHistory.get_bottom_display())
                    {
                        curs_set(1);
                    }
                    wrefresh(stdscr);
                    free(line);
                    break;
                }
                }
            }
        }
        else {
            pthread_mutex_lock(&read_mutex);
            SF_OSAL_inputBuffer[read_head_idx % SF_OSAL_READ_BUFLEN] = ch;
            read_head_idx++;
            pthread_mutex_unlock(&read_mutex);
        }
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
        int rv;
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
        // Block until there is a new input
        while (!SF_OSAL_kbhit())
        {
        }
        pthread_mutex_lock(&read_mutex);
        int retval = SF_OSAL_inputBuffer[read_tail_idx % SF_OSAL_READ_BUFLEN];
        read_tail_idx++;
        pthread_mutex_unlock(&read_mutex);

        if (!conioHistory.get_display())
        {
            char retval_str[2] = {(char)retval, '\0'};
            conioHistory.write_line(retval_str, 1, false);
        }
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
        int userInput;

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
                    buffer[i + 1] = 0;
                    SF_OSAL_putch('\r');
                    SF_OSAL_putch('\n');
                    return i;
                }
            }

            // If we get USB disconnect, abort
            if (!pSystemDesc->flags->hasCharger)
            {
                return -1;
            }
        }
#elif SF_PLATFORM == SF_PLATFORM_GLIBC
        conioHistory.set_display(true);
        file_buf = "";
        offset = conioHistory.get_offset();
        while (i < buflen)
        {
            if (SF_OSAL_kbhit())
            {
                userInput = SF_OSAL_getch();

                buf_written = false;
                // Move window back to the bottom first if necessary
                if (conioHistory.get_cur_bottom_display() != conioHistory.get_bottom_display() && conioHistory.is_active())
                {
                    wclear(stdscr);
                    char *line;
                    for (std::size_t idx = conioHistory.get_bottom_display() - wind_h + 1; idx < conioHistory.get_bottom_display(); idx++)
                    {
                        line = conioHistory.retrieve_display_line(idx);
                        if (!line)
                        {
                            wprintw(stdscr, "\n");
                            continue;
                        }
                        wprintw(stdscr, "%s\n", line);
                        wrefresh(stdscr);
                        free(line);
                    }
                    line = conioHistory.retrieve_display_line(conioHistory.get_bottom_display());
                    if (!line)
                    {
                        break;
                    }
                    wprintw(stdscr, "%s", line);
                    wrefresh(stdscr);
                    free(line);
                    conioHistory.set_cur_bottom_display(conioHistory.get_bottom_display());
                    curs_set(1);
                }
                switch (userInput)
                {
                    case KEY_BACKSPACE:
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
                    case '\n':
                        buffer[i++] = 0;
                        SF_OSAL_putch('\n');
                        conioHistory.overwrite_last_line_at(file_buf.c_str(), file_buf.size(), offset, true);
                        conioHistory.set_display(false);
                        return i;
                    default:
                        buffer[i++] = userInput;
                        SF_OSAL_putch(userInput);
                        file_buf += userInput;
                        break;
                }
            }
        }
        // Exceeded buffer is automatically entered as command
        SF_OSAL_putch('\n');
        conioHistory.write_line(file_buf.c_str(), file_buf.size(), true);
        conioHistory.set_display(false);
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
        conioHistory.set_display(true);
        nBytes = vsnprintf(SF_OSAL_printfBuffer, SF_OSAL_PRINTF_BUFLEN, fmt, vargs);
        wprintw(stdscr, "%s", SF_OSAL_printfBuffer);

        const char* start = SF_OSAL_printfBuffer;
        const char* newline;
        while ((newline = strchr(start, '\n')) != NULL) 
        {
            size_t len = newline - start;
            conioHistory.write_line(start, len, true);
            start = newline + 1;
        }

        // Write remaining portion after last newline (if any)
        if (*start != '\0') 
        {
            conioHistory.write_line(start, strlen(start), false);
        }
        wrefresh(stdscr);
        conioHistory.set_display(false);
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
        keypad(stdscr, TRUE);

        if (conioHistory.init_file_mapping() < 0)
        {
            printf("Failed to initialize conio history file mapping. Stopping history logging.\n");
            conioHistory.deinit_file_mapping();
        }
        getmaxyx(stdscr, wind_h, wind_w);

        pthread_create(&read_thread, NULL, read_loop, NULL);
        pthread_detach(read_thread);
#endif
    }

    void SF_OSAL_deinit_conio(void)
    {
#if SF_PLATFORM == SF_PLATFORM_GLIBC
        if (!buf_written)
        {
            conioHistory.overwrite_last_line_at(file_buf.c_str(), file_buf.size(), offset, false);
        }
        conioHistory.deinit_file_mapping();
        endwin();
#endif
    }
}