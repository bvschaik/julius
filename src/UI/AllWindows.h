#ifndef UI_ALLWINDOWS_H
#define UI_ALLWINDOWS_H

#include "Tooltip.h"

#include "graphics/window.h"

void UI_BuildingMenu_init(int submenu);
void UI_BuildingMenu_drawSidebarImage(int xOffset, int forceDraw);
void UI_BuildingMenu_drawBackground();
void UI_BuildingMenu_drawForeground();
void UI_BuildingMenu_handleMouse(const mouse *m);

#endif
