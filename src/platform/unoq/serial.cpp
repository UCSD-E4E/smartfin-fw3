/**
 * @file serial.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @author Updated by Brent
 * @brief Uno Q (QRB2210 Linux) implementation of SF_HAL debug/CLI serial
 *        functions.
 * @date 2026-09-22
 *
 * The debug/CLI console is native to Linux (stdio or a tty), not the SPI
 * link to the STM32U585.
 */
#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_UNOQ

#include "platform/hal.hpp"
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>

namespace SF_HAL
{

void serial_begin(uint32_t baud)
{
    // Native Linux stdout doesn't require baud rate initialization
}

int serial_available()
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    // Use select() to check stdin without blocking
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0 ? 1 : 0;
}

int serial_read()
{
    if (!serial_available()) return -1;
    
    int ch;
    struct termios oldt, newt;
    
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // Disable buffering and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    ch = getchar();
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

void serial_print_char(char ch)
{
    putchar(ch);
    fflush(stdout);
}

void serial_write(const uint8_t* buf, std::size_t len)
{
    fwrite(buf, 1, len, stdout);
    fflush(stdout);
}

} // namespace SF_HAL

// --- OSAL Functions Required by CLI and Linker ---

extern "C" {

void SF_OSAL_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fflush(stdout);
}

void SF_OSAL_getline(char *buffer, int max_length) {
    if (fgets(buffer, max_length, stdin) != NULL) {
        for (int i = 0; i < max_length; i++) {
            if (buffer[i] == '\n') {
                buffer[i] = '\0';
                break;
            }
        }
    }
}

bool SF_OSAL_kbhit() {
    return SF_HAL::serial_available() > 0;
}

int SF_OSAL_getch() {
    // Block until a character is available
    while (!SF_HAL::serial_available()) {
        usleep(10000); 
    }
    return SF_HAL::serial_read();
}

} // end extern "C"

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ