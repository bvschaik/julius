#ifndef UI_ALLWINDOWS_H
#define UI_ALLWINDOWS_H

#include "Window.h"

void UI_MainMenu_drawBackground();
void UI_MainMenu_drawForeground();
void UI_MainMenu_handleMouse();

void UI_NewCareerDialog_drawBackground();
void UI_NewCareerDialog_drawForeground();
void UI_NewCareerDialog_handleMouse();

void UI_CCKSelection_init();
void UI_CCKSelection_drawBackground();
void UI_CCKSelection_drawForeground();
void UI_CCKSelection_handleMouse();

void UI_DifficultyOptions_drawForeground();
void UI_DifficultyOptions_handleMouse();

void UI_DisplayOptions_drawForeground();

void UI_SoundOptions_init();
void UI_SoundOptions_drawForeground();
void UI_SoundOptions_handleMouse();

void UI_SpeedOptions_init();
void UI_SpeedOptions_drawForeground();
void UI_SpeedOptions_handleMouse();

void UI_Advisors_setAdvisor(int advisor);
void UI_Advisors_init();
void UI_Advisors_drawBackground();
void UI_Advisors_drawForeground();
void UI_Advisors_handleMouse();

void UI_LaborPriorityDialog_drawBackground();
void UI_LaborPriorityDialog_drawForeground();
void UI_LaborPriorityDialog_handleMouse();

void UI_SetSalaryDialog_drawBackground();
void UI_SetSalaryDialog_drawForeground();
void UI_SetSalaryDialog_handleMouse();

void UI_DonateToCityDialog_init();
void UI_DonateToCityDialog_drawBackground();
void UI_DonateToCityDialog_drawForeground();
void UI_DonateToCityDialog_handleMouse();

void UI_SendGiftToCaesarDialog_init();
void UI_SendGiftToCaesarDialog_drawBackground();
void UI_SendGiftToCaesarDialog_drawForeground();
void UI_SendGiftToCaesarDialog_handleMouse();

void UI_TradePricesDialog_drawBackground();
void UI_TradePricesDialog_handleMouse();

void UI_ResourceSettingsDialog_drawBackground();
void UI_ResourceSettingsDialog_drawForeground();
void UI_ResourceSettingsDialog_handleMouse();

void UI_HoldFestivalDialog_drawBackground();
void UI_HoldFestivalDialog_drawForeground();
void UI_HoldFestivalDialog_handleMouse();

void UI_City_drawBackground();
void UI_City_drawForeground();
void UI_City_drawCity();
void UI_City_handleMouse();

void UI_Empire_drawBackground();
void UI_Empire_drawForeground();
void UI_Empire_handleMouse();

void UI_TradeOpenedDialog_drawBackground();
void UI_TradeOpenedDialog_drawForeground();
void UI_TradeOpenedDialog_handleMouse();

void UI_PlayerMessageList_init();
void UI_PlayerMessageList_drawBackground();
void UI_PlayerMessageList_drawForeground();
void UI_PlayerMessageList_handleMouse();

void UI_OverlayMenu_init();
void UI_OverlayMenu_drawBackground();
void UI_OverlayMenu_drawForeground();
void UI_OverlayMenu_handleMouse();

void UI_BuildingMenu_init(int submenu);
void UI_BuildingMenu_drawSidebarImage(int xOffset, int forceDraw);
void UI_BuildingMenu_drawBackground();
void UI_BuildingMenu_drawForeground();
void UI_BuildingMenu_handleMouse();

void UI_Intermezzo_show(int type, WindowId nextWindowId, int timeMillis);
void UI_Intermezzo_drawBackground();
void UI_Intermezzo_handleMouse();

#endif
