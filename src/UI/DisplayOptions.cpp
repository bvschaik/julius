#include "Window.h"

#include "../Graphics.h"
#include "../System.h"
#include "../Widget.h"

#include "../Data/Constants.h"
#include "../Data/Graphics.h"
#include "../Data/Mouse.h"
#include "../Data/Screen.h"
#include "../Data/Settings.h"

static void buttonFullscreen(int param1, int param2);
static void buttonSetResolution(int param1, int param2);
static void buttonCancel(int param1, int param2);

static CustomButton buttons[5] = {
	{144, 136, 336, 156, buttonFullscreen, Widget_Button_doNothing, 1, 1, 0},
	{144, 160, 336, 180, buttonSetResolution, Widget_Button_doNothing, 1, 1, 0},
	{144, 184, 336, 204, buttonSetResolution, Widget_Button_doNothing, 1, 2, 0},
	{144, 208, 336, 228, buttonSetResolution, Widget_Button_doNothing, 1, 3, 0},
	{144, 232, 336, 252, buttonCancel, Widget_Button_doNothing, 1, 1, 0},
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
	
	Widget_Panel_drawSmallLabelButton(2,
		baseOffsetX + 128, baseOffsetY + 136,
		14, 0, focusButtonId == 1 ? 1 : 2
	);
	Widget_Panel_drawSmallLabelButton(2,
		baseOffsetX + 128, baseOffsetY + 160,
		14, 0, focusButtonId == 2 ? 1 : 2
	);
	Widget_Panel_drawSmallLabelButton(2,
		baseOffsetX + 128, baseOffsetY + 184,
		14, 0, focusButtonId == 3 ? 1 : 2
	);
	Widget_Panel_drawSmallLabelButton(2,
		baseOffsetX + 128, baseOffsetY + 208,
		14, 0, focusButtonId == 4 ? 1 : 2
	);
	Widget_Panel_drawSmallLabelButton(2,
		baseOffsetX + 128, baseOffsetY + 232,
		14, 0, focusButtonId == 5 ? 1 : 2
	);
	
	Widget_GameText_drawCentered(42, 0,
		baseOffsetX + 128, baseOffsetY + 94,
		224, Font_LargeBlack
	);
	
	Widget_GameText_drawCentered(42,
		Data_Settings.fullscreen ? 2 : 1,
		baseOffsetX + 128, baseOffsetY + 140,
		224, Font_NormalGreen
	);
	
	Widget_GameText_drawCentered(42, 3,
		baseOffsetX + 128, baseOffsetY + 164,
		224, Font_NormalGreen
	);
	Widget_GameText_drawCentered(42, 4,
		baseOffsetX + 128, baseOffsetY + 188,
		224, Font_NormalGreen
	);
	Widget_GameText_drawCentered(42, 5,
		baseOffsetX + 128, baseOffsetY + 212,
		224, Font_NormalGreen
	);
	Widget_GameText_drawCentered(42, 6,
		baseOffsetX + 128, baseOffsetY + 236,
		224, Font_NormalGreen
	);
}

void UI_DisplayOptions_handleMouse()
{
	if (Data_Mouse.right.wentUp) {
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
