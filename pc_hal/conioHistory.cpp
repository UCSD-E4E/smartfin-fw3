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
size_t cur_bottom = 0;
size_t bottom_idx = 0;
bool coalesce = false;

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
    // Check if there's enough space, else resize
    if (current_offset + line.size() + 2 >= file_size)
    {
        resize_file();
    }

    strncpy(mapped_memory + current_offset, line.c_str(), line.size());
    current_offset += line.size();
    Lines[bottom_idx].len += line.size();

    if (NL_exists)
    {
        mapped_memory[current_offset++] = '\n';
        Lines.push_back(CONIO_hist_line(current_offset));
        cur_bottom = ++bottom_idx;
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

char *retrieve_line(const size_t line_idx)
{
    if (Lines[line_idx].len == 0)
        return nullptr;
    size_t len = Lines[line_idx].len + 1;
    char *line = (char *)malloc(len);
    strncpy(line, mapped_memory + Lines[line_idx].offset, len);
    line[len - 1] = 0;
    return line;
}

size_t get_offset()
{
    return current_offset;
}
#endif