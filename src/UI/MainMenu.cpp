#include "AllWindows.h"
#include "FileDialog.h"
#include "PopupDialog.h"
#include "Window.h"

#include "../Widget.h"
#include "../Graphics.h"
#include "../Data/Constants.h"
#include "../Data/Graphics.h"
#include "../Data/Mouse.h"
#include "../Data/Screen.h"

#include <stdio.h> // TODO remove

static void buttonClick(int param1, int param2);

static int focusButtonId;

static CustomButton buttons[4] = {
	{192, 100, 448, 125, buttonClick, Widget_Button_doNothing, 1, 1, 0},
	{192, 140, 448, 165, buttonClick, Widget_Button_doNothing, 1, 2, 0},
	{192, 180, 448, 205, buttonClick, Widget_Button_doNothing, 1, 3, 0},
	{192, 220, 448, 245, buttonClick, Widget_Button_doNothing, 1, 4, 0},
};

void UI_MainMenu_drawBackground()
{
	Graphics_clearScreen();
	Graphics_drawImage(
		GraphicId(ID_Graphic_MainMenuBackground),
		Data_Screen.offset640x480.x,
		Data_Screen.offset640x480.y);
}

void UI_MainMenu_drawForeground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	Widget_Panel_drawLargeLabelButton(6,
		baseOffsetX + 192, baseOffsetY + 100,
		16, focusButtonId == 1 ? 1 : 0
	);
	Widget_Panel_drawLargeLabelButton(6,
		baseOffsetX + 192, baseOffsetY + 140,
		16, focusButtonId == 2 ? 1 : 0
	);
	Widget_Panel_drawLargeLabelButton(6,
		baseOffsetX + 192, baseOffsetY + 180,
		16, focusButtonId == 3 ? 1 : 0
	);
	Widget_Panel_drawLargeLabelButton(6,
		baseOffsetX + 192, baseOffsetY + 220,
		16, focusButtonId == 4 ? 1 : 0
	);

	Widget_GameText_drawCentered(30, 1,
		baseOffsetX + 192, baseOffsetY + 106,
		256, Font_NormalGreen
	);
	Widget_GameText_drawCentered(30, 2,
		baseOffsetX + 192, baseOffsetY + 146,
		256, Font_NormalGreen
	);
	Widget_GameText_drawCentered(30, 3,
		baseOffsetX + 192, baseOffsetY + 186,
		256, Font_NormalGreen
	);
	Widget_GameText_drawCentered(30, 5,
		baseOffsetX + 192, baseOffsetY + 226,
		256, Font_NormalGreen
	);
}

void UI_MainMenu_handleMouse()
{
	Widget_Button_handleCustomButtons(
		Data_Screen.offset640x480.x, Data_Screen.offset640x480.y,
		buttons, 4, &focusButtonId);
}

static void buttonClick(int param1, int param2)
{
	if (param1 == 1) {
		UI_Window_goTo(Window_NewCareerDialog);
	} else if (param1 == 2) {
		UI_FileDialog_show(FileDialogType_Load);
	} else if (param1 == 3) {
		UI_Window_goTo(Window_CCKSelection);
	} else if (param1 == 4) {
		UI_Window_goTo(Window_VictoryDialog); // TODO exit
	}
}
