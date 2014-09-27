#include "KeyboardHotkey.h"

#include "CityView.h"

#include "UI/Advisors.h"
#include "UI/AllWindows.h"
#include "UI/Warning.h"
#include "UI/Window.h"

#include "Data/Constants.h"
#include "Data/Formation.h"
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

static void showAdvisor(int advisor)
{
	ExitMilitaryCommand();
	if (UI_Window_getId() == Window_Advisors) {
		if (UI_Advisors_getId() == advisor) {
			UI_Window_goTo(Window_City);
		} else {
			UI_Advisors_goToFromMessage(advisor);
		}
	} else if (UI_Window_getId() == Window_City) {
		UI_Advisors_goToFromMessage(advisor);
	}
}

static void cycleLegion()
{
	static int currentLegionId = 1;
	if (UI_Window_getId() == Window_City) {
		int legionId = currentLegionId;
		currentLegionId = 0;
		for (int i = 1; i <= 6; i++) {
			legionId++;
			if (legionId > 6) {
				legionId = 1;
			}
			struct Data_Formation *f = &Data_Formations[legionId];
			if (f->inUse == 1 && !f->isHerd && f->isLegion) {
				if (currentLegionId == 0) {
					currentLegionId = legionId;
					break;
				}
			}
		}
		if (currentLegionId > 0) {
			struct Data_Formation *f = &Data_Formations[currentLegionId];
			CityView_goToGridOffset(GridOffset(f->xHome, f->yHome));
			UI_Window_requestRefresh();
		}
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
		case 'L': case 'l':
			cycleLegion();
			break;
		case '1':
			showAdvisor(Advisor_Chief);
			break;
		case '2':
			showAdvisor(Advisor_Military);
			break;
		case '3':
			showAdvisor(Advisor_Imperial);
			break;
		case '4':
			showAdvisor(Advisor_Ratings);
			break;
		case '5':
			showAdvisor(Advisor_Trade);
			break;
		case '6':
			showAdvisor(Advisor_Population);
			break;
		case '7':
			showAdvisor(Advisor_Health);
			break;
		case '8':
			showAdvisor(Advisor_Education);
			break;
		case '9':
			showAdvisor(Advisor_Entertainment);
			break;
		case '0':
			showAdvisor(Advisor_Religion);
			break;
		case '-':
			showAdvisor(Advisor_Financial);
			break;
		case '=':
			showAdvisor(Advisor_Chief);
			break;
	}
}

