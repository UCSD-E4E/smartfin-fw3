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
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

conioHistory::conioHistory() 
{
}

conioHistory &conioHistory::getInstance(void)
{
    static conioHistory instance;
    return instance;
}

conioHistory::CONIO_history_line_::CONIO_history_line_(std::size_t _offset) : offset(_offset), len(0), frag_seq(0), display(false), more_frag(false)
{
}

conioHistory::CONIO_history_line_::CONIO_history_line_(std::size_t _offset,
                                                       std::size_t _len,
                                                       std::uint32_t _frag_seq,
                                                       bool _display,
                                                       bool _more_frag)
    : offset(_offset), len(_len), frag_seq(_frag_seq), display(_display), more_frag(_more_frag)
    {
    }

void conioHistory::init_file_mapping(void)
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

    lines.push_back(CONIO_hist_line(0)); // Initialize the first line
}

void conioHistory::deinit_file_mapping(void)
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

void conioHistory::resize_file(void)
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

void conioHistory::write_line(const char *line, const std::size_t size, bool NL_exists)
{
    // TEMPORARY: Stop writing to file at max size
    if (current_offset + size + 2 >= MAX_FILE_SIZE)
    {
        return;
    }
    // Check if there's enough space, else resize
    if (current_offset + size + 2 >= file_size)
    {
        resize_file();
    }

    // Set or create bottom line depending on display flag
    if (display && !lines[bottom_idx].display)
    {
        if (lines[bottom_idx].len == 0)
        {
            // Default line struct converted to display line
            lines[bottom_idx].display = true;
            bottom_display = display_starts.size();
            display_starts.push_back(bottom_idx);
            cur_bottom_display = bottom_display;
        }
        else
        {
            int fs = 0;
            // Check if fragmented display line exists for frag_seq num
            std::size_t possible_frag = bottom_idx - 1;
            if (possible_frag >= 0 && lines[possible_frag].more_frag)
            {
                fs = lines[possible_frag].frag_seq + 1;
            }
            // Not a fragment, update display starts
            else
            {
                bottom_display = display_starts.size();
                display_starts.push_back(bottom_idx + 1);
                cur_bottom_display = bottom_display;
            }
            lines.push_back(CONIO_hist_line(current_offset, 0, fs, true, false));
            bottom_idx++;
        }
    }
    else if (!display && lines[bottom_idx].display)
    {
        if (lines[bottom_idx].len == 0)
        {
            // Convert empty display line to non-display line
            lines[bottom_idx].display = false;
            if (display_starts[bottom_display] == bottom_idx)
            {
                display_starts.pop_back();
                bottom_display--;
            }
        }
        else
        {
            // Fragment the display line
            lines[bottom_idx++].more_frag = true;
            lines.push_back(CONIO_hist_line(current_offset));
        }
    }
    strncpy(mapped_memory + current_offset, line, size);
    current_offset += size;
    lines[bottom_idx].len += size;

    if (display && NL_exists)
    {
        mapped_memory[current_offset++] = '\n';
        lines.push_back(CONIO_hist_line(current_offset));
        bottom_idx++;
    }

    msync(mapped_memory, file_size, MS_SYNC);
}

void conioHistory::overwrite_last_line_at(const char *line, const std::size_t size, std::size_t offset, const bool NL_exists)
{
    if (offset > current_offset)
    {
        return; // Invalid offset
    }
    // Clear out the file within the interval [offset, current_offset)
    for (std::size_t i = offset; i < current_offset; i++)
    {
        mapped_memory[i] = 0;
    }
    lines[bottom_idx].len -= current_offset - offset;
    current_offset = offset;
    write_line(line, size, NL_exists);
}

char *conioHistory::retrieve_display_line(const std::size_t line_idx)
{
    // Invalid line index
    if (line_idx >= display_starts.size())
    {
        return nullptr;
    }
    std::size_t true_idx = display_starts[line_idx]; // Grab the true index
    if (lines[true_idx].len == 0)
        return nullptr;
    std::vector<std::size_t> relevant_offsets, relevant_lens;
    // Collect any fragments and their relevant metadata
    while (lines[true_idx].more_frag)
    {
        relevant_lens.push_back(lines[true_idx].len);
        relevant_offsets.push_back(lines[true_idx].offset);
        true_idx += 2; // Fragments only exist because true_idx + 1 is non-display line
    }
    relevant_lens.push_back(lines[true_idx].len);
    relevant_offsets.push_back(lines[true_idx].offset);

    std::size_t len = 1;
    for (std::size_t i = 0; i < relevant_lens.size(); i++)
        len += relevant_lens[i];

    char *line = (char *)malloc(len);
    std::size_t pos = 0;
    for (std::size_t i = 0; i < relevant_lens.size(); i++)
    {
        memcpy(line + pos, mapped_memory + relevant_offsets[i], relevant_lens[i]);
    }
    line[len - 1] = 0;
    return line;
}

std::size_t conioHistory::get_offset() const
{
    return current_offset;
}

bool conioHistory::get_display() const
{
    return display;
}

void conioHistory::set_display(bool _display)
{
    display = _display;
}

std::size_t conioHistory::get_bottom_display() const
{
    return bottom_display;
}

void conioHistory::set_bottom_display(std::size_t _bottom_display)
{
    bottom_display = _bottom_display;
}

std::size_t conioHistory::get_cur_bottom_display() const
{
    return cur_bottom_display;
}

void conioHistory::set_cur_bottom_display(std::size_t _cur_bottom_display)
{
    cur_bottom_display = _cur_bottom_display;
}
#endif