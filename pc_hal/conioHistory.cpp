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
#define map_memory (char *)mmap(nullptr, this->file_size, PROT_READ | PROT_WRITE, MAP_SHARED, this->fd, 0);

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

int conioHistory::init_file_mapping(void)
{
    const char *env_path = std::getenv("E4E_SF_CONIO_HISTORY");
    env_path = env_path ? env_path : "/code/build/history.log";

    this->fd = open(env_path, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (this->fd < 0)
    {
        this->active = false;
        return -1;
    }

    // Set initial file size
    ftruncate(this->fd, INITIAL_FILE_SIZE);

    // Map the file to memory
    this->mapped_memory = map_memory;
    if (this->mapped_memory == MAP_FAILED)
    {
        close(this->fd);
        this->active = false;
        return -1;
    }

    this->lines.push_back(CONIO_hist_line(0)); // Initialize the first line
    this->active = true;
    return 0;
}

void conioHistory::deinit_file_mapping(void)
{
    if (this->mapped_memory)
    {
        munmap(this->mapped_memory, this->file_size);
    }
    if (this->fd >= 0)
    {
        close(this->fd);
    }
    this->active = false;
}

int conioHistory::resize_file(void)
{
    if (this->file_size >= LINEAR_STEP_RESIZE)
    {
        this->file_size += LINEAR_STEP_RESIZE;
    }
    else
    {
        this->file_size *= FILE_RESIZE_FACTOR;
    }

    // Resize the file on disk
    if (ftruncate(this->fd, this->file_size) < 0)
    {
        close(this->fd);
        this->active = false;
        return -1;
    }

    // Unmap old memory and remap the file to the new size
    if (munmap(this->mapped_memory, this->file_size / FILE_RESIZE_FACTOR) == -1)
    {
        close(this->fd);
        this->active = false;
        return -1;
    }

    this->mapped_memory = map_memory;
    if (this->mapped_memory == MAP_FAILED)
    {
        close(this->fd);
        this->active = false;
        return -1;
    }
    return 0;
}

void conioHistory::write_line(const char *line, const std::size_t size, bool NL_exists)
{
    if (!this->active)
    {
        return; // History logging is not active
    }
    // TEMPORARY: Stop writing to file at max size
    if (this->current_offset + size + 2 >= MAX_FILE_SIZE)
    {
        this->active = false;
        return;
    }
    // Check if there's enough space, else resize
    if (this->current_offset + size + 2 >= this->file_size)
    {
        if (this->resize_file() < 0)
        {
            deinit_file_mapping();
            return; // Failed to resize file, deactivate history logging
        }
    }

    // Set or create bottom line depending on display flag
    if (this->display && !this->lines[this->bottom_idx].display)
    {
        if (this->lines[this->bottom_idx].len == 0)
        {
            // Default line struct converted to display line
            this->lines[this->bottom_idx].display = true;
            this->bottom_display = this->display_starts.size();
            this->display_starts.push_back(this->bottom_idx);
            this->cur_bottom_display = this->bottom_display;
        }
        else
        {
            int fs = 0;
            // Check if fragmented display line exists for frag_seq num
            std::int64_t possible_frag = this->bottom_idx - 1;
            if (possible_frag >= 0 && this->lines[possible_frag].more_frag)
            {
                fs = this->lines[possible_frag].frag_seq + 1;
            }
            // Not a fragment, update display starts
            else
            {
                this->bottom_display = this->display_starts.size();
                this->display_starts.push_back(this->bottom_idx + 1);
                this->cur_bottom_display = this->bottom_display;
            }
            this->lines.push_back(CONIO_hist_line(this->current_offset, 0, fs, true, false));
            this->bottom_idx++;
        }
    }
    else if (!this->display && this->lines[this->bottom_idx].display)
    {
        if (this->lines[this->bottom_idx].len == 0)
        {
            // Convert empty display line to non-display line
            this->lines[this->bottom_idx].display = false;
            if (this->display_starts[this->bottom_display] == this->bottom_idx)
            {
                this->display_starts.pop_back();
                this->bottom_display--;
            }
        }
        else
        {
            // Fragment the display line
            this->lines[this->bottom_idx++].more_frag = true;
            this->lines.push_back(CONIO_hist_line(this->current_offset));
        }
    }
    strncpy(this->mapped_memory + this->current_offset, line, size);
    this->current_offset += size;
    this->lines[this->bottom_idx].len += size;

    if (this->display && NL_exists)
    {
        this->mapped_memory[this->current_offset++] = '\n';
        this->lines.push_back(CONIO_hist_line(this->current_offset));
        this->bottom_idx++;
    }

    msync(this->mapped_memory, this->file_size, MS_SYNC);
}

void conioHistory::overwrite_last_line_at(const char *line, const std::size_t size, std::size_t offset, const bool NL_exists)
{
    if (offset > this->current_offset || !this->active)
    {
        return; // Invalid offset or inactive history logging
    }
    // Clear out the file within the interval [offset, this->current_offset)
    for (std::size_t i = offset; i < this->current_offset; i++)
    {
        this->mapped_memory[i] = 0;
    }
    this->lines[this->bottom_idx].len -= this->current_offset - offset;
    this->current_offset = offset;
    this->write_line(line, size, NL_exists);
}

char *conioHistory::retrieve_display_line(const std::size_t line_idx)
{
    // Invalid line index
    if (line_idx >= display_starts.size())
    {
        return nullptr;
    }
    std::size_t true_idx = this->display_starts[line_idx]; // Grab the true index
    if (this->lines[true_idx].len == 0)
    {
        return nullptr;
    }

    std::vector<std::size_t> relevant_offsets, relevant_lens;
    // Collect any fragments and their relevant metadata
    while (this->lines[true_idx].more_frag)
    {
        relevant_lens.push_back(this->lines[true_idx].len);
        relevant_offsets.push_back(this->lines[true_idx].offset);
        true_idx += 2; // Fragments only exist because true_idx + 1 is non-display line
    }
    relevant_lens.push_back(this->lines[true_idx].len);
    relevant_offsets.push_back(this->lines[true_idx].offset);

    std::size_t len = 1;
    for (std::size_t i = 0; i < relevant_lens.size(); i++)
    {
        len += relevant_lens[i];
    }

    char *line = (char *)malloc(len);
    if (!line)
    {
        return nullptr;
    }
    std::size_t pos = 0;
    for (std::size_t i = 0; i < relevant_lens.size(); i++)
    {
        memcpy(line + pos, this->mapped_memory + relevant_offsets[i], relevant_lens[i]);
        pos += relevant_lens[i];
    }
    line[len - 1] = 0;
    return line;
}

std::size_t conioHistory::get_offset() const
{
    return this->current_offset;
}

bool conioHistory::get_display() const
{
    return this->display;
}

void conioHistory::set_display(bool _display)
{
    this->display = _display;
}

std::size_t conioHistory::get_bottom_display() const
{
    return this->bottom_display;
}

void conioHistory::set_bottom_display(std::size_t _bottom_display)
{
    this->bottom_display = _bottom_display;
}

std::size_t conioHistory::get_cur_bottom_display() const
{
    return this->cur_bottom_display;
}

void conioHistory::set_cur_bottom_display(std::size_t _cur_bottom_display)
{
    this->cur_bottom_display = _cur_bottom_display;
}

bool conioHistory::is_active() const
{
    return this->active;
}
#endif