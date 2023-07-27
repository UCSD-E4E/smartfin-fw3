#include "menu.hpp"

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