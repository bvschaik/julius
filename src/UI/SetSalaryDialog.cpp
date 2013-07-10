#include "AllWindows.h"
#include "Window.h"
#include "Advisors_private.h"
#include "../Data/Mouse.h"
#include "../Data/Constants.h"

static void buttonCancel(int param1, int param2);
static void buttonSetSalary(int param1, int param2);

static CustomButton buttons[12] = {
	{240, 395, 400, 415, buttonCancel, Widget_Button_doNothing, 1, 0, 0},
	{144, 85, 432, 105, buttonSetSalary, Widget_Button_doNothing, 1, 0, 0},
	{144, 105, 432, 125, buttonSetSalary, Widget_Button_doNothing, 1, 1, 0},
	{144, 125, 432, 145, buttonSetSalary, Widget_Button_doNothing, 1, 2, 0},
	{144, 145, 432, 165, buttonSetSalary, Widget_Button_doNothing, 1, 3, 0},
	{144, 165, 432, 185, buttonSetSalary, Widget_Button_doNothing, 1, 4, 0},
	{144, 185, 432, 205, buttonSetSalary, Widget_Button_doNothing, 1, 5, 0},
	{144, 205, 432, 225, buttonSetSalary, Widget_Button_doNothing, 1, 6, 0},
	{144, 225, 432, 245, buttonSetSalary, Widget_Button_doNothing, 1, 7, 0},
	{144, 245, 432, 265, buttonSetSalary, Widget_Button_doNothing, 1, 8, 0},
	{144, 265, 432, 285, buttonSetSalary, Widget_Button_doNothing, 1, 9, 0},
	{144, 285, 432, 305, buttonSetSalary, Widget_Button_doNothing, 1, 10, 0},
};

static int focusButtonId;

void UI_SetSalaryDialog_drawBackground()
{
	UI_Advisor_drawGeneralBackground();
	UI_SetSalaryDialog_drawForeground();
}

void UI_SetSalaryDialog_drawForeground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;
	
	Widget_Panel_drawOuterPanel(baseOffsetX + 128, baseOffsetY + 32, 24, 25);
	Graphics_drawImage(GraphicId(ID_Graphic_ResourceIcons) + 16,
		baseOffsetX + 144, baseOffsetY + 48);
	Widget_GameText_drawCentered(52, 15,
		baseOffsetX + 144, baseOffsetY + 48, 368, Font_LargeBlack, 0);

	Widget_Panel_drawInnerPanel(baseOffsetX + 144, baseOffsetY + 80, 22, 15);

	for (int rank = 0; rank < 11; rank++) {
		Font font = (focusButtonId == rank + 2) ? Font_NormalRed : Font_NormalWhite;
		int width = Widget_GameText_draw(52, rank + 4,
			baseOffsetX + 176, baseOffsetY + 90 + 20 * rank, font, 0);
		Widget_Text_drawNumber(Constant_SalaryForRank[rank], '@', " Dn",
			baseOffsetX + 176 + width, baseOffsetY + 90 + 20 * rank, font, 0);
	}

	if (Data_CityInfo.victoryContinueMonths <= 0) {
		if (Data_CityInfo.salaryRank <= Data_CityInfo.playerRank) {
			Widget_GameText_drawMultiline(52, 76,
				baseOffsetX + 152, baseOffsetY + 336, 336, Font_NormalBlack, 0);
		} else {
			Widget_GameText_drawMultiline(52, 71,
				baseOffsetX + 152, baseOffsetY + 336, 336, Font_NormalBlack, 0);
		}
	} else {
		Widget_GameText_drawMultiline(52, 77,
				baseOffsetX + 152, baseOffsetY + 336, 336, Font_NormalBlack, 0);
	}
	Widget_Panel_drawButtonBorder(baseOffsetX + 240, baseOffsetY + 395,
		160, 20, focusButtonId == 1);
	Widget_GameText_drawCentered(13, 4, baseOffsetX + 176, baseOffsetY + 400, 288, Font_NormalBlack, 0);
}

void UI_SetSalaryDialog_handleMouse()
{
	if (Data_Mouse.isRightClick) {
		UI_Window_goTo(Window_Advisors);
	} else {
		int offsetX = Data_Screen.offset640x480.x;
		int offsetY = Data_Screen.offset640x480.y;
		Widget_Button_handleCustomButtons(offsetX, offsetY,
			buttons, 12, &focusButtonId);
	}
}

static void buttonCancel(int param1, int param2)
{
	UI_Window_goTo(Window_Advisors);
}

static void buttonSetSalary(int param1, int param2)
{
	if (Data_CityInfo.victoryContinueMonths <= 0) {
		Data_CityInfo.salaryRank = param1;
		Data_CityInfo.salaryAmount = Constant_SalaryForRank[param1];
		// TODO update finance salary + favor rating
		UI_Window_goTo(Window_Advisors);
	}
}

