#ifndef __MENU_H__
#define __MENU_H__

#include "conio.hpp"
#include "consts.hpp"
#include <cstdlib>

/**
 * @brief Menu Type
 * 
 */
typedef enum MenuType_
{
    /**
     * @brief Null Menu
     * 
     */
    MENU_NULL,
    /**
     * @brief Command Item
     * 
     */
    MENU_CMD,
    /**
     * @brief Submenu
     * 
     */
    MENU_SUBMENU,
}MenuType_t;

/**
 * @brief Menu structure
 * 
 */
typedef const struct Menu_
{
    /**
     * @brief Command ID - this must be non-zero
     * 
     */
    const int cmd;
    /**
     * @brief Command name/description
     * 
     */
    const char* const fnName;
    /**
     * @brief Stores a command function or submenu pointer
     */
    union {
        /**
         * @brief Command function pointer
         * 
         */
        void (*fn)(void);
        /**
         * @brief Pointer to submenu
         * 
         */
        const struct Menu_* pMenu;
    } ptr;
    /**
     * @brief Type of menu item
     */
    MenuType_t menuType;
}Menu_t;

/**
 * @brief Finds the specified command in the specified menu
 * 
 * @param input Input string
 * @param pMenu Menu structure
 * @return const Menu_t* Discovered menu if found, otherwise nullptr
 */
const Menu_t* MNU_findCommand(const char* input, const Menu_t* pMenu);

/**
 * @brief Displays the menu to the terminal
 * 
 * @param pMenu Menu structure to display
 */
void MNU_displayMenu(const Menu_t* pMenu);

/**
 * @brief Executes the given menu
 * 
 * @param pMenu Menu to execute
 */
void MNU_executeMenu(const Menu_t* pMenu);
#endif