#include "menu.hpp"
#include "product.hpp"
#include "cli/conio.hpp"
#include "consts.hpp"

#include <string.h>
#include <ctype.h>

char userInput[SF_CLI_MAX_CMD_LEN];

const Menu_t* MNU_findCommand(const char* input, const Menu_t* pMenu)
{
    int cmd_value = atoi(input);

    if (cmd_value == 0)
    {
        return nullptr;
    }
    for (; pMenu->cmd; pMenu++)
    {
        if (cmd_value == pMenu->cmd)
        {
            return pMenu;
        }
    }
    return nullptr;
}

void MNU_displayMenu(const Menu_t* pMenu)
{
    for(; pMenu->cmd; pMenu++)
    {
        SF_OSAL_printf("%6d: %s" __NL__, pMenu->cmd, pMenu->fnName);
    }
}

int MNU_executeMenu(const Menu_t *pMenu)
{
    const Menu_t* cmd;
    int retval;

    MNU_displayMenu(pMenu);

    SF_OSAL_printf(__NL__);
    while (1)
    {
        SF_OSAL_printf(">");
        memset(userInput, 0, SF_CLI_MAX_CMD_LEN);
        retval = SF_OSAL_getline(userInput, SF_CLI_MAX_CMD_LEN);
        if (retval == -1)
        {
            // Abort due to USB terminal disconnect
            return -1;
        }

        // Trim leading and trailing whitespace
        char *trimmedInput = userInput;
        while(isspace((unsigned char)*trimmedInput)) trimmedInput++;
        
        char *end = trimmedInput + strlen(trimmedInput) - 1;
        while(end > trimmedInput && isspace((unsigned char)*end)) end--;
        *(end + 1) = 0;

        if (strlen(trimmedInput) == 0)
        {
            continue;
        }

        if (trimmedInput[0] == 'q' && strlen(trimmedInput) == 1)
        {
            // Escape
            SF_OSAL_printf(__NL__);
            break;
        }
        if (trimmedInput[0] == '?' && strlen(trimmedInput) == 1)
        {
            // Help
            MNU_displayMenu(pMenu);
            SF_OSAL_printf(__NL__);
            continue;
        }

        cmd = MNU_findCommand(trimmedInput, pMenu);

        if (!cmd)
        {
            SF_OSAL_printf("Unknown command" __NL__);
            MNU_displayMenu(pMenu);
            continue;
        }

        if (MENU_CMD == cmd->menuType)
        {
            cmd->ptr.fn();
            SF_OSAL_printf(__NL__);
        }
        else if (MENU_SUBMENU == cmd->menuType)
        {
            MNU_executeMenu(cmd->ptr.pMenu);
            MNU_displayMenu(pMenu);
        }
    }
    return 0;
}