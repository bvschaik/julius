#ifndef UI_ALLWINDOWS_H
#define UI_ALLWINDOWS_H

#include "Tooltip.h"
#include "Window.h"

#include "input/mouse.h"

void UI_Logo_init();
void UI_Logo_drawBackground();
void UI_Logo_handleMouse(const mouse *m);

void UI_MainMenu_drawBackground();
void UI_MainMenu_drawForeground();
void UI_MainMenu_handleMouse(const mouse *m);

void UI_NewCareerDialog_init();
void UI_NewCareerDialog_drawBackground();
void UI_NewCareerDialog_drawForeground();
void UI_NewCareerDialog_handleMouse(const mouse *m);

void UI_CCKSelection_init();
void UI_CCKSelection_drawBackground();
void UI_CCKSelection_drawForeground();
void UI_CCKSelection_handleMouse(const mouse *m);

void UI_DifficultyOptions_drawForeground();
void UI_DifficultyOptions_handleMouse(const mouse *m);

void UI_DisplayOptions_drawForeground();
void UI_DisplayOptions_handleMouse(const mouse *m);

void UI_SoundOptions_init();
void UI_SoundOptions_drawForeground();
void UI_SoundOptions_handleMouse(const mouse *m);

void UI_SpeedOptions_init();
void UI_SpeedOptions_drawForeground();
void UI_SpeedOptions_handleMouse(const mouse *m);

void UI_Advisors_init();
void UI_Advisors_drawBackground();
void UI_Advisors_drawForeground();
void UI_Advisors_handleMouse(const mouse *m);
void UI_Advisors_getTooltip(struct TooltipContext *c);

void UI_LaborPriorityDialog_drawBackground();
void UI_LaborPriorityDialog_drawForeground();
void UI_LaborPriorityDialog_handleMouse(const mouse *m);
void UI_LaborPriorityDialog_getTooltip(struct TooltipContext *c);

void UI_SetSalaryDialog_drawBackground();
void UI_SetSalaryDialog_drawForeground();
void UI_SetSalaryDialog_handleMouse(const mouse *m);

void UI_DonateToCityDialog_init();
void UI_DonateToCityDialog_drawBackground();
void UI_DonateToCityDialog_drawForeground();
void UI_DonateToCityDialog_handleMouse(const mouse *m);
void UI_DonateToCityDialog_getTooltip(struct TooltipContext *c);

void UI_SendGiftToCaesarDialog_init();
void UI_SendGiftToCaesarDialog_drawBackground();
void UI_SendGiftToCaesarDialog_drawForeground();
void UI_SendGiftToCaesarDialog_handleMouse(const mouse *m);

void UI_TradePricesDialog_drawBackground();
void UI_TradePricesDialog_handleMouse(const mouse *m);
void UI_TradePricesDialog_getTooltip(struct TooltipContext *c);

void UI_ResourceSettingsDialog_drawBackground();
void UI_ResourceSettingsDialog_drawForeground();
void UI_ResourceSettingsDialog_handleMouse(const mouse *m);

void UI_HoldFestivalDialog_drawBackground();
void UI_HoldFestivalDialog_drawForeground();
void UI_HoldFestivalDialog_handleMouse(const mouse *m);
void UI_HoldFestivalDialog_getTooltip(struct TooltipContext *c);

void UI_City_drawBackground();
void UI_City_drawForeground();
void UI_City_drawForegroundMilitary();
void UI_City_drawCity();
void UI_City_drawPausedAndTimeLeft();
void UI_City_handleMouse(const mouse *m);
void UI_City_handleMouseMilitary(const mouse *m);
void UI_City_getTooltip(struct TooltipContext *c);

void UI_Empire_init();
void UI_Empire_drawBackground();
void UI_Empire_drawForeground();
void UI_Empire_handleMouse(const mouse *m);
void UI_EmpireMap_getTooltip(struct TooltipContext *c);

void UI_TradeOpenedDialog_drawBackground();
void UI_TradeOpenedDialog_drawForeground();
void UI_TradeOpenedDialog_handleMouse(const mouse *m);

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
