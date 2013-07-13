#include "AllWindows.h"
#include "Window.h"
#include "../Widget.h"
#include "../Graphics.h"
#include "../CityInfoUpdater.h"
#include "Advisors_private.h"
#include "../Data/CityInfo.h"
#include "../Data/Screen.h"
#include "../Data/Mouse.h"

static void buttonSetGift(int param1, int param2);
static void buttonSendGift(int param1, int param2);
static void buttonCancel(int param1, int param2);

static CustomButton buttons[5] = {
	{208, 213, 528, 233, buttonSetGift, Widget_Button_doNothing, 1, 1, 0},
	{208, 233, 528, 253, buttonSetGift, Widget_Button_doNothing, 1, 2, 0},
	{208, 253, 528, 273, buttonSetGift, Widget_Button_doNothing, 1, 3, 0},
	{128, 336, 368, 356, buttonSendGift, Widget_Button_doNothing, 1, 0, 0},
	{400, 336, 560, 356, buttonCancel, Widget_Button_doNothing, 1, 0, 0},
};

int focusButtonId;

void UI_SendGiftToCaesarDialog_init()
{
	if (Data_CityInfo.giftSizeSelected == 2 &&
		Data_CityInfo.giftCost_lavish > Data_CityInfo.personalSavings) {
		Data_CityInfo.giftSizeSelected = 1;
	}
	if (Data_CityInfo.giftSizeSelected == 2 &&
		Data_CityInfo.giftCost_generous > Data_CityInfo.personalSavings) {
		Data_CityInfo.giftSizeSelected = 0;
	}
}

void UI_SendGiftToCaesarDialog_drawBackground()
{
	UI_Advisor_drawGeneralBackground();

	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	Widget_Panel_drawOuterPanel(baseOffsetX + 96, baseOffsetY + 144, 30, 15);
	Graphics_drawImage(GraphicId(ID_Graphic_ResourceIcons) + 16,
		baseOffsetX + 128, baseOffsetY + 160);
	Widget_GameText_drawCentered(52, 69,
		baseOffsetX + 128, baseOffsetY + 160, 432, Font_LargeBlack, 0);
	Widget_Panel_drawInnerPanel(baseOffsetX + 112, baseOffsetY + 208, 28, 5);

	int width = Widget_GameText_draw(52, 50,
		baseOffsetX + 144, baseOffsetY + 304, Font_NormalBlack, 0);
	Widget_GameText_drawNumberWithDescription(8, 4,
		Data_CityInfo.giftMonthsSinceLast,
		baseOffsetX + 144 + width, baseOffsetY + 304, Font_NormalBlack, 0);
	Widget_GameText_drawCentered(13, 4,
		baseOffsetX + 400, baseOffsetY + 341, 160, Font_NormalBlack, 0);
}

void UI_SendGiftToCaesarDialog_drawForeground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	if (Data_CityInfo.giftCost_modest <= Data_CityInfo.personalSavings) {
		Widget_GameText_draw(52, 63,
			baseOffsetX + 128, baseOffsetY + 218, Font_NormalWhite, 0);
		Font font = focusButtonId == 1 ? Font_NormalRed : Font_NormalWhite;
		int width = Widget_GameText_draw(52, 51 + Data_CityInfo.giftId_modest,
			baseOffsetX + 224, baseOffsetY + 218, font, 0);
		Widget_Text_drawNumber(Data_CityInfo.giftCost_modest, '@', "Dn",
			baseOffsetX + 224 + width, baseOffsetY + 218, font, 0);
	} else {
		Widget_GameText_drawMultiline(52, 70,
			baseOffsetX + 160, baseOffsetY + 224, 352, Font_NormalWhite, 0);
	}
	if (Data_CityInfo.giftCost_generous <= Data_CityInfo.personalSavings) {
		Widget_GameText_draw(52, 64,
			baseOffsetX + 128, baseOffsetY + 238, Font_NormalWhite, 0);
		Font font = focusButtonId == 2 ? Font_NormalRed : Font_NormalWhite;
		int width = Widget_GameText_draw(52, 55 + Data_CityInfo.giftId_generous,
			baseOffsetX + 224, baseOffsetY + 238, font, 0);
		Widget_Text_drawNumber(Data_CityInfo.giftCost_generous, '@', "Dn",
			baseOffsetX + 224 + width, baseOffsetY + 238, font, 0);
	}
	if (Data_CityInfo.giftCost_lavish <= Data_CityInfo.personalSavings) {
		Widget_GameText_draw(52, 65,
			baseOffsetX + 128, baseOffsetY + 258, Font_NormalWhite, 0);
		Font font = focusButtonId == 3 ? Font_NormalRed : Font_NormalWhite;
		int width = Widget_GameText_draw(52, 59 + Data_CityInfo.giftId_lavish,
			baseOffsetX + 224, baseOffsetY + 258, font, 0);
		Widget_Text_drawNumber(Data_CityInfo.giftCost_lavish, '@', "Dn",
			baseOffsetX + 224 + width, baseOffsetY + 258, font, 0);
	}
	// can give at least one type
	if (Data_CityInfo.giftCost_modest <= Data_CityInfo.personalSavings) {
		Widget_GameText_drawCentered(52, 66 + Data_CityInfo.giftSizeSelected,
			baseOffsetX + 128, baseOffsetY + 341, 240, Font_NormalBlack, 0);
		Widget_Panel_drawButtonBorder(
			baseOffsetX + 128, baseOffsetY + 336, 240, 20, focusButtonId == 4);
	}
	Widget_Panel_drawButtonBorder(
		baseOffsetX + 400, baseOffsetY + 336, 160, 20, focusButtonId == 5);
}

void UI_SendGiftToCaesarDialog_handleMouse()
{
	if (Data_Mouse.isRightClick) {
		UI_Window_goTo(Window_Advisors);
	} else {
		int offsetX = Data_Screen.offset640x480.x;
		int offsetY = Data_Screen.offset640x480.y;
		Widget_Button_handleCustomButtons(offsetX, offsetY,
			buttons, 5, &focusButtonId);
	}
}

static void buttonSetGift(int param1, int param2)
{
	if ((param1 == 1 && Data_CityInfo.giftCost_modest <= Data_CityInfo.personalSavings) ||
		(param1 == 2 && Data_CityInfo.giftCost_generous <= Data_CityInfo.personalSavings) ||
		(param1 == 3 && Data_CityInfo.giftCost_lavish <= Data_CityInfo.personalSavings)) {
		Data_CityInfo.giftSizeSelected = param1 - 1;
		UI_Window_requestRefresh();
	}
}

static void buttonSendGift(int param1, int param2)
{
	if (Data_CityInfo.giftCost_modest <= Data_CityInfo.personalSavings) {
		CityInfoUpdater_Ratings_sendGiftToCaesar();
		UI_Window_goTo(Window_Advisors);
	}
}

static void buttonCancel(int param1, int param2)
{
	UI_Window_goTo(Window_Advisors);
}

