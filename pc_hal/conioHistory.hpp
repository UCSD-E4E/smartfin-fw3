#ifndef __CONIOHISTORY_HPP__
#define __CONIOHISTORY_HPP__

#include <stdint.h>
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
 * @brief Upon reaching a size of 32MB, increase file size in steps of 32MB
 * 
 */
#define LINEAR_STEP_RESIZE 32 * 1024 * 1024

/**
 * @brief Struct that contains the necessary metadata for a line in the history
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
    /**
     * @brief Display flag: 0 do not display, 1 display to window
     * 
     */
    uint32_t display : 1;
    /**
     * @brief More Framgents flag: 0 no successive fragments, 1 more fragments for the line
     * 
     */
    uint32_t more_frag : 1;
    /**
     * @brief Fragment sequence number within the line
     * 
     */
    uint32_t frag_seq : 30;
    // Additional metadata may be necessary

    /**
     * @brief Construct a new conio history line object with manually set parameters
     * 
     * @param o Starting offset
     * @param l Current length of the line
     * @param d Display flag
     * @param mf More Fragments flag
     * @param fs Fragment sequence number
     */
    CONIO_history_line_(size_t o, size_t l, uint32_t d, uint32_t mf, uint32_t fs) : offset(o), len(l), display(d), more_frag(mf), frag_seq(fs) {}
    /**
     * @brief Construct a new conio history line object with default flags
     * 
     * @param o Starting offset
     */
    CONIO_history_line_(size_t o) : offset(o) {
        len = 0;
        display = 0;
        more_frag = 0;
        frag_seq = 0;
    }
} CONIO_hist_line;

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

/**
 * @brief Flag set to coalesce writes to log file.
 * 
 */
extern bool coalesce;

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