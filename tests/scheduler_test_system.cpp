#include "./scheduler_test_system.hpp"
#include <chrono>
#include <stdio.h>
#include <cstdarg> 
uint32_t millis() {
    static auto startTime = std::chrono::system_clock::now();
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
}
int SF_OSAL_printf(const char* fmt, ...) {
    va_list vargs;
    va_start(vargs, fmt);
    int nBytes = vprintf(fmt, vargs);
    va_end(vargs);
    return nBytes;
}