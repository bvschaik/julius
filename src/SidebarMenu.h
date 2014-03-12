#ifndef SIDEBAR_H
#define SIDEBAR_H

void SidebarMenu_enableBuildingMenuItems();

void SidebarMenu_enableAllBuildingMenuItems();

int SidebarMenu_countBuildingMenuItems(int submenu);

int SidebarMenu_getNextItemIndex(int submenu, int currentItem);

int SidebarMenu_getBuildingId(int submenu, int item);

#endif
