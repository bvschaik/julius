#ifndef SIDEBAR_H
#define SIDEBAR_H

void SidebarMenu_enableBuildingButtons();
void SidebarMenu_enableBuildingMenuItems();
void SidebarMenu_enableBuildingMenuItemsAndButtons();

void SidebarMenu_enableAllBuildingMenuItems();

int SidebarMenu_countBuildingMenuItems(int submenu);

int SidebarMenu_getNextBuildingItemIndex(int submenu, int currentItem);

int SidebarMenu_getBuildingType(int submenu, int item);

#endif
