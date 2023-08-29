#ifndef __CONIO_HPP__
#define __CONIO_HPP__

#include <stdint.h>

#define SF_OSAL_PRINTF_BUFLEN   1024

#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * @brief Gets character from serial
     * 
     * @return int key thats pressed
     */
    int getch(void);
    /**
     * @brief Checks if key is pressed
     * 
     * @return int whether key is pressed
     */
    int kbhit(void);
    /**
     * @brief Pushes character to serial
     * 
     * @param ch character to push
     * @return int Sucsess value
     */
    int putch(int ch);
    /**
     * @brief Printf equivilent
     * 
     * @param fmt initial text
     * @param ... values to push
     * @return int 1 if sucssesful 
     */
    int SF_OSAL_printf(const char* fmt, ...);

    /**
     * @brief Gets user input lin
     * @param buffer buffer to write too
     * @param buflen length of buffer
     */
    int getline(char* buffer, int buflen);

#ifdef __cplusplus
}
#endif

#endif