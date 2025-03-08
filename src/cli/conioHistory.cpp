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
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdlib>

char *mapped_memory = nullptr;
size_t file_size = INITIAL_FILE_SIZE;
size_t current_offset = 0;
int fd;
std::vector<CONIO_hist_line> Lines;
size_t cur_bottom = 0;
size_t bottom_idx = 0;
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

        Lines.push_back(CONIO_hist_line(mapped_memory, 0)); // Initialize the first line
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
            Lines.push_back(CONIO_hist_line(mapped_memory + current_offset, 0));
            mapped_memory[current_offset++] = '\n';
            bottom_idx++;
            cur_bottom++;
        }

        msync(mapped_memory, file_size, MS_SYNC);
    }
}
#endif