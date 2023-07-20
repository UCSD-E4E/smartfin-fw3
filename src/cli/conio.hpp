#ifndef __CONIO_HPP__
#define __CONIO_HPP__
// #include <stdarg.h>
// #include <stddef.h>
#include <stdint.h>

#define SF_OSAL_PRINTF_BUFLEN   1024

#ifdef __cplusplus
extern "C"
{
#endif
    int getch(void);
    int kbhit(void);
    int putch(int ch);
    int getline(char* buffer, int buflen);
    int SF_OSAL_printf(const char* fmt, ...);
    char* getUserInput(char* userInput);
#ifdef __cplusplus
}
#endif

#endif