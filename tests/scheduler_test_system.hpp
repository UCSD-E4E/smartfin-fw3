/** @todo Add documentation*/
#ifndef __SCHEDULER__TEST__HPP_
#define __SCHEDULER__TEST__HPP_
#include <stdint.h>

/** @todo Add documentation*/
typedef uint32_t system_tick_t;
/** @todo Add documentation*/
static int testTime = 0;
/** @todo Add documentation*/
void addTime(uint32_t add);
/** @todo Add documentation*/
void setTime(uint32_t set);
/** @todo Add documentation*/
void delay(uint32_t add);
/** @todo Add documentation*/
uint32_t millis();
/** @todo Add documentation*/
struct TestLog
{
    char name;
    uint32_t start, end;
    TestLog(char name, uint32_t start, uint32_t end) : name(name), 
                                    start(start), end(end){}

    bool operator==(const TestLog& rhs);
};
#endif //__SCHEDULER__TEST__HPP_