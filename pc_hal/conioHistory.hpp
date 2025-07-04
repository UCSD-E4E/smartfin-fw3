/**
 * @file conioHistory.cpp
 * @author Ethan Huang (e8huang@ucsd.edu)
 * @brief Command Line History Implementation
 * @version 0.1
 * @date 2025-03-06
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef __CONIOHISTORY_HPP__
#define __CONIOHISTORY_HPP__

#include <cstdint>
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
 * @brief Max size of a single log file is 1GB
 * 
 */
#define MAX_FILE_SIZE 1024 * 1024 * 1024

class conioHistory
{
    public:
        /**
         * @brief Get the singleton instance of conioHistory
         *
         * @return conioHistory& Reference to the singleton instance
         */
        static conioHistory &getInstance(void);
        /**
         * @brief Initialize memory mapped file for IO
         *
         */
        int init_file_mapping(void);

        /**
         * @brief Resize the file
         *
         */
        int resize_file(void);

        /**
         * @brief Deinitializes the memory mapped file and deactivates history logging
         *
         */
        void deinit_file_mapping(void);

        /**
         * @brief Write the given line into the file
         *
         * @param line Line to be written
         * @param size Size of the line (not counting the null terminator)
         * @param NL_exists Add a new line after
         */
        void write_line(const char *line, const std::size_t size, const bool NL_exists);

        /**
         * @brief Clears the file from the given offset to EOF and writes the file at that offset
         * @details This function is not safe to use without proper implementation knowledge and
         * assumes that the offset provided is for the last line struct of file. Improper usage can
         * break the history logging system
         *
         * @param line Line to be written
         * @param size Size of the line (not counting the null terminator)
         * @param offset Starting offset point to write
         * @param NL_exists Add a new line after
         */
        void overwrite_last_line_at(const char *line, const std::size_t size,const std::size_t offset, const bool NL_exists);

        /**
         * @brief Retrives the display line from the given index
         * @details This function dynamically allocates memory for the line. The user must free it after usage. A nullptr is returned if the line is empty at the index. Due to this return mechanism, the user must explicitly check if history logging is active before calling this function.
         *
         * @param line_idx Index of the line to be retrieved
         * @return char* of the line if successful
         */
        char *retrieve_display_line(const std::size_t line_idx);

        /**
         * @brief Get the current offset of the file
         *
         * @return Current offset of the file
         */
        std::size_t get_offset() const;

        /**
         * @brief Get the display flag
         *
         * @return true if display is enabled, false otherwise
         */
        bool get_display() const;

        /**
         * @brief Set the display flag
         *
         * @param _display true to enable display, false to disable
         */
        void set_display(bool _display);

        /**
         * @brief Get the bottom display index
         *
         * @return Bottom display index
         */
        std::size_t get_bottom_display() const;

        /**
         * @brief Set the bottom display index
         *
         * @param _bottom_display New bottom display index
         */
        void set_bottom_display(std::size_t _bottom_display);

        /**
         * @brief Get the current bottom display index
         *
         * @return Current bottom display index
         */
        std::size_t get_cur_bottom_display() const;

        /**
         * @brief Set the current bottom display index
         *
         * @param _cur_bottom_display New current bottom display index
         */
        void set_cur_bottom_display(std::size_t _cur_bottom_display);

        /**
         * @brief Get the current status of the history logging
         * 
         * @return true if active, false if not
         */
        bool is_active() const;

    private:
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
            std::size_t offset;
            /**
             * @brief Length of the line
             * 
             */
            std::size_t len;
            /**
             * @brief Fragment sequence number within the line
             * 
             */
            std::uint32_t frag_seq;
            /**
             * @brief Display flag: false = do not display, true = display to window
             *
             */
            bool display;
            /**
             * @brief More Framgents flag: false = no successive fragments, true = more fragments for the line
             *
             */
            bool more_frag;
            // Additional metadata may be necessary

            /**
             * @brief Construct a new conio history line object with manually set parameters
             * 
             * @param _offset Starting offset
             * @param _len Current length of the line
             * @param _display Display flag
             * @param _more_frag More Fragments flag
             * @param _frag_seq Fragment sequence number
             */
            CONIO_history_line_(std::size_t _offset,
                                std::size_t _len,
                                std::uint32_t _frag_seq,
                                bool _display,
                                bool _more_frag);
            /**
             * @brief Construct a new conio history line object with default flags
             *
             * @param _offset Starting offset
             */
            CONIO_history_line_(std::size_t _offset);
        } CONIO_hist_line;

        /**
         * @brief Flag to indicate if the history logging is active
         * @details This is flag is only set to false if there is an error with mapping the file.
         */
        bool active = false;
        /**
         * @brief Flag set to display line for CLI window
         * @details This flag should be set for functions that will put characters/strings into the
         * ncurses CLI window, such as SF_OSAL_getline and SF_OSAL_printf found in conio.cpp
         */
        bool display = false;

        /**
         * @brief Keeps track of the bottom index of the CLI window
         *
         */
        std::size_t cur_bottom_display = 0;

        /**
         * @brief Keeps track of the bottom of the display lines
         *
         */
        std::size_t bottom_display = 0;

        /**
         * @brief Pointer to the memory mapped file
         * 
         */
        char *mapped_memory = nullptr;

        /**
         * @brief Current file size
         * 
         */
        std::size_t file_size = INITIAL_FILE_SIZE;

        /**
         * @brief Current offset within the file
         * 
         */
        std::size_t current_offset = 0;

        /**
         * @brief File descriptor for the memory mapped file
         * 
         */
        int fd = 0;

        /**
         * @brief List of all lines in the history
         * 
         */
        std::vector<CONIO_hist_line> lines;

        /**
         * @brief Vector of starting indices for lines set to display
         * 
         */
        std::vector<std::size_t> display_starts;

        /**
         * @brief Keeps track of the bottom index of the history
         * 
         */
        std::size_t bottom_idx = 0;

        conioHistory();
        // Disable copy constructor and assignment operator to preserve singleton nature
        conioHistory(const conioHistory&) = delete;
        conioHistory& operator=(const conioHistory&) = delete;
};
#endif