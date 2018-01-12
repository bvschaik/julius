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

void UI_City_drawBackground();
void UI_City_drawForeground();
void UI_City_drawForegroundMilitary();
void UI_City_drawCity();
void UI_City_handleMouse(const mouse *m);
void UI_City_handleMouseMilitary(const mouse *m);
void UI_City_getTooltip(struct TooltipContext *c);

void UI_PlayerMessageList_resetScroll();
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

enum {
	Intermezzo_MissionBriefing = 0,
	Intermezzo_Fired = 1,
	Intermezzo_Won = 2,
};
void UI_Intermezzo_show(int type, WindowId nextWindowId, int timeMillis);
void UI_Intermezzo_drawBackground();
void UI_Intermezzo_handleMouse(const mouse *m);

void UI_MissionStart_show();

void UI_MissionStart_Selection_drawBackground();
void UI_MissionStart_Selection_drawForeground();
void UI_MissionStart_Selection_handleMouse(const mouse *m);

void UI_MissionStart_Briefing_init();
void UI_MissionStart_Briefing_drawBackground();
void UI_MissionStart_BriefingInitial_drawForeground();
void UI_MissionStart_BriefingReview_drawForeground();
void UI_MissionStart_BriefingInitial_handleMouse(const mouse *m);
void UI_MissionStart_BriefingReview_handleMouse(const mouse *m);

void UI_VictoryIntermezzo_init();

void UI_MissionEnd_drawBackground();
void UI_MissionEnd_drawForeground();
void UI_MissionEnd_handleMouse(const mouse *m);

void UI_VictoryDialog_drawBackground();
void UI_VictoryDialog_drawForeground();
void UI_VictoryDialog_handleMouse(const mouse *m);

#endif
