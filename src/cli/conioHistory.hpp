#ifndef __CONIOHISTORY_HPP__
#define __CONIOHISTORY_HPP__

#include <cstring>
#include <string>
#include <vector>
#define INITIAL_FILE_SIZE 4096
#define FILE_RESIZE_FACTOR 2

typedef struct CONIO_history_line_
{
    const char *beginning;
    size_t len;
    // Additional metadata may be necessary

    CONIO_history_line_(const char *b, size_t l) : beginning(b), len(l) {}
} CONIO_hist_line;

#ifdef __cplusplus
extern "C"
{
#endif

    void init_file_mapping(void);

    void resize_file(void);

    void deinit_file_mapping(void);

    void write_line(const std::string &line, const bool NL_exists);
}
extern size_t cur_bottom;
extern size_t bottom_idx;
#endif