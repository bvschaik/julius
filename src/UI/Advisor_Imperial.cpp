#include "Advisors_private.h"
#include "Window.h"

#include "../CityInfoUpdater.h"

#include "../Data/Settings.h"

static void buttonDonateToCity(int param1, int param2);
static void buttonSetSalary(int param1, int param2);
static void buttonGiftToCaesar(int param1, int param2);
static void buttonRequest(int param1, int param2);

static CustomButton imperialButtons[8] = {
	{320, 367, 570, 387, buttonDonateToCity, Widget_Button_doNothing, 1, 0, 0},
	{70, 393, 570, 413, buttonSetSalary, Widget_Button_doNothing, 1, 0, 0},
	{320, 341, 570, 361, buttonGiftToCaesar, Widget_Button_doNothing, 1, 0, 0},
	{38, 96, 598, 136, buttonRequest, Widget_Button_doNothing, 1, 0, 0},
	{38, 138, 598, 178, buttonRequest, Widget_Button_doNothing, 1, 1, 0},
	{38, 180, 598, 220, buttonRequest, Widget_Button_doNothing, 1, 2, 0},
	{38, 222, 598, 262, buttonRequest, Widget_Button_doNothing, 1, 3, 0},
	{38, 264, 598, 304, buttonRequest, Widget_Button_doNothing, 1, 4, 0},
};

static int focusButtonId;

void UI_Advisor_Imperial_drawBackground()
{
	CityInfoUpdater_Imperial_calculateGiftCosts();

	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	Widget_Panel_drawOuterPanel(baseOffsetX, baseOffsetY, 40, 27);
	Graphics_drawImage(GraphicId(ID_Graphic_AdvisorIcons) + 2, baseOffsetX + 10, baseOffsetY + 10);

	Widget_Text_draw(Data_Settings.playerName, baseOffsetX + 60, baseOffsetY + 12, Font_LargeBlack, 0);

	int width = Widget_GameText_draw(52, 0, baseOffsetX + 60, baseOffsetY + 44, Font_NormalBlack, 0);
	Widget_Text_drawNumber(Data_CityInfo.ratingFavor, '@', " ", baseOffsetX + 60 + width, baseOffsetY + 44, Font_NormalBlack, 0);

	Widget_GameText_drawMultiline(52, Data_CityInfo.ratingFavor / 5 + 22,
		baseOffsetX + 60, baseOffsetY + 60, 544, Font_NormalBlack, 0);

	Widget_Panel_drawInnerPanel(baseOffsetX + 32, baseOffsetY + 90, 36, 14);
	
	// TODO
}

void UI_Advisor_Imperial_drawForeground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	Widget_Panel_drawInnerPanel(baseOffsetX + 64, baseOffsetY + 324, 32, 6);

	Widget_GameText_draw(32, Data_CityInfo.playerRank,
		baseOffsetX + 72, baseOffsetY + 338, Font_LargeBrown, 0);
	
	int width = Widget_GameText_draw(52, 1,
		baseOffsetX + 72, baseOffsetY + 372, Font_NormalWhite, 0);
	Widget_Text_drawNumber(Data_CityInfo.personalSavings, '@', " Dn",
		baseOffsetX + 72 + width, baseOffsetY + 372, Font_NormalWhite, 0);

	Widget_Panel_drawButtonBorder(baseOffsetX + 320, baseOffsetY + 367,
		250, 20, focusButtonId == 1);
	Widget_GameText_drawCentered(52, 2, baseOffsetX + 320, baseOffsetY + 372, 250, Font_NormalWhite, 0);

	Widget_Panel_drawButtonBorder(baseOffsetX + 70, baseOffsetY + 393,
		500, 20, focusButtonId == 2);
	width = Widget_GameText_draw(52, Data_CityInfo.salaryRank + 4,
		baseOffsetX + 120, baseOffsetY + 398, Font_NormalWhite, 0);
	width += Widget_Text_drawNumber(Data_CityInfo.salaryAmount, '@', " ",
		baseOffsetX + 120 + width, baseOffsetY + 398, Font_NormalWhite, 0);
	Widget_GameText_draw(52, 3, baseOffsetX + 120 + width, baseOffsetY + 398, Font_NormalWhite, 0);

	Widget_Panel_drawButtonBorder(baseOffsetX + 320, baseOffsetY + 341,
		250, 20, focusButtonId == 3);
	Widget_GameText_drawCentered(52, 49, baseOffsetX + 320, baseOffsetY + 346, 250, Font_NormalWhite, 0);

	// Request buttons
	// TODO
}

void UI_Advisor_Imperial_handleMouse()
{
	int offsetX = Data_Screen.offset640x480.x;
	int offsetY = Data_Screen.offset640x480.y;
	Widget_Button_handleCustomButtons(offsetX, offsetY,
		imperialButtons, 8, &focusButtonId);
}

static void buttonDonateToCity(int param1, int param2)
{
}

static void buttonSetSalary(int param1, int param2)
{
	UI_Window_goTo(Window_SetSalaryDialog);
}

static void buttonGiftToCaesar(int param1, int param2)
{
	UI_Window_goTo(Window_SendGiftToCaesarDialog);
}

static void buttonRequest(int param1, int param2)
{
}

