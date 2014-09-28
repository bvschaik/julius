#ifndef UI_SIDEBAR_H
#define UI_SIDEBAR_H

void UI_Sidebar_setLastAdvisor(int advisor);

void UI_Sidebar_drawBackground();

int UI_Sidebar_handleMouse();
void UI_Sidebar_handleMouseBuildButtons();

void UI_Sidebar_requestMinimapRefresh();
void UI_Sidebar_enableBuildingButtons();

void UI_SlidingSidebar_drawBackground();
void UI_SlidingSidebar_drawForeground();

#endif
