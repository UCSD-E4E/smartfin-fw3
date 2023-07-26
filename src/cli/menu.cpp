#include "menu.hpp"

const Menu_t* MNU_findCommand(const char* input, const Menu_t* pMenu)
{
    Menu_t* pCmd;
    int cmd_value = atoi(input);

    if (cmd_value == 0)
    {
        return nullptr;
    }
    for (pCmd = pMenu; pCmd->cmd; pCmd++)
    {
        if (cmd_value == pCmd->cmd)
        {
            return pCmd;
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