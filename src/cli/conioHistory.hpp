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
    char *end;
    // Additional metadata may be necessary

    CONIO_history_line_(const char *b, char* e) : beginning(b), end(e) {}
} CONIO_hist_line;

#ifdef __cplusplus
extern "C"
{
#endif

    void init_file_mapping(void);

    void resize_file(void);

    void deinit_file_mapping(void);

    void append_line(const std::string &line, bool NL_exists);
}
extern std::vector<CONIO_hist_line> Lines;
#endif