#include "Window.h"

#include "../Graphics.h"
#include "../System.h"
#include "../Widget.h"

#include "../Data/Constants.h"
#include "../Data/Screen.h"
#include "../Data/Settings.h"

#include "graphics/mouse.h"

static void buttonFullscreen(int param1, int param2);
static void buttonSetResolution(int param1, int param2);
static void buttonCancel(int param1, int param2);

static CustomButton buttons[] = {
	{144, 136, 336, 156, CustomButton_Immediate, buttonFullscreen, Widget_Button_doNothing, 1, 0},
	{144, 160, 336, 180, CustomButton_Immediate, buttonSetResolution, Widget_Button_doNothing, 1, 0},
	{144, 184, 336, 204, CustomButton_Immediate, buttonSetResolution, Widget_Button_doNothing, 2, 0},
	{144, 208, 336, 228, CustomButton_Immediate, buttonSetResolution, Widget_Button_doNothing, 3, 0},
	{144, 232, 336, 252, CustomButton_Immediate, buttonCancel, Widget_Button_doNothing, 1, 0},
};

static int focusButtonId;

void UI_DisplayOptions_drawForeground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;
	Widget_Panel_drawOuterPanel(
		baseOffsetX + 96, baseOffsetY + 80,
		18, 12
	);
	
	Widget_Panel_drawSmallLabelButton(
		baseOffsetX + 128, baseOffsetY + 136,
		14, focusButtonId == 1 ? 1 : 2
	);
	Widget_Panel_drawSmallLabelButton(
		baseOffsetX + 128, baseOffsetY + 160,
		14, focusButtonId == 2 ? 1 : 2
	);
	Widget_Panel_drawSmallLabelButton(
		baseOffsetX + 128, baseOffsetY + 184,
		14, focusButtonId == 3 ? 1 : 2
	);
	Widget_Panel_drawSmallLabelButton(
		baseOffsetX + 128, baseOffsetY + 208,
		14, focusButtonId == 4 ? 1 : 2
	);
	Widget_Panel_drawSmallLabelButton(
		baseOffsetX + 128, baseOffsetY + 232,
		14, focusButtonId == 5 ? 1 : 2
	);
	
	Widget_GameText_drawCentered(42, 0,
		baseOffsetX + 128, baseOffsetY + 94,
		224, FONT_LARGE_BLACK
	);
	
	Widget_GameText_drawCentered(42,
		Data_Settings.fullscreen ? 2 : 1,
		baseOffsetX + 128, baseOffsetY + 140,
		224, FONT_NORMAL_GREEN
	);
	
	Widget_GameText_drawCentered(42, 3,
		baseOffsetX + 128, baseOffsetY + 164,
		224, FONT_NORMAL_GREEN
	);
	Widget_GameText_drawCentered(42, 4,
		baseOffsetX + 128, baseOffsetY + 188,
		224, FONT_NORMAL_GREEN
	);
	Widget_GameText_drawCentered(42, 5,
		baseOffsetX + 128, baseOffsetY + 212,
		224, FONT_NORMAL_GREEN
	);
	Widget_GameText_drawCentered(42, 6,
		baseOffsetX + 128, baseOffsetY + 236,
		224, FONT_NORMAL_GREEN
	);
}

void UI_DisplayOptions_handleMouse(const mouse *m)
{
	if (m->right.went_up) {
		// cancel dialog
		UI_Window_goTo(Window_City);
	} else {
		int baseOffsetX = Data_Screen.offset640x480.x;
		int baseOffsetY = Data_Screen.offset640x480.y;
		Widget_Button_handleCustomButtons(
			baseOffsetX, baseOffsetY, buttons, 5, &focusButtonId);
	}
}

static void buttonFullscreen(int param1, int param2)
{
	System_toggleFullscreen();
	UI_Window_goTo(Window_City);
}

static void buttonSetResolution(int id, int param2)
{
	switch (id) {
		case 1: System_resize(640, 480); break;
		case 2: System_resize(800, 600); break;
		case 3: System_resize(1024, 768); break;
	}
	UI_Window_goTo(Window_City);
}

static void buttonCancel(int param1, int param2)
{
	UI_Window_goTo(Window_City);
}
