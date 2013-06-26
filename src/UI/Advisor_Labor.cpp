#include "Advisors_private.h"
#include "../Data/Mouse.h"

static void arrowButtonWages(int param1, int param2);
static void buttonPriority(int param1, int param2);

static CustomButton categoryButtons[9] = {
	{40, 77, 600, 99, buttonPriority, Widget_Button_doNothing, 1, 0, 0},
	{40, 102, 600, 124, buttonPriority, Widget_Button_doNothing, 1, 1, 0},
	{40, 127, 600, 149, buttonPriority, Widget_Button_doNothing, 1, 2, 0},
	{40, 152, 600, 174, buttonPriority, Widget_Button_doNothing, 1, 3, 0},
	{40, 177, 600, 199, buttonPriority, Widget_Button_doNothing, 1, 4, 0},
	{40, 202, 600, 224, buttonPriority, Widget_Button_doNothing, 1, 5, 0},
	{40, 227, 600, 249, buttonPriority, Widget_Button_doNothing, 1, 6, 0},
	{40, 252, 600, 274, buttonPriority, Widget_Button_doNothing, 1, 7, 0},
	{40, 277, 600, 299, buttonPriority, Widget_Button_doNothing, 1, 8, 0},
};

static ArrowButton wageButtons[2] = {
	{158, 354, 17, 24, arrowButtonWages, 1, 0},
	{182, 354, 15, 24, arrowButtonWages, 0, 0}
};

static int focusButtonId;

void UI_Advisor_Labor_drawBackground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;
	
	Widget_Panel_drawOuterPanel(baseOffsetX, baseOffsetY, 40, 26);
	Graphics_drawImage(GraphicId(ID_Graphic_AdvisorIcons), baseOffsetX + 10, baseOffsetY + 10);
	
	Widget_GameText_draw(50, 0, baseOffsetX + 60, baseOffsetY + 12, Font_LargeBlack, 0);
	
	// table headers
	Widget_GameText_draw(50, 21, baseOffsetX + 60, baseOffsetY + 56, Font_SmallPlain, 0);
	Widget_GameText_draw(50, 22, baseOffsetX + 170, baseOffsetY + 56, Font_SmallPlain, 0);
	Widget_GameText_draw(50, 23, baseOffsetX + 400, baseOffsetY + 56, Font_SmallPlain, 0);
	Widget_GameText_draw(50, 24, baseOffsetX + 500, baseOffsetY + 56, Font_SmallPlain, 0);
	
	int width = Widget_Text_drawNumber(Data_CityInfo.workersEmployed, '@', " ",
		baseOffsetX + 32, baseOffsetY + 320, Font_NormalBlack, 0
	);
	width += Widget_GameText_draw(50, 12,
		baseOffsetX + 32 + width, baseOffsetY + 320, Font_NormalBlack, 0
	);
	width += Widget_Text_drawNumber(Data_CityInfo.workersUnemployed, '@', " ",
		baseOffsetX + 50 + width, baseOffsetY + 320, Font_NormalBlack, 0
	);
	width += Widget_GameText_draw(50, 13,
		baseOffsetX + 50 + width, baseOffsetY + 320, Font_NormalBlack, 0
	);
	width += Widget_Text_drawNumber(Data_CityInfo.unemploymentPercentage, '@', "%)",
		baseOffsetX + 50 + width, baseOffsetY + 320, Font_NormalBlack, 0
	);
	
	Widget_Panel_drawInnerPanel(baseOffsetX + 64, baseOffsetY + 350, 32, 2);
	Widget_GameText_draw(50, 14, baseOffsetX + 70, baseOffsetY + 359, Font_NormalWhite, 0);
	
	width = Widget_Text_drawNumber(Data_CityInfo.wages, '@', " ",
		baseOffsetX + 230, baseOffsetY + 359, Font_NormalWhite, 0
	);
	width += Widget_GameText_draw(50, 15,
		baseOffsetX + 230 + width, baseOffsetY + 359, Font_NormalWhite, 0
	);
	width += Widget_GameText_draw(50, 18,
		baseOffsetX + 230 + width, baseOffsetY + 359, Font_NormalWhite, 0
	);
	width += Widget_Text_drawNumber(Data_CityInfo.wagesRome, '@', " )",
		baseOffsetX + 230 + width, baseOffsetY + 359, Font_NormalWhite, 0
	);
	
	width = Widget_GameText_draw(50, 19,
		baseOffsetX + 64, baseOffsetY + 390, Font_NormalBlack, 0
	);
	width += Widget_Text_drawNumber(Data_CityInfo.estimatedYearlyWages, '@', " Dn",
		baseOffsetX + 64 + width, baseOffsetY + 390, Font_NormalBlack, 0
	);
}

void UI_Advisor_Labor_drawForeground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	Widget_Button_drawArrowButtons(
		baseOffsetX, baseOffsetY, wageButtons, 2);

	Widget_Panel_drawInnerPanel(baseOffsetX + 32, baseOffsetY + 70, 36, 15);

	for (int i = 0; i < 9; i++) {
		int focus = i == focusButtonId - 1;
		Widget_Panel_drawButtonBorder(
			baseOffsetX + 40, baseOffsetY + 77 + 25 * i,
			560, 22, focus
		);
		if (Data_CityInfo.laborCategory[i].priority) {
			Graphics_drawImage(GraphicId(ID_Graphic_LaborPriorityLock),
				baseOffsetX + 70, baseOffsetY + 80 + 25 * i);
			Widget_Text_drawNumber(Data_CityInfo.laborCategory[i].priority, '@', " ",
				baseOffsetX + 90, baseOffsetY + 82 + 25 * i, Font_NormalWhite, 0);
		}
		Widget_GameText_draw(50, i + 1,
			baseOffsetX + 170, baseOffsetY + 82 + 25 * i, Font_NormalWhite, 0);
		Widget_Text_drawNumber(Data_CityInfo.laborCategory[i].workersNeeded, '@', " ",
			baseOffsetX + 410, baseOffsetY + 82 + 25 * i, Font_NormalWhite, 0);
		Font font = Font_NormalWhite;
		if (Data_CityInfo.laborCategory[i].workersNeeded != Data_CityInfo.laborCategory[i].workersAllocated) {
			font = Font_NormalRed;
		}
		Widget_Text_drawNumber(Data_CityInfo.laborCategory[i].workersAllocated, '@', " ",
			baseOffsetX + 510, baseOffsetY + 82 + 25 * i, font, 0);
	}
}

void UI_Advisor_Labor_handleMouse()
{
	int offsetX = Data_Screen.offset640x480.x;
	int offsetY = Data_Screen.offset640x480.y;
	if (!Widget_Button_handleCustomButtons(offsetX, offsetY, categoryButtons, 9, &focusButtonId)) {
		Widget_Button_handleArrowButtons(offsetX, offsetY, wageButtons, 2);
	}
}

static void arrowButtonWages(int param1, int param2)
{
	// TODO
}

static void buttonPriority(int param1, int param2)
{
	// TODO
}
