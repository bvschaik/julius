#include "Window.h"
#include "AllWindows.h"

#include "../Data/Mouse.h"

struct Window {
	void (*init)(void);
	void (*drawBackground)(void);
	void (*drawForeground)(void);
	void (*handleMouse)(void);
};

static void noop()
{
}

static void rightClickToExit()
{
	if (Data_Mouse.isRightClick) {
		// cancel dialog
		UI_Window_goTo(Window_City);
	}
}

static struct Window windows[] = {
	{ noop, UI_MainMenu_drawBackground, UI_MainMenu_drawForeground, UI_MainMenu_handleMouse },
	{ noop, noop, noop, noop },
	{ UI_SoundOptions_init, noop, UI_SoundOptions_drawForeground, UI_SoundOptions_handleMouse },
	{ UI_SpeedOptions_init, noop, UI_SpeedOptions_drawForeground, UI_SpeedOptions_handleMouse },
	{ noop, noop, UI_DifficultyOptions_drawForeground, UI_DifficultyOptions_handleMouse },
	{ noop, UI_Advisors_drawBackground, UI_Advisors_drawForeground, UI_Advisors_handleMouse },
	{ noop, UI_SetSalaryDialog_drawBackground, UI_SetSalaryDialog_drawForeground, UI_SetSalaryDialog_handleMouse },
	{ UI_DonateToCityDialog_init, UI_DonateToCityDialog_drawBackground, UI_DonateToCityDialog_drawForeground, UI_DonateToCityDialog_handleMouse },
	{ UI_SendGiftToCaesarDialog_init, UI_SendGiftToCaesarDialog_drawBackground, UI_SendGiftToCaesarDialog_drawForeground, UI_SendGiftToCaesarDialog_handleMouse },
};

static WindowId currentWindow;

void UI_Window_goTo(WindowId windowId)
{
	currentWindow = windowId;
	windows[currentWindow].init();
	UI_Window_requestRefresh();
}

void UI_Window_refresh(int force)
{
	if (force) {
		windows[currentWindow].drawBackground();
	}
	windows[currentWindow].drawForeground();
	windows[currentWindow].handleMouse();
}

void UI_Window_requestRefresh()
{
	// TODO
}
