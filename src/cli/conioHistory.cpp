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

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdlib>

char *mapped_memory = nullptr;
size_t file_size = INITIAL_FILE_SIZE;
size_t current_offset = 0;
int fd;
std::vector<CONIO_hist_line> Lines;

extern "C" 
{
    void init_file_mapping()
    {
        const char *env_path = std::getenv("E4E_SF_CONIO_HISTORY");
        env_path = env_path ? env_path : "/code/build/history.log";

        fd = open(env_path, O_RDWR | O_CREAT | O_TRUNC, 0666);
        if (fd < 0)
        {
            perror("open");
            exit(1);
        }

        // Set initial file size
        ftruncate(fd, INITIAL_FILE_SIZE);

        // Map the file to memory
        mapped_memory = (char *)mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (mapped_memory == MAP_FAILED)
        {
            perror("mmap");
            close(fd);
            exit(1);
        }
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
        file_size *= FILE_RESIZE_FACTOR;

        // Resize the file on disk
        if (ftruncate(fd, file_size) < 0)
        {
            perror("resize");
            close(fd);
            exit(1);
        }

        // Unmap old memory and remap the file to the new size
        if (munmap(mapped_memory, file_size / FILE_RESIZE_FACTOR) == -1)
        {
            perror("unmap");
            close(fd);
            exit(1);
        }

        mapped_memory = (char *)mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (mapped_memory == MAP_FAILED)
        {
            perror("remap");
            close(fd);
            exit(1);
        }
    }

    void append_line(const std::string &line, bool NL_exists)
    {
        // Check if there's enough space, else resize
        if (current_offset + line.size() + 2 >= file_size)
        {
            resize_file();
        }

        char *current_pos = mapped_memory + current_offset;

        strncpy(current_pos, line.c_str(), line.size());
        current_offset += line.size();

        Lines.push_back(CONIO_hist_line(current_pos, mapped_memory + current_offset - 1));

        if (NL_exists)
        {
            mapped_memory[current_offset++] = '\n';
        }

        msync(mapped_memory, file_size, MS_SYNC);
    }
}