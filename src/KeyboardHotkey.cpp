#include "KeyboardHotkey.h"

#include "UI/AllWindows.h"
#include "UI/Warning.h"
#include "UI/Window.h"

#include "Data/Settings.h"
#include "Data/State.h"

#define ExitMilitaryCommand() \
	if (UI_Window_getId() == Window_CityMilitary) {\
		UI_Window_goTo(Window_City);\
	}

static void changeGameSpeed(int isDown)
{
	if (UI_Window_getId() == Window_City) {
		UI_SpeedOptions_changeGameSpeed(isDown);
	}
}

static void toggleOverlay()
{
	ExitMilitaryCommand();
	if (UI_Window_getId() == Window_City) {
		int tmp = Data_State.previousOverlay;
		Data_State.previousOverlay = Data_State.currentOverlay;
		Data_State.currentOverlay = tmp;
		UI_Window_requestRefresh();
	}
}

static void showOverlay(int overlay)
{
	ExitMilitaryCommand();
	WindowId window = UI_Window_getId();
	// TODO tooltip windowid??
	if (window == Window_City) {
		if (Data_State.currentOverlay == overlay) {
			Data_State.previousOverlay = overlay;
			Data_State.currentOverlay = 0;
		} else {
			Data_State.previousOverlay = 0;
			Data_State.currentOverlay = overlay;
		}
		UI_Window_requestRefresh();
	}
}

static void togglePause()
{
	ExitMilitaryCommand();
	if (UI_Window_getId() == Window_City) {
		if (Data_Settings.gamePaused) {
			Data_Settings.gamePaused = 0;
		} else {
			Data_Settings.gamePaused = 1;
		}
		UI_Warning_clearAll();
	}
}

void KeyboardHotkey_character(int c)
{
	switch (c) {
		case '[':
			changeGameSpeed(1);
			break;
		case ']':
			changeGameSpeed(0);
			break;
		case ' ':
			toggleOverlay();
			break;
		case 'P': case 'p':
			togglePause();
			break;
		case 'F': case 'f':
			showOverlay(Overlay_Fire);
			break;
		case 'D': case 'd':
			showOverlay(Overlay_Damage);
			break;
		case 'C': case 'c':
			showOverlay(Overlay_Crime);
			break;
		case 'T': case 't':
			showOverlay(Overlay_Problems);
			break;
		case 'W': case 'w':
			showOverlay(Overlay_Water);
			break;
		// TODO more
	}
}

