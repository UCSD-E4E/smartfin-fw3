#include "./scheduler_test_system.hpp"
#include "../src/cli/conio.hpp"
#include <chrono>

#include <stdarg.h>
#include <string>
#include <cstdio>



int SF_OSAL_printf(const char* fmt, ...)
{
    va_list vargs;
    va_start(vargs, fmt);
    int nBytes = vprintf(fmt, vargs);
    va_end(vargs);
    return nBytes;
}

uint32_t millis()
{
    return testTime;
}
void addTime(uint32_t add)
{
    testTime += add;
}
void setTime(uint32_t set)
{
    testTime = set;
}
void delay(uint32_t time)
{
    addTime(time);
}