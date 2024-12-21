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
     * @return length of the user input line
     */
    int SF_OSAL_getline(char *buffer, int buflen);

    /**
     * @brief Reads the next character from stdin
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

    /**
     * @brief Flushes any input buffer
     *
     */
    void SF_OSAL_flush_input(void);

#ifdef __cplusplus
}
#endif

#endif
