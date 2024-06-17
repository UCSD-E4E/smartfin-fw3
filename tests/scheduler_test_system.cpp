/**@todo Add documentation*/
#include "scheduler_test_system.hpp"
#include "conio.hpp"
#include <chrono>

#include <stdarg.h>
#include <string>
#include <cstdio>

/**@todo Add documentation*/
bool TestLog::operator==(const TestLog& rhs)
{
    return  (name == rhs.name) &&
        (start == rhs.start) &&
        (end == rhs.end);
};
/**@todo Add documentation*/
int SF_OSAL_printf(const char* fmt, ...)
{
    va_list vargs;
    va_start(vargs, fmt);
    int nBytes = vprintf(fmt, vargs);
    va_end(vargs);
    return nBytes;
}
/**@todo Add documentation*/
uint32_t millis()
{
    return testTime;
}
/**@todo Add documentation*/
void addTime(uint32_t add)
{
    testTime += add;
}
/**@todo Add documentation*/
void setTime(uint32_t set)
{
    testTime = set;
}
/**@todo Add documentation*/
void delay(uint32_t time)
{
    addTime(time);
}