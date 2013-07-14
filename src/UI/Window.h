#ifndef UI_WINDOW_H
#define UI_WINDOW_H

// TODO numbers
typedef enum {
	Window_MainMenu = 0,
	Window_City = 0,
	Window_DisplayOptions = 1,//10,
	Window_SoundOptions = 2,//11,
	Window_SpeedOptions = 3,//12,
	Window_DifficultyOptions = 4,//69
	Window_Advisors = 5, // 7
	Window_SetSalaryDialog = 6, //44
	Window_DonateToCityDialog = 7, //45
	Window_SendGiftToCaesarDialog = 8, //46
} WindowId;

void UI_Window_goTo(WindowId windowId);
void UI_Window_refresh(int force);
void UI_Window_requestRefresh();

#endif
