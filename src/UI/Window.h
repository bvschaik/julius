#ifndef UI_WINDOW_H
#define UI_WINDOW_H

// TODO numbers
typedef enum {
	Window_MainMenu = 0,
	Window_City = 1, // 1
	Window_PopupDialog = 2, //63
	Window_TopMenu = 3, // 3
	Window_DifficultyOptions = 4,//69
	Window_Advisors = 5, // 7
	Window_SetSalaryDialog = 6, //44
	Window_DonateToCityDialog = 7, //45
	Window_SendGiftToCaesarDialog = 8, //46
	Window_LaborPriorityDialog = 9, // 42
	Window_DisplayOptions = 10,//10,
	Window_SoundOptions = 11,//11,
	Window_SpeedOptions = 12,//12,
	Window_Empire = 13,//20
	Window_TradeOpenedDialog = 14,//38
	Window_HoldFestivalDialog = 15,//43
	Window_TradePricesDialog = 16,//41
	Window_ResourceSettingsDialog = 17,//40
	Window_MessageDialog = 18, // 5
	Window_PlayerMessageList = 19, //17
	Window_CCKSelection = 20, //61
	Window_FileDialog = 21, //13
	Window_OverlayMenu = 22, //8
	Window_BuildingMenu = 23, //6
	Window_Intermezzo = 24, //4
	Window_BuildingInfo = 25, //9
	Window_NewCareerDialog = 26, //54
	Window_SlidingSidebar = 27, //2
	Window_CityMilitary = 28, // 30 TODO
	Window_MissionSelection = 29, // 52
	Window_MissionBriefingInitial = 30, // 53
	Window_MissionBriefingReview = 31, // 59
	Window_VictoryDialog = 32, // 67
	Window_MissionEnd = 33, // 51
	Window_VictoryIntermezzo = 34,
	Window_VideoIntermezzo = 35, // 19
} WindowId;

WindowId UI_Window_getId();
void UI_Window_goTo(WindowId windowId);
void UI_Window_goBack();

void UI_Window_refresh(int force);
void UI_Window_requestRefresh();

#endif
