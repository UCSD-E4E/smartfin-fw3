#ifndef __CONIO_HPP__
#define __CONIO_HPP__
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif
    int getch(void);
    int kbhit(void);
    int putch(int ch);
    int getline(char* buffer, int buflen);
#ifdef __cplusplus
}
#endif

#endif