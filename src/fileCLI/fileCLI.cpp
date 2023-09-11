#include "fileCLI.hpp"

#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "consts.hpp"

#include <fcntl.h>

#include "Particle.h"

#include <sys/dirent.h>


FileCLI::menu_t FileCLI::fsExplorerMenu[] = 
{
    {'l', &FileCLI::list_dir},
    {'\0', NULL}
};


void FileCLI::execute(void)
{
    char input_buffer[FILE_CLI_INPUT_BUFFER_LEN];
    menu_t* cmd;
    void (FileCLI::*fn)(void);

    this->run = 1;
    this->current_dir = opendir("/");
    if(NULL == this->current_dir)
    {
        FLOG_AddError(FLOG_FS_OPENDIR_FAIL, (uint32_t)this->current_dir);
        SF_OSAL_printf("Failed to open root" __NL__);
        return;
    }
}

void FileCLI::list_dir(void)
{

}
