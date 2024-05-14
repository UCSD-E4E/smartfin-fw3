#include "./scheduler_test_system.hpp"
#include <chrono>
#include <stdio.h>
#include <stdarg.h> 
uint32_t millis() 
{
    return testTime;
}
int SF_OSAL_printf(const char* fmt, ...) 
{
    va_list vargs;
    va_start(vargs, fmt);
    int nBytes = vprintf(fmt, vargs);
    va_end(vargs);
    return nBytes;
}
void addTime(uint32_t add)
{
    testTime += add;
}
void setTime(uint32_t set)
{
    testTime = set;
}