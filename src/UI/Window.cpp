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
	{ UI_SoundOptions_init, noop, UI_SpeedOptions_drawForeground, UI_SpeedOptions_handleMouse },
	{ noop, noop, UI_DifficultyOptions_drawForeground, UI_DifficultyOptions_handleMouse },
	{ noop, UI_Advisors_drawBackground, UI_Advisors_drawForeground, UI_Advisors_handleMouse },
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
