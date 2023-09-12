#include "fileCLI.hpp"

#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "consts.hpp"
#include "cellular/encoding/base64.h"
#include "cellular/encoding/base85.h"
#include "product.hpp"
#include "system.hpp"

#include "Particle.h"

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syslimits.h>

static char path_buffer[PATH_MAX];

FileCLI::menu_t FileCLI::fsExplorerMenu[] = 
{
    {'c', &FileCLI::change_dir},
    {'d', &FileCLI::dumpBase85},
    {'h', &FileCLI::dumpHex},
    {'l', &FileCLI::list_dir},
    {'p', &FileCLI::print_dir},
    {'q', &FileCLI::exit},
    {'r', &FileCLI::deleteFile},
    {'?', &FileCLI::print_help},
    {'\0', NULL}
};

void FileCLI::print_help(void)
{
    SF_OSAL_printf("%c\t%s" __NL__, 'p', "Print Working Directory");
    SF_OSAL_printf("%c\t%s" __NL__, 'l', "List Dir");
    SF_OSAL_printf("%c\t%s" __NL__, 'c', "Change Directory");
    SF_OSAL_printf("%c\t%s" __NL__, 'r', "Remove File/Directory");
    SF_OSAL_printf("%c\t%s" __NL__, 'd', "Base85/64 Dump");
    SF_OSAL_printf("%c\t%s" __NL__, 'h', "Hex Dump");
    SF_OSAL_printf("%c\t%s" __NL__, 'q', "Quit");
    SF_OSAL_printf("%c\t%s" __NL__, '?', "Displays this help text");
}

void FileCLI::execute(void)
{
    char input_buffer[FILE_CLI_INPUT_BUFFER_LEN];
    menu_t* cmd;
    void (FileCLI::*fn)(void);

    this->run = 1;
    this->current_dir = 0;
    this->dir_stack[this->current_dir] = opendir("/");
    if(NULL == this->dir_stack[this->current_dir])
    {
        FLOG_AddError(FLOG_FS_OPENDIR_FAIL,
                      (uint32_t)this->dir_stack[this->current_dir]);
        SF_OSAL_printf("Failed to open root" __NL__);
        return;
    }

    while (this->run)
    {
        FLOG_AddError(FLOG_DEBUG, 5);
        SF_OSAL_printf(":>");
        memset(input_buffer, 0, FILE_CLI_INPUT_BUFFER_LEN);
        getline(input_buffer, FILE_CLI_INPUT_BUFFER_LEN);
        cmd = findCommand(input_buffer);
        if (!cmd)
        {
            SF_OSAL_printf("Unknown command" __NL__);
        }
        else
        {
            fn = cmd->fn;
            ((*this).*fn)();
        }
        FLOG_AddError(FLOG_DEBUG, 1);
    }
    for(;this->current_dir >= 0; this->current_dir--)
    {
        FLOG_AddError(FLOG_DEBUG, 2);
        closedir(this->dir_stack[this->current_dir]);
        FLOG_AddError(FLOG_DEBUG, 3);
    }
    FLOG_AddError(FLOG_DEBUG, 4);
}

void FileCLI::list_dir(void)
{
    DIR* cwd = this->dir_stack[this->current_dir];
    struct dirent* dirent;
    char f_type;
    struct stat file_stats;

    while ((dirent = readdir(cwd)))
    {
        switch(dirent->d_type)
        {
            default:
            case DT_REG:
                f_type = ' ';
                break;
            case DT_DIR:
                f_type = 'd';
                break;
        }
        strncpy(this->path_stack[this->current_dir], dirent->d_name, NAME_MAX);
        const char* path = buildPath(true);
        if(stat(path, &file_stats))
        {
            FLOG_AddError(FLOG_FS_STAT_FAIL, errno);
            SF_OSAL_printf("Failed to stat %s" __NL__, path);
        }
        SF_OSAL_printf("%c %8d %-16s" __NL__,
                        f_type,
                        file_stats.st_size,
                        dirent->d_name);
    }
    rewinddir(cwd);
    memset(this->path_stack[this->current_dir], 0, NAME_MAX);
}

FileCLI::menu_t* FileCLI::findCommand(const char* const cmd)
{
    menu_t* pMenu;

    for (pMenu = fsExplorerMenu; pMenu->cmd; pMenu++)
    {
        if (pMenu->cmd == cmd[0])
        {
            return pMenu;
        }
    }
    return nullptr;
}
void FileCLI::exit(void)
{
    this->run = 0;
    FLOG_AddError(FLOG_DEBUG, 0);
}

const char* FileCLI::buildPath(bool is_dir)
{
    size_t path_buffer_idx = 0;
    int dir_idx = 0;

    memset(path_buffer, 0, PATH_MAX);


    for(; 
        is_dir ? (dir_idx <= this->current_dir) : (dir_idx < this->current_dir);
        dir_idx++)
    {

        strcpy(path_buffer + path_buffer_idx, PATH_SEP);
        strncpy(path_buffer + path_buffer_idx + 1,
                this->path_stack[dir_idx],
                PATH_MAX - path_buffer_idx - 1);
        path_buffer_idx = strlen(path_buffer);
    }
    return path_buffer;
}

void FileCLI::change_dir(void)
{
    char input_buffer[FILE_CLI_INPUT_BUFFER_LEN];
    DIR* cwd = this->dir_stack[this->current_dir];
    struct dirent* dirent;
    long idx = 0;
    int cmd_val;
    const char* path;
    long same_dir_idx = 0, prev_dir_idx = 0;

    idx = telldir(cwd);
    while ((dirent = readdir(cwd)))
    {
        if(dirent->d_type == DT_REG)
        {
            idx = telldir(cwd);
            continue;
        }
        SF_OSAL_printf("%d: %-16s" __NL__,
                        idx,
                        dirent->d_name);
        if(strcmp(dirent->d_name, ".") == 0)
        {
            same_dir_idx = idx;
        }
        if(strcmp(dirent->d_name, "..") == 0)
        {
            prev_dir_idx = idx;
        }
        idx = telldir(cwd);
    }
    rewinddir(cwd);
    SF_OSAL_printf(__NL__);
    SF_OSAL_printf("Enter the number of the directory to change to: ");
    getline(input_buffer, FILE_CLI_INPUT_BUFFER_LEN);
    cmd_val = atoi(input_buffer);

    if(cmd_val == same_dir_idx)
    {
        return;
    }
    if(cmd_val == prev_dir_idx && this->current_dir != 0)
    {
        memset(this->path_stack[this->current_dir - 1], 0, NAME_MAX);
        closedir(this->dir_stack[this->current_dir]);
        this->current_dir--;
        return;
    }

    seekdir(cwd, cmd_val);
    dirent = readdir(cwd);
    rewinddir(cwd);
    strncpy(this->path_stack[this->current_dir], dirent->d_name, NAME_MAX);
    this->current_dir++;
    path = buildPath(true);
    this->dir_stack[this->current_dir] = opendir(path);
}

FileCLI::FileCLI(void)
{
    memset(this->path_stack,
           0,
           sizeof(char) * FILE_CLI_MAX_DIR_DEPTH * NAME_MAX);
}

void FileCLI::print_dir(void)
{
    const char* path;
    path = buildPath(true);
    SF_OSAL_printf("%s" __NL__, path);
}

void FileCLI::deleteFile(void)
{
    char input_buffer[FILE_CLI_INPUT_BUFFER_LEN];
    DIR* cwd = this->dir_stack[this->current_dir];
    struct dirent* dirent;
    char f_type;
    long idx;
    int cmd_val;
    const char* path;

    idx = telldir(cwd);
    while ((dirent = readdir(cwd)))
    {
        switch(dirent->d_type)
        {
            default:
            case DT_REG:
                f_type = ' ';
                break;
            case DT_DIR:
                f_type = 'd';
                break;
        }
        strncpy(this->path_stack[this->current_dir], dirent->d_name, NAME_MAX);
        SF_OSAL_printf("%d: %c %-16s" __NL__,
                        idx,
                        f_type,
                        dirent->d_name);
        idx = telldir(cwd);
    }
    rewinddir(cwd);
    memset(this->path_stack[this->current_dir], 0, NAME_MAX);

    SF_OSAL_printf("Enter the number of the file to remove: ");
    getline(input_buffer, FILE_CLI_INPUT_BUFFER_LEN);
    cmd_val = atoi(input_buffer);

    seekdir(cwd, cmd_val);
    dirent = readdir(cwd);
    rewinddir(cwd);
    strncpy(this->path_stack[this->current_dir], dirent->d_name, NAME_MAX);
    this->current_dir++;
    path = buildPath(false);
    if(unlink(path))
    {
        SF_OSAL_printf("Failed to unlink file: %s" __NL__, strerror(errno));
    }
    this->current_dir--;
    memset(this->path_stack[this->current_dir], 0, NAME_MAX);
}


void hexdump(int fp, size_t file_len)
{
    #define BYTES_PER_LINE 16
    size_t file_idx = 0;
    uint8_t byte_buffer[BYTES_PER_LINE + 1];
    int bytes_read = 0;
    size_t line_idx = 0;

    for (file_idx = 0; file_idx < file_len; file_idx += bytes_read)
    {
        SF_OSAL_printf("%08x  ", file_idx);
        memset(byte_buffer, 0, BYTES_PER_LINE + 1);
        bytes_read = read(fp, byte_buffer, BYTES_PER_LINE);
        if (-1 == bytes_read)
        {
            SF_OSAL_printf(__NL__ "Error in reading: %s" __NL__, 
                           strerror(errno));
            return;
        }
        // Print hex
        for(line_idx = 0; line_idx < (size_t) bytes_read; line_idx++)
        {
            SF_OSAL_printf("%02hx ", byte_buffer[line_idx]);
            if (7 == line_idx)
            {
                SF_OSAL_printf(" ");
            }
            // Make the byte printable
            if (!isprint(byte_buffer[line_idx]))
            {
                byte_buffer[line_idx] = (uint8_t) '.';
            }
        }
        // fill line
        for(;line_idx < BYTES_PER_LINE; line_idx++)
        {
            SF_OSAL_printf("   ");
            if (7 == line_idx)
            {
                SF_OSAL_printf(" ");
            }
        }
        SF_OSAL_printf(" |%s|" __NL__, (const char*) byte_buffer);
    }
    SF_OSAL_printf("0x%08x" __NL__, file_idx);

}

void FileCLI::dumpHex(void)
{
    char input_buffer[FILE_CLI_INPUT_BUFFER_LEN];
    DIR* cwd = this->dir_stack[this->current_dir];
    struct dirent* dirent;
    long idx;
    int cmd_val;
    const char* path;
    int fp;
    struct stat fstats;

    idx = telldir(cwd);
    while ((dirent = readdir(cwd)))
    {
        if (dirent->d_type != DT_REG)
        {
            idx = telldir(cwd);
            continue;
        }
        strncpy(this->path_stack[this->current_dir], dirent->d_name, NAME_MAX);
        SF_OSAL_printf("%d: %-16s" __NL__,
                        idx,
                        dirent->d_name);
        idx = telldir(cwd);
    }
    rewinddir(cwd);
    memset(this->path_stack[this->current_dir], 0, NAME_MAX);

    SF_OSAL_printf("Enter the number of the file to hexdump: ");
    getline(input_buffer, FILE_CLI_INPUT_BUFFER_LEN);
    cmd_val = atoi(input_buffer);

    seekdir(cwd, cmd_val);
    dirent = readdir(cwd);
    rewinddir(cwd);
    strncpy(this->path_stack[this->current_dir], dirent->d_name, NAME_MAX);
    this->current_dir++;
    path = buildPath(false);

    fp = open(path, O_RDONLY);
    if(-1 == fp)
    {
        SF_OSAL_printf("Unable to open %s: %s" __NL__, path, strerror(errno));
        return;
    }

    if(fstat(fp, &fstats))
    {
        SF_OSAL_printf("Unable to stat file: %s" __NL__, strerror(errno));
        close(fp);
        return;
    }

    hexdump(fp, fstats.st_size);

    close(fp);
}

void base85dump(int fp, size_t file_len)
{
    size_t file_idx = 0, bytes_read;
    uint8_t byte_buffer[512];
    char encoded_buffer[1024];
    size_t totalEncodedLen = 0;
    size_t n_packets = 0;
    size_t encodedLen = 0;
    for(file_idx = 0; file_idx < file_len; file_idx += bytes_read)
    {
        bytes_read = read(fp, byte_buffer, SF_BLOCK_SIZE);
        #if SF_UPLOAD_ENCODING == SF_UPLOAD_BASE85
            encodedLen = bintob85(encoded_buffer, byte_buffer, bytes_read) - encodedBuffer;
        #elif SF_UPLOAD_ENCODING == SF_UPLOAD_BASE64
            encodedLen = 1024;
            b64_encode(byte_buffer, bytes_read, encoded_buffer, &encodedLen);
        #elif SF_UPLOAD_ENCODING == SF_UPLOAD_BASE64URL
            encodedLen = 1024;
            urlsafe_b64_encode(byte_buffer, bytes_read, encoded_buffer, &encodedLen);
        #endif
        totalEncodedLen += encodedLen;
        SF_OSAL_printf("%s" __NL__, encoded_buffer);
        n_packets++;
    }

    SF_OSAL_printf(__NL__ "%d chars of encoded data" __NL__, totalEncodedLen);
    SF_OSAL_printf("%d packets" __NL__, n_packets);
}

void FileCLI::dumpBase85(void)
{
    char input_buffer[FILE_CLI_INPUT_BUFFER_LEN];
    DIR* cwd = this->dir_stack[this->current_dir];
    struct dirent* dirent;
    long idx;
    int cmd_val;
    const char* path;
    int fp;
    struct stat fstats;

    idx = telldir(cwd);
    while ((dirent = readdir(cwd)))
    {
        if (dirent->d_type != DT_REG)
        {
            idx = telldir(cwd);
            continue;
        }
        strncpy(this->path_stack[this->current_dir], dirent->d_name, NAME_MAX);
        SF_OSAL_printf("%d: %-16s" __NL__,
                        idx,
                        dirent->d_name);
        idx = telldir(cwd);
    }
    rewinddir(cwd);
    memset(this->path_stack[this->current_dir], 0, NAME_MAX);

    SF_OSAL_printf("Enter the number of the file to dump: ");
    getline(input_buffer, FILE_CLI_INPUT_BUFFER_LEN);
    cmd_val = atoi(input_buffer);

    seekdir(cwd, cmd_val);
    dirent = readdir(cwd);
    rewinddir(cwd);
    strncpy(this->path_stack[this->current_dir], dirent->d_name, NAME_MAX);
    this->current_dir++;
    path = buildPath(false);

    fp = open(path, O_RDONLY);
    if(-1 == fp)
    {
        SF_OSAL_printf("Unable to open %s: %s" __NL__, path, strerror(errno));
        return;
    }

    if(fstat(fp, &fstats))
    {
        SF_OSAL_printf("Unable to stat file: %s" __NL__, strerror(errno));
        close(fp);
        return;
    }

    SF_OSAL_printf("Publish Header: %s-%s" __NL__,
                   pSystemDesc->deviceID,
                   dirent->d_name);
    base85dump(fp, fstats.st_size);

    close(fp);
}
