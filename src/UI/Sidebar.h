#ifndef UI_SIDEBAR_H
#define UI_SIDEBAR_H

#include "input/mouse.h"

void UI_Sidebar_drawBackground();
void UI_Sidebar_drawForeground();

int UI_Sidebar_handleMouse(const mouse *m);
void UI_Sidebar_handleMouseBuildButtons(const mouse *m);
int UI_Sidebar_getTooltipText();

void UI_Sidebar_drawMinimap(int force);
void UI_Sidebar_requestMinimapRefresh();
void UI_Sidebar_enableBuildingButtons();

void UI_SlidingSidebar_drawBackground();
void UI_SlidingSidebar_drawForeground();

#endif
