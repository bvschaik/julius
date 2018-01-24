#ifndef UI_ALLWINDOWS_H
#define UI_ALLWINDOWS_H

#include "Tooltip.h"

#include "graphics/window.h"

void UI_LaborPriorityDialog_drawBackground();
void UI_LaborPriorityDialog_drawForeground();
void UI_LaborPriorityDialog_handleMouse(const mouse *m);
void UI_LaborPriorityDialog_getTooltip(struct TooltipContext *c);

void UI_TradePricesDialog_drawBackground();
void UI_TradePricesDialog_handleMouse(const mouse *m);
void UI_TradePricesDialog_getTooltip(struct TooltipContext *c);

void UI_ResourceSettingsDialog_drawBackground();
void UI_ResourceSettingsDialog_drawForeground();
void UI_ResourceSettingsDialog_handleMouse(const mouse *m);

void UI_PlayerMessageList_init();
void UI_PlayerMessageList_drawBackground();
void UI_PlayerMessageList_drawForeground();
void UI_PlayerMessageList_handleMouse(const mouse *m);
void UI_PlayerMessageList_getTooltip(struct TooltipContext *c);

void UI_OverlayMenu_init();
void UI_OverlayMenu_drawBackground();
void UI_OverlayMenu_drawForeground();
void UI_OverlayMenu_handleMouse(const mouse *m);

void UI_BuildingMenu_init(int submenu);
void UI_BuildingMenu_drawSidebarImage(int xOffset, int forceDraw);
void UI_BuildingMenu_drawBackground();
void UI_BuildingMenu_drawForeground();
void UI_BuildingMenu_handleMouse(const mouse *m);

#endif
