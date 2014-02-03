#ifndef UI_WINDOW_H
#define UI_WINDOW_H

// TODO numbers
typedef enum {
	Window_MainMenu = 0,
	Window_City = 1, // 1?
	Window_ConfirmDialog = 2, //63
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
} WindowId;

WindowId UI_Window_getId();
void UI_Window_goTo(WindowId windowId);
void UI_Window_goBack();

void UI_Window_refresh(int force);
void UI_Window_requestRefresh();

#endif
