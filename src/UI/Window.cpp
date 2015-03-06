#include "Window.h"

#include "AllWindows.h"
#include "BuildingInfo.h"
#include "CityBuildings.h"
#include "FileDialog.h"
#include "PopupDialog.h"
#include "MessageDialog.h"
#include "Sidebar.h"
#include "Tooltip.h"
#include "TopMenu.h"
#include "Warning.h"

#include "../Cursor.h"
#include "../KeyboardInput.h"
#include "../Data/Mouse.h"

struct Window {
	void (*init)(void);
	void (*drawBackground)(void);
	void (*drawForeground)(void);
	void (*handleMouse)(void);
	void (*getTooltip)(struct TooltipContext *c);
};

static void noop()
{
}

static struct Window windows[] = {
	// 0
	{ noop, UI_MainMenu_drawBackground, UI_MainMenu_drawForeground, UI_MainMenu_handleMouse },
	{ noop, UI_City_drawBackground, UI_City_drawForeground, UI_City_handleMouse, UI_CityBuildings_getTooltip },
	{ noop, UI_PopupDialog_drawBackground, UI_PopupDialog_drawForeground, UI_PopupDialog_handleMouse },
	{ noop, UI_City_drawBackground, UI_TopMenu_drawForeground, UI_TopMenu_handleMouse },
	{ noop, noop, UI_DifficultyOptions_drawForeground, UI_DifficultyOptions_handleMouse },
	{ UI_Advisors_init, UI_Advisors_drawBackground, UI_Advisors_drawForeground, UI_Advisors_handleMouse, UI_Advisors_getTooltip },
	{ noop, UI_SetSalaryDialog_drawBackground, UI_SetSalaryDialog_drawForeground, UI_SetSalaryDialog_handleMouse },
	{ UI_DonateToCityDialog_init, UI_DonateToCityDialog_drawBackground, UI_DonateToCityDialog_drawForeground, UI_DonateToCityDialog_handleMouse, UI_DonateToCityDialog_getTooltip },
	{ UI_SendGiftToCaesarDialog_init, UI_SendGiftToCaesarDialog_drawBackground, UI_SendGiftToCaesarDialog_drawForeground, UI_SendGiftToCaesarDialog_handleMouse },
	{ noop, UI_LaborPriorityDialog_drawBackground, UI_LaborPriorityDialog_drawForeground, UI_LaborPriorityDialog_handleMouse, UI_LaborPriorityDialog_getTooltip },
	// 10
	{ noop, noop, UI_DisplayOptions_drawForeground, UI_DisplayOptions_handleMouse },
	{ UI_SoundOptions_init, noop, UI_SoundOptions_drawForeground, UI_SoundOptions_handleMouse },
	{ UI_SpeedOptions_init, noop, UI_SpeedOptions_drawForeground, UI_SpeedOptions_handleMouse },
	{ noop, UI_Empire_drawBackground, UI_Empire_drawForeground, UI_Empire_handleMouse, UI_EmpireMap_getTooltip },
	{ noop, UI_TradeOpenedDialog_drawBackground, UI_TradeOpenedDialog_drawForeground, UI_TradeOpenedDialog_handleMouse },
	{ noop, UI_HoldFestivalDialog_drawBackground, UI_HoldFestivalDialog_drawForeground, UI_HoldFestivalDialog_handleMouse, UI_HoldFestivalDialog_getTooltip },
	{ noop, UI_TradePricesDialog_drawBackground, noop, UI_TradePricesDialog_handleMouse, UI_TradePricesDialog_getTooltip },
	{ noop, UI_ResourceSettingsDialog_drawBackground, UI_ResourceSettingsDialog_drawForeground, UI_ResourceSettingsDialog_handleMouse },
	{ noop, UI_MessageDialog_drawBackground, UI_MessageDialog_drawForeground, UI_MessageDialog_handleMouse },
	{ UI_PlayerMessageList_init, UI_PlayerMessageList_drawBackground, UI_PlayerMessageList_drawForeground, UI_PlayerMessageList_handleMouse, UI_PlayerMessageList_getTooltip },
	// 20
	{ UI_CCKSelection_init, UI_CCKSelection_drawBackground, UI_CCKSelection_drawForeground, UI_CCKSelection_handleMouse},
	{ noop, UI_FileDialog_drawBackground, UI_FileDialog_drawForeground, UI_FileDialog_handleMouse },
	{ UI_OverlayMenu_init, UI_OverlayMenu_drawBackground, UI_OverlayMenu_drawForeground, UI_OverlayMenu_handleMouse },
	{ noop, UI_BuildingMenu_drawBackground, UI_BuildingMenu_drawForeground, UI_BuildingMenu_handleMouse },
	{ noop, UI_Intermezzo_drawBackground, noop, UI_Intermezzo_handleMouse },
	{ UI_BuildingInfo_init, UI_BuildingInfo_drawBackground, UI_BuildingInfo_drawForeground, UI_BuildingInfo_handleMouse, UI_BuildingInfo_getTooltip },
	{ UI_NewCareerDialog_init, UI_NewCareerDialog_drawBackground, UI_NewCareerDialog_drawForeground, UI_NewCareerDialog_handleMouse },
	{ noop, UI_SlidingSidebar_drawBackground, UI_SlidingSidebar_drawForeground, noop },
	{ noop, UI_City_drawBackground, UI_City_drawForegroundMilitary, UI_City_handleMouseMilitary, UI_CityBuildings_getTooltip }, // TODO military command
	{ noop, UI_MissionStart_Selection_drawBackground, UI_MissionStart_Selection_drawForeground, UI_MissionStart_Selection_handleMouse },
	// 30
	{ UI_MissionStart_Briefing_init, UI_MissionStart_Briefing_drawBackground, UI_MissionStart_BriefingInitial_drawForeground, UI_MissionStart_BriefingInitial_handleMouse },
	{ UI_MissionStart_Briefing_init, UI_MissionStart_Briefing_drawBackground, UI_MissionStart_BriefingReview_drawForeground, UI_MissionStart_BriefingReview_handleMouse },
	{ noop, UI_VictoryDialog_drawBackground, UI_VictoryDialog_drawForeground, UI_VictoryDialog_handleMouse },
	{ noop, UI_MissionEnd_drawBackground, UI_MissionEnd_drawForeground, UI_MissionEnd_handleMouse },
	{ UI_VictoryIntermezzo_init, noop, noop, noop },
};

static WindowId previousWindow;
static WindowId currentWindow;
static int refreshRequested;

WindowId UI_Window_getId()
{
	return currentWindow;
}

void UI_Window_goTo(WindowId windowId)
{
	previousWindow = currentWindow;
	currentWindow = windowId;
	windows[currentWindow].init();
	UI_Window_requestRefresh();
}

void UI_Window_goBack()
{
	UI_Window_goTo(previousWindow);
}

static void updateMouseBefore()
{
	int prevLeftIsDown = Data_Mouse.left.isDown;
	int prevRightIsDown = Data_Mouse.right.isDown;
	Data_Mouse.left.isDown = 0;
	Data_Mouse.left.wentDown = 0;
	Data_Mouse.left.wentUp = 0;
	Data_Mouse.right.isDown = 0;
	Data_Mouse.right.wentDown = 0;
	Data_Mouse.right.wentUp = 0;

	Data_Mouse.left.isDown = Data_Mouse.leftDown;
	Data_Mouse.right.isDown = Data_Mouse.rightDown;

	if (Data_Mouse.left.isDown != prevLeftIsDown) {
		if (Data_Mouse.left.isDown) {
			Data_Mouse.left.wentDown = 1;
		} else {
			Data_Mouse.left.wentUp = 1;
		}
	}
	if (Data_Mouse.right.isDown != prevRightIsDown) {
		if (Data_Mouse.right.isDown) {
			Data_Mouse.right.wentDown = 1;
		} else {
			Data_Mouse.right.wentUp = 1;
		}
	}
}

static void updateMouseAfter()
{
	Data_Mouse.scrollDown = 0;
	Data_Mouse.scrollUp = 0;
	Cursor_set();
}

void UI_Window_refresh(int force)
{
	updateMouseBefore();
	if (force || refreshRequested) {
		// TODO tooltip: if (tooltip_enabled) window = tooltip_windowid; else window = currentWindow
		windows[currentWindow].drawBackground();
		refreshRequested = 0;
	}
	windows[currentWindow].drawForeground();
	KeyboardInput_initInput(0);
	windows[currentWindow].handleMouse();
	UI_Tooltip_handle(windows[currentWindow].getTooltip);
	UI_Warning_draw();
	updateMouseAfter();
}

void UI_Window_requestRefresh()
{
	refreshRequested = 1;
}
