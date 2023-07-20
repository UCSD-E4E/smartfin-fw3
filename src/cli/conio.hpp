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
    /// @brief Get character from serial
    /// @param  void
    /// @return void
    int getch(void);
    /// @brief Check if key is pressed
    /// @param void
    /// @return 1 if pressed, 0 otherwise
    int kbhit(void);
    /// @brief Pushes character to serial
    /// @param ch character to display
    /// @return void
    int putch(int ch);
    /**
     * @brief Printf equivilent
     * 
     * @param fmt initial text
     * @param ... values to push
     * @return int 1 if sucssesful 
     */
    int SF_OSAL_printf(const char* fmt, ...);
#ifdef __cplusplus
}
#endif

#endif