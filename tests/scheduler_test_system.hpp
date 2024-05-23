#ifndef __SCHEDULER__TEST__HPP_
#define __SCHEDULER__TEST__HPP_
#include <stdint.h>

typedef uint32_t system_tick_t;
static int testTime = 0;
void addTime(uint32_t add);

void setTime(uint32_t set);
void delay(uint32_t add);
uint32_t millis();
int SF_OSAL_printf(const char* fmt, ...);
struct TestLog
{
    char name;
    uint32_t start, end;
    TestLog(char name, uint32_t start, uint32_t end) : name(name), start(start), end(end)
    {
    }

    bool operator==(const TestLog& rhs)
    {
        return  (name == rhs.name) &&
            (start == rhs.start) &&
            (end == rhs.end);
    };
};
#endif //__SCHEDULER__TEST__HPP_