#ifndef __CONIOHISTORY_HPP__
#define __CONIOHISTORY_HPP__

#include <cstring>
#include <string>
#include <vector>

/**
 * @brief Starting file size for the history's log file
 *
 */
#define INITIAL_FILE_SIZE 4096
/**
 * @brief Resize the file by a factor of FILE_RESIZE_FACTOR
 * 
 */
#define FILE_RESIZE_FACTOR 2

/**
 * @brief C struct that contains the necessary metadata for a line in the history
 * 
 */
typedef struct CONIO_history_line_
{
    /**
     * @brief Starting offset within the file
     * 
     */
    size_t offset;
    /**
     * @brief Length of the line
     * 
     */
    size_t len;
    // Additional metadata may be necessary

    /**
     * @brief Construct a new conio history line object
     * 
     * @param o Starting offset
     * @param l Current length of the line
     */
    CONIO_history_line_(size_t o, size_t l) : offset(o), len(l) {}
} CONIO_hist_line;

#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * @brief Initialize memory mapped file for IO
     * 
     */
    void init_file_mapping(void);

    /**
     * @brief Resize the file
     * 
     */
    void resize_file(void);
    
    /**
     * @brief Deinitializes the memory mapped file
     * 
     */
    void deinit_file_mapping(void);

    /**
     * @brief Write the given line into the file
     * 
     * @param line Line to be written
     * @param NL_exists Add a new line after
     */
    void write_line(const std::string &line, const bool NL_exists);

    /**
     * @brief Clears the file from the given offset to EOF and writes the file at that offset
     * @details This function is not safe to use without proper implementation knowledge and assumes that the offset provided is for the current last line of file. Improper usage can break the history logging system
     * 
     * @param line Line to be written
     * @param offset Starting offset point to write
     * @param NL_exists Add a new line after
     */
    void overwrite_last_line_at(const std::string &line, const size_t offset, const bool NL_exists);

    /**
     * @brief Retrives the line from the given index
     * @details This function dynamically allocates memory for the line. The user must free it after usage. A nullptr is returned if there is no line at the index
     * 
     * @param line_idx Index of the line to be retrieved
     * @return char* of the line if successful
     */
    char *retrieve_line(const size_t line_idx);

    /**
     * @brief Get the current offset of the file
     * 
     * @return Current offset of the file
     */
    size_t get_offset();
}

/**
 * @brief Keeps track of the bottom index of the CLI window
 * 
 */
extern size_t cur_bottom;
/**
 * @brief Keeps track of the bottom of the file
 * 
 */
extern size_t bottom_idx;
#endif