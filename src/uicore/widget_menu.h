#ifndef WIDGET_MENU_H
#define WIDGET_MENU_H

struct MenuBarItem;
struct mouse;

namespace Widget
{

struct Menu
{
    static void drawMenuBar(MenuBarItem *items, int numItems);
    static void drawSubMenu(MenuBarItem *menu, int focusSubMenu);

    static int handleMenuBar(const mouse *m, MenuBarItem *items, int numItems, int *focusMenuId);
    static int handleMenuItem(const mouse *m, MenuBarItem *menu, int *focusSubMenuId);
};

}

#endif
