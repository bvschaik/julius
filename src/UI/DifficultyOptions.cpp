
#include "Window.h"
#include "../Widget.h"
#include "../Graphics.h"
#include "../Data/Constants.h"
#include "../Data/Graphics.h"
#include "../Data/Mouse.h"
#include "../Data/Screen.h"
#include "../Data/Settings.h"

static void arrowButtonDifficulty(int param1, int param2);
static void arrowButtonGods(int param1, int param2);

static ArrowButton arrowButtons[3] = {
	{0, 54, 15, 24, arrowButtonDifficulty, 0, 0},
	{24, 54, 17, 24, arrowButtonDifficulty, 1, 0},
	{0, 102, 21, 24, arrowButtonGods, 2, 0}
};

void UI_DifficultyOptions_drawForeground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;
	
	Widget_Panel_drawOuterPanel(
		baseOffsetX + 48, baseOffsetY + 80,
		24, 12
	);
	
	Widget_GameText_drawCentered(153, 0,
		baseOffsetX + 48, baseOffsetY + 94,
		384, Font_LargeBlack
	);
	
	Widget_GameText_drawCentered(153, Data_Settings.difficulty + 1,
		baseOffsetX + 80, baseOffsetY + 142,
		224, Font_NormalBlack
	);
	Widget_GameText_drawCentered(
		153, Data_Settings.godsEnabled ? 7 : 6,
		baseOffsetX + 80, baseOffsetY + 190,
		224, Font_NormalBlack
	);
	Widget_Button_drawArrowButtons(
		baseOffsetX + 288, baseOffsetY + 80,
		arrowButtons, 3
	);
	Widget_GameText_drawCentered(153, 8,
		baseOffsetX + 48, baseOffsetY + 246,
		384, Font_NormalBlack
	);
}

void UI_DifficultyOptions_handleMouse()
{
	if (Data_Mouse.right.wentUp) {
		// cancel dialog
		UI_Window_goTo(Window_City);
	} else {
		int baseOffsetX = Data_Screen.offset640x480.x;
		int baseOffsetY = Data_Screen.offset640x480.y;
		Widget_Button_handleArrowButtons(
			baseOffsetX + 288, baseOffsetY + 80, arrowButtons, 4);
	}
}

static void arrowButtonDifficulty(int param1, int param2)
{
	if (param1) {
		Data_Settings.difficulty--;
	} else {
		Data_Settings.difficulty++;
	}
	if (Data_Settings.difficulty < Difficulty_VeryEasy) {
		Data_Settings.difficulty = Difficulty_VeryEasy;
	}
	if (Data_Settings.difficulty > Difficulty_VeryHard) {
		Data_Settings.difficulty = Difficulty_VeryHard;
	}
	UI_Window_requestRefresh();
}

static void arrowButtonGods(int param1, int param2)
{
	if (Data_Settings.godsEnabled) {
		Data_Settings.godsEnabled = 0;
	} else {
		Data_Settings.godsEnabled = 1;
	}
	UI_Window_requestRefresh();
}

