#ifndef __FILECLI_H__
#define __FILECLI_H__

#include "Particle.h"

#include <stddef.h>
#include <dirent.h>
#include <sys/syslimits.h>

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

    /**
     * @brief Indicates whether the CLI is running.
     *
     * @details When set to 1, the CLI continues running; when set to 0,
     * the CLI exits.
     */
    int run = 1;

    /**
     * @brief Stack of directory pointers for navigating through directories.
     *
     * @details This stack holds up to FILE_CLI_MAX_DIR_DEPTH directories.
     */
    DIR* dir_stack[FILE_CLI_MAX_DIR_DEPTH];

    /**
     * @brief Stack of paths corresponding to the directory stack.
     *
     * This stack holds the paths for the directories in the `dir_stack`.
     */
    char path_stack[FILE_CLI_MAX_DIR_DEPTH][NAME_MAX];
    /**
     * @brief Index of the current directory in the directory stack.
     *
     * @details This integer represents the current position within
     * the `dir_stack`.
     */
    int current_dir;
    /**
     * @brief typedef for a menu entry for fileCli.
     *
     * @details contains a command and a function for each entry
     */
    typedef struct menu_
    {
        //! Command character for the menu entry.
        const char cmd;
        //! Function pointer to the command handler.
        void (FileCLI::*fn)(void);
    } menu_t;

    /**
     * @brief Array of menu entries.
     *
     *  @details This menu is used to handle various file operations in the CLI.
     */
    static menu_t fsExplorerMenu[];

    /**
     * @brief Finds the menu entry corresponding to the specified command string
     * 
     * @param cmd Command String
     * @return Pointer to menu entry if matching entry found, otherwise nullptr
     */
    static menu_t* findCommand(const char* const cmd);

    /**
     * @brief Constructs a path from the current directory stack.
     *
     * @param is_dir If true, the path is a directory path.
     * @return const char* The constructed file or directory path.
     */
    const char* buildPath(bool is_dir);
};
#endif
