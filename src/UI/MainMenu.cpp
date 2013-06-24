#include "AllWindows.h"
#include "Window.h"

#include "../Widget.h"
#include "../Graphics.h"
#include "../Data/Constants.h"
#include "../Data/Graphics.h"
#include "../Data/Mouse.h"
#include "../Data/Screen.h"

#include <stdio.h> // TODO remove

static void buttonClick(int param1, int param2);

static CustomButton buttons[4] = {
	{
		192, 100, 448, 125,
		buttonClick,
		Widget_Button_doNothing,
		1, 1, 0
	},
	{
		192, 140, 448, 165,
		buttonClick,
		Widget_Button_doNothing,
		1, 2, 0
	},
	{
		192, 180, 448, 205,
		buttonClick,
		Widget_Button_doNothing,
		1, 3, 0
	},
	{
		192, 220, 448, 245,
		buttonClick,
		Widget_Button_doNothing,
		1, 4, 0
	},
};

void UI_MainMenu_drawBackground()
{
	Graphics_drawImage(
		GraphicId(ID_Graphic_MainMenuBackground),
		Data_Screen.offset640x480.x,
		Data_Screen.offset640x480.y);
}

void UI_MainMenu_drawForeground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;
	int focusButton = Data_Mouse.focusButtonId;

	Widget_Panel_drawLargeLabelButton(6,
		baseOffsetX + 192, baseOffsetY + 100,
		16, focusButton == 1 ? 1 : 0
	);
	Widget_Panel_drawLargeLabelButton(6,
		baseOffsetX + 192, baseOffsetY + 140,
		16, focusButton == 2 ? 1 : 0
	);
	Widget_Panel_drawLargeLabelButton(6,
		baseOffsetX + 192, baseOffsetY + 180,
		16, focusButton == 3 ? 1 : 0
	);
	Widget_Panel_drawLargeLabelButton(6,
		baseOffsetX + 192, baseOffsetY + 220,
		16, focusButton == 4 ? 1 : 0
	);

	Widget_GameText_drawCentered(30, 1,
		baseOffsetX + 192, baseOffsetY + 106,
		256, Font_SmallBrown, 0
	);
	Widget_GameText_drawCentered(30, 2,
		baseOffsetX + 192, baseOffsetY + 146,
		256, Font_SmallBrown, 0
	);
	Widget_GameText_drawCentered(30, 3,
		baseOffsetX + 192, baseOffsetY + 186,
		256, Font_SmallBrown, 0
	);
	Widget_GameText_drawCentered(30, 5,
		baseOffsetX + 192, baseOffsetY + 226,
		256, Font_SmallBrown, 0
	);
}

void UI_MainMenu_handleMouse()
{
	Widget_Button_handleCustomButtons(
		Data_Screen.offset640x480.x, Data_Screen.offset640x480.y,
		buttons, 4);
}

static void buttonClick(int param1, int param2)
{
	// TODO old stuff
	if (Data_Mouse.focusButtonId == 1) {
		UI_Window_goTo(Window_SoundOptions);
	}
	if (Data_Mouse.focusButtonId == 2) {
		UI_Window_goTo(Window_SpeedOptions);
	}
	if (Data_Mouse.focusButtonId == 3) {
		UI_Window_goTo(Window_DifficultyOptions);
	}
	if (Data_Mouse.focusButtonId == 4) {
		UI_Window_goTo(Window_Advisors);
	}
	printf("Clicked: %d\n", Data_Mouse.focusButtonId);
}
