#include "fileCLI.hpp"

#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "consts.hpp"

#include "Particle.h"

#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

static char path_buffer[FILE_CLI_MAX_PATH_LEN];

FileCLI::menu_t FileCLI::fsExplorerMenu[] = 
{
    {'l', &FileCLI::list_dir},
    {'q', &FileCLI::exit},
    {'\0', NULL}
};


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
        FLOG_AddError(FLOG_FS_OPENDIR_FAIL, (uint32_t)this->dir_stack[this->current_dir]);
        SF_OSAL_printf("Failed to open root" __NL__);
        return;
    }

    while (this->run)
    {
        FLOG_AddError(FLOG_DEBUG, 5);
        SF_OSAL_printf(":>");
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
        this->path_stack[this->current_dir] = dirent->d_name;
        const char* path = buildPath();
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

const char* FileCLI::buildPath(void)
{
    size_t path_buffer_idx = 0;
    int dir_idx = 0;
    for(; dir_idx < this->current_dir; dir_idx++)
    {
        strcpy(path_buffer, PATH_SEP);
        strncpy(path_buffer + path_buffer_idx + 1,
                this->path_stack[this->current_dir],
                FILE_CLI_MAX_PATH_LEN - path_buffer_idx - 1);
        path_buffer_idx = strlen(path_buffer);
    }
    return path_buffer;
}