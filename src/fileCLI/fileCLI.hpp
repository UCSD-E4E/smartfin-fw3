#ifndef __FILECLI_H__
#define __FILECLI_H__

#include "Particle.h"

#include <dirent.h>
#include <limits.h>
#include <stddef.h>

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
    FileCLI(void);
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
     * @brief Dumps the current file to console as base85
     * 
     */
    void dumpBase85(void);

    /**
     * @brief Dumps the current file as a hexdump
     * 
     */
    void dumpHex(void);

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
    /**
     * @brief Changes to the specified directory
     * 
     */
    void change_dir(void);
    /**
     * @brief Print working directory
     * 
     */
    void print_dir(void);

    /**
     * @brief Prints the help for the FileCLI
     * 
     */
    void print_help(void);

    /**
     * @brief Creates a new directory
     * 
     */
    void mkdir(void);

    int run = 1;
    DIR* dir_stack[FILE_CLI_MAX_DIR_DEPTH];
    char path_stack[FILE_CLI_MAX_DIR_DEPTH][NAME_MAX];
    int current_dir;

    /**
     * @brief Structure representing a command menu entry for FileCLI.
     *
     */
    typedef struct menu_
    {
        /**
         * A character representing a user command. Will trigger a specific function.
         */
        const char cmd;
        /**
         * A pointer to a place in flash memory or RAM where a function with properties is held.
         * Executes the command associated with cmd.
         */
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
    const char* buildPath(bool is_dir);
};
#endif
