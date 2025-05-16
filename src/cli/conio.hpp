#ifndef __CONIO_HPP__
#define __CONIO_HPP__

#include <cstdarg>
#include <stdint.h>

/**
 * @brief int length of character array SF_OSAL_printfBuffer
 *
 */
#define SF_OSAL_PRINTF_BUFLEN   1536

#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * @brief Checks if key is pressed
     *
     * @return 1 if next keypress is available, otherwise 0
     */
    int SF_OSAL_kbhit(void);
    /**
     * @brief Pushes character to serial
     *
     * @param ch character to push
     * @return Printed char
     */
    int SF_OSAL_putch(int ch);
    /**
     * @brief Printf equivilent
     * 
     * @param fmt initial text
     * @param ... values to push
     * @return int 1 if sucssesful 
     */
    int SF_OSAL_printf(const char* fmt, ...);

    /**
     * @brief Gets user input line
     * @param buffer buffer to write too
     * @param buflen length of buffer
     * @return length of the user input line.  If -1, we have aborted due
     * to USB terminal disconnect
     */
    int SF_OSAL_getline(char *buffer, int buflen);

    /**
     * @brief Reads the next character from stdin
     * @details If the character is invalid, this function consumes the character and blocks until the next character can be read from stdin. This process continues until the character is valid.
     *
     * @return The obtained character on success or EOF on failure.
     */
    int SF_OSAL_getch(void);

    /**
     * @brief Initializes the conio facility
     *
     */
    void SF_OSAL_init_conio(void);

    /**
     * @brief Safely deinitializes the conio facility
     *
     */
    void SF_OSAL_deinit_conio(void);

#ifdef __cplusplus
}
#endif

#endif
