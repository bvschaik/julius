#include "AllWindows.h"
#include "Window.h"
#include "../Widget.h"
#include "../Graphics.h"
#include "../CityInfo.h"
#include "Advisors_private.h"
#include "../Data/CityInfo.h"
#include "../Data/Screen.h"

#include "core/calc.h"
#include "game/resource.h"

static void buttonSetAmount(int param1, int param2);
static void buttonDonate(int param1, int param2);
static void buttonCancel(int param1, int param2);
static void arrowButtonAmount(int isDown, int param2);

static CustomButton buttons[] = {
	{336, 283, 496, 303, CustomButton_Immediate, buttonCancel, Widget_Button_doNothing, 0, 0},
	{144, 283, 304, 303, CustomButton_Immediate, buttonDonate, Widget_Button_doNothing, 0, 0},
	{128, 216, 192, 236, CustomButton_Immediate, buttonSetAmount, Widget_Button_doNothing, 0, 0},
	{208, 216, 272, 236, CustomButton_Immediate, buttonSetAmount, Widget_Button_doNothing, 1, 0},
	{288, 216, 352, 236, CustomButton_Immediate, buttonSetAmount, Widget_Button_doNothing, 2, 0},
	{368, 216, 432, 236, CustomButton_Immediate, buttonSetAmount, Widget_Button_doNothing, 3, 0},
	{448, 216, 512, 236, CustomButton_Immediate, buttonSetAmount, Widget_Button_doNothing, 4, 0},
};

static ArrowButton arrowButtons[] = {
	{240, 242, 17, 24, arrowButtonAmount, 1, 0},
	{264, 242, 15, 24, arrowButtonAmount, 0, 0},
};

static int focusButtonId;
static int arrowButtonFocus;

void UI_DonateToCityDialog_init()
{
	if (Data_CityInfo.donateAmount > Data_CityInfo.personalSavings) {
		Data_CityInfo.donateAmount = Data_CityInfo.personalSavings;
	}
}

void UI_DonateToCityDialog_drawBackground()
{
	UI_Advisor_drawGeneralBackground();

	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	Widget_Panel_drawOuterPanel(baseOffsetX + 64, baseOffsetY + 160, 32, 10);
	Graphics_drawImage(image_group(ID_Graphic_ResourceIcons) + RESOURCE_DENARII,
		baseOffsetX + 96, baseOffsetY + 176);
	Widget_GameText_drawCentered(52, 16,
		baseOffsetX + 80, baseOffsetY + 176, 496, FONT_LARGE_BLACK);

	Widget_Panel_drawInnerPanel(baseOffsetX + 112, baseOffsetY + 208, 26, 4);

	Widget_Text_drawNumberCentered(0, baseOffsetX + 124, baseOffsetY + 221, 64, FONT_NORMAL_WHITE);
	Widget_Text_drawNumberCentered(500, baseOffsetX + 204, baseOffsetY + 221, 64, FONT_NORMAL_WHITE);
	Widget_Text_drawNumberCentered(2000, baseOffsetX + 284, baseOffsetY + 221, 64, FONT_NORMAL_WHITE);
	Widget_Text_drawNumberCentered(5000, baseOffsetX + 364, baseOffsetY + 221, 64, FONT_NORMAL_WHITE);
	Widget_GameText_drawCentered(52, 19, baseOffsetX + 444, baseOffsetY + 221, 64, FONT_NORMAL_WHITE);

	Widget_GameText_draw(52, 17, baseOffsetX + 128, baseOffsetY + 248, FONT_NORMAL_WHITE);
	Widget_Text_drawNumber(Data_CityInfo.donateAmount, '@', " ",
		baseOffsetX + 316, baseOffsetY + 248, FONT_NORMAL_WHITE);

	Widget_GameText_drawCentered(13, 4,
		baseOffsetX + 336, baseOffsetY + 288, 160, FONT_NORMAL_BLACK);
	Widget_GameText_drawCentered(52, 18,
		baseOffsetX + 144, baseOffsetY + 288, 160, FONT_NORMAL_BLACK);
}

void UI_DonateToCityDialog_drawForeground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	Widget_Panel_drawButtonBorder(baseOffsetX + 128, baseOffsetY + 216, 64, 20, focusButtonId == 3);
	Widget_Panel_drawButtonBorder(baseOffsetX + 208, baseOffsetY + 216, 64, 20, focusButtonId == 4);
	Widget_Panel_drawButtonBorder(baseOffsetX + 288, baseOffsetY + 216, 64, 20, focusButtonId == 5);
	Widget_Panel_drawButtonBorder(baseOffsetX + 368, baseOffsetY + 216, 64, 20, focusButtonId == 6);
	Widget_Panel_drawButtonBorder(baseOffsetX + 448, baseOffsetY + 216, 64, 20, focusButtonId == 7);

	Widget_Panel_drawButtonBorder(baseOffsetX + 336, baseOffsetY + 283, 160, 20, focusButtonId == 1);
	Widget_Panel_drawButtonBorder(baseOffsetX + 144, baseOffsetY + 283, 160, 20, focusButtonId == 2);

	Widget_Button_drawArrowButtons(baseOffsetX, baseOffsetY, arrowButtons, 2);
}

void UI_DonateToCityDialog_handleMouse(const mouse *m)
{
	arrowButtonFocus = 0;
	if (m->right.went_up) {
		UI_Window_goTo(Window_Advisors);
	} else {
		int offsetX = Data_Screen.offset640x480.x;
		int offsetY = Data_Screen.offset640x480.y;
		if (!Widget_Button_handleCustomButtons(offsetX, offsetY, buttons, 7, &focusButtonId)) {
			arrowButtonFocus = Widget_Button_handleArrowButtons(offsetX, offsetY, arrowButtons, 2);
		}
	}
}

static void buttonSetAmount(int param1, int param2)
{
	int amount;
	switch (param1) {
		case 0: amount = 0; break;
		case 1: amount = 500; break;
		case 2: amount = 2000; break;
		case 3: amount = 5000; break;
		case 4: amount = 1000000; break;
		default: return;
	}
	Data_CityInfo.donateAmount = calc_bound(amount, 0, Data_CityInfo.personalSavings);
	UI_Window_requestRefresh();
}

static void buttonDonate(int param1, int param2)
{
	Data_CityInfo.treasury += Data_CityInfo.donateAmount;
	Data_CityInfo.financeDonatedThisYear += Data_CityInfo.donateAmount;
	Data_CityInfo.personalSavings -= Data_CityInfo.donateAmount;
	CityInfo_Finance_calculateTotals();
	UI_Window_goTo(Window_Advisors);
}

static void buttonCancel(int param1, int param2)
{
	UI_Window_goTo(Window_Advisors);
}

static void arrowButtonAmount(int isDown, int param2)
{
	if (isDown == 0) {
		Data_CityInfo.donateAmount += 10;
	} else if (isDown == 1) {
		Data_CityInfo.donateAmount -= 10;
	}
	Data_CityInfo.donateAmount = calc_bound(Data_CityInfo.donateAmount, 0, Data_CityInfo.personalSavings);
	UI_Window_requestRefresh();
}

void UI_DonateToCityDialog_getTooltip(struct TooltipContext *c)
{
	if (!focusButtonId && !arrowButtonFocus) {
		return;
	}
	c->type = TooltipType_Button;
	if (focusButtonId == 1) {
		c->textId = 98;
	} else if (focusButtonId == 2) {
		c->textId = 99;
	} else if (focusButtonId) {
		c->textId = 100;
	} else if (arrowButtonFocus) {
		c->textId = 101;
	}
}
