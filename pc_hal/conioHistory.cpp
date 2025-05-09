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

#include "conioHistory.hpp"

#include "product.hpp"

#if SF_PLATFORM == SF_PLATFORM_GLIBC
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

char *mapped_memory = nullptr;
size_t file_size = INITIAL_FILE_SIZE;
size_t current_offset = 0;
int fd;
std::vector<CONIO_hist_line> Lines;
std::vector<size_t> display_starts;
size_t bottom_idx = 0;

namespace conioHistory
{
    size_t cur_bottom_display = 0;
    size_t bottom_display = 0;
    bool display = false;

    void init_file_mapping()
    {
        const char *env_path = std::getenv("E4E_SF_CONIO_HISTORY");
        env_path = env_path ? env_path : "/code/build/history.log";

        fd = open(env_path, O_RDWR | O_CREAT | O_TRUNC, 0666);
        if (fd < 0)
        {
            exit(1);
        }

        // Set initial file size
        ftruncate(fd, INITIAL_FILE_SIZE);

        // Map the file to memory
        mapped_memory = (char *)mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (mapped_memory == MAP_FAILED)
        {
            close(fd);
            exit(1);
        }

        Lines.push_back(CONIO_hist_line(0)); // Initialize the first line
    }

    void deinit_file_mapping()
    {
        if (mapped_memory)
        {
            munmap(mapped_memory, file_size);
        }
        if (fd >= 0)
        {
            close(fd);
        }
    }

    void resize_file()
    {
        if (file_size >= LINEAR_STEP_RESIZE)
        {
            file_size += LINEAR_STEP_RESIZE;
        }
        else
        {
            file_size *= FILE_RESIZE_FACTOR;
        }

        // Resize the file on disk
        if (ftruncate(fd, file_size) < 0)
        {
            close(fd);
            exit(1);
        }

        // Unmap old memory and remap the file to the new size
        if (munmap(mapped_memory, file_size / FILE_RESIZE_FACTOR) == -1)
        {
            close(fd);
            exit(1);
        }

        mapped_memory = (char *)mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (mapped_memory == MAP_FAILED)
        {
            close(fd);
            exit(1);
        }
    }

    void write_line(const std::string &line, const bool NL_exists)
    {
        // TEMPORARY: Stop writing to file at max size
        if (current_offset + line.size() + 2 >= MAX_FILE_SIZE)
        {
            return;
        }
        // Check if there's enough space, else resize
        if (current_offset + line.size() + 2 >= file_size)
        {
            resize_file();
        }

        // Set or create bottom line depending on display flag
        if (display && !Lines[bottom_idx].display)
        {
            if (Lines[bottom_idx].len == 0)
            {
                // Default line struct converted to display line
                Lines[bottom_idx].display = 1;
                bottom_display = display_starts.size();
                display_starts.push_back(bottom_idx);
                cur_bottom_display = bottom_display;
            }
            else
            {
                int fs = 0;
                // Check if fragmented display line exists for frag_seq num
                int64_t possible_frag = bottom_idx - 1;
                if (possible_frag >= 0 && Lines[possible_frag].more_frag)
                {
                    fs = Lines[possible_frag].frag_seq + 1;
                }
                // Not a fragment, update display starts
                else
                {
                    bottom_display = display_starts.size();
                    display_starts.push_back(bottom_idx + 1);
                    cur_bottom_display = bottom_display;
                }
                Lines.push_back(CONIO_hist_line(current_offset, 0, 1, 0, fs));
                bottom_idx++;
            }
        }
        else if (!display && Lines[bottom_idx].display)
        {
            if (Lines[bottom_idx].len == 0)
            {
                // Convert empty display line to non-display line
                Lines[bottom_idx].display = 0;
                if (display_starts[bottom_display] == bottom_idx)
                {
                    display_starts.pop_back();
                    bottom_display--;
                }
            }
            else
            {
                // Fragment the display line
                Lines[bottom_idx++].more_frag = 1;
                Lines.push_back(CONIO_hist_line(current_offset));
            }
        }
        strncpy(mapped_memory + current_offset, line.c_str(), line.size());
        current_offset += line.size();
        Lines[bottom_idx].len += line.size();

        if (display && NL_exists)
        {
            mapped_memory[current_offset++] = '\n';
            Lines.push_back(CONIO_hist_line(current_offset));
            bottom_idx++;
        }

        msync(mapped_memory, file_size, MS_SYNC);
    }

    void overwrite_last_line_at(const std::string &line, const size_t offset, const bool NL_exists)
    {
        if (offset > current_offset)
        {
            return; // Invalid offset
        }
        // Clear out the file within the interval [offset, current_offset)
        for (size_t i = offset; i < current_offset; i++)
        {
            mapped_memory[i] = 0;
        }
        Lines[bottom_idx].len -= current_offset - offset;
        current_offset = offset;
        write_line(line, NL_exists);
    }

    char *retrieve_display_line(const size_t line_idx)
    {
        // Invalid line index
        if (line_idx >= display_starts.size())
        {
            return nullptr;
        }
        size_t true_idx = display_starts[line_idx]; // Grab the true index
        if (Lines[true_idx].len == 0)
            return nullptr;
        std::vector<size_t> relevant_offsets, relevant_lens;
        // Collect any fragments and their relevant metadata
        while (Lines[true_idx].more_frag)
        {
            relevant_lens.push_back(Lines[true_idx].len);
            relevant_offsets.push_back(Lines[true_idx].offset);
            true_idx += 2; // Fragments only exist because true_idx + 1 is non-display line
        }
        relevant_lens.push_back(Lines[true_idx].len);
        relevant_offsets.push_back(Lines[true_idx].offset);

        size_t len = 1;
        for (size_t i = 0; i < relevant_lens.size(); i++)
            len += relevant_lens[i];

        char *line = (char *)malloc(len);
        size_t pos = 0;
        for (size_t i = 0; i < relevant_lens.size(); i++)
        {
            memcpy(line + pos, mapped_memory + relevant_offsets[i], relevant_lens[i]);
        }
        line[len - 1] = 0;
        return line;
    }

    size_t get_offset()
    {
        return current_offset;
    }
}
#endif