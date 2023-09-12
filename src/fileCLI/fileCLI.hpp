#ifndef __FILECLI_H__
#define __FILECLI_H__

#include "Particle.h"

#include <stddef.h>
#include <dirent.h>

#define FILE_CLI_INPUT_BUFFER_LEN   80
#define FILE_CLI_MAX_DIR_DEPTH  4
#define PATH_SEP    "/"
#define FILE_CLI_MAX_PATH_LEN   64

/**
 * @brief File CLI
 * 
 * Provides a set of commands to interact with the local filesystem
 * 
 */
class FileCLI{
    public:
    /**
     * @brief Creates a new FileCLI object
     * 
     */
    FileCLI(void){};
    /**
     * @brief Executes the File CLI
     * 
     */
    void execute(void);
    /**
     * @brief Destructor
     * 
     */
    ~FileCLI(void){};

    protected:
    private:
    /**
     * @brief Switches to the next active file
     * 
     */
    void doNextFile(void);
    /**
     * @brief Duplicates the current file
     * 
     */
    void copyFile(void);
    /**
     * @brief Dumps the current file to console as base85
     * 
     */
    void dumpBase85(void);
    /**
     * @brief Dumps the current file as a comma separate sequence of byte values
     * 
     */
    void dumpDecimal(void);
    /**
     * @brief Dumps the current file as a hexdump
     * 
     */
    void dumpHex(void);
    /**
     * @brief Dumps the current file to console as ASCII
     * 
     */
    void dumpAscii(void);
    /**
     * @brief Removes the current file from the filesystem
     * 
     */
    void deleteFile(void);
    /**
     * @brief Exits the file CLI
     * 
     */
    void exit(void);
    /**
     * @brief Lists the files in the current directory
     * 
     */
    void list_dir(void);
    int run = 1;
    DIR* dir_stack[FILE_CLI_MAX_DIR_DEPTH];
    char* path_stack[FILE_CLI_MAX_DIR_DEPTH];
    int current_dir;
    typedef struct menu_
    {
        const char cmd;
        void (FileCLI::*fn)(void);
    } menu_t;
    static menu_t fsExplorerMenu[];
    /**
     * @brief Finds the menu entry corresponding to the specified command string
     * 
     * @param cmd Command String
     * @return Pointer to menu entry if matching entry found, otherwise nullptr
     */
    static menu_t* findCommand(const char* const cmd);
    const char* buildPath(void);
};
#endif
