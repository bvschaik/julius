#include "Advisors_private.h"
#include "../Data/Scenario.h"
#include "Window.h"

static void drawRatingColumn(int xOffset, int yOffset, int value, int hasReached);
static void buttonRating(int param1, int param2);

static CustomButton ratingButtons[4] = {
	{ 80, 286, 190, 352, buttonRating, Widget_Button_doNothing, 1, 1, 0},
	{200, 286, 310, 352, buttonRating, Widget_Button_doNothing, 1, 2, 0},
	{320, 286, 430, 352, buttonRating, Widget_Button_doNothing, 1, 3, 0},
	{440, 286, 550, 352, buttonRating, Widget_Button_doNothing, 1, 4, 0},
};

static int focusButtonId;

void UI_Advisor_Ratings_drawBackground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;
	int width, hasReached;

	Widget_Panel_drawOuterPanel(baseOffsetX, baseOffsetY, 40, 27);
	Graphics_drawImage(GraphicId(ID_Graphic_AdvisorIcons) + 3,
		baseOffsetX + 10, baseOffsetY + 10);
	width = Widget_GameText_draw(53, 0,
		baseOffsetX + 60, baseOffsetY + 12, Font_LargeBlack);
	if (!Data_Scenario.winCriteria_populationEnabled || Data_Scenario.isOpenPlay) {
		Widget_GameText_draw(53, 7,
			baseOffsetX + 80 + width, baseOffsetY + 17, Font_NormalBlack);
	} else {
		width += Widget_GameText_draw(53, 6,
			baseOffsetX + 80 + width, baseOffsetY + 17, Font_NormalBlack);
		Widget_Text_drawNumber(Data_Scenario.winCriteria_population, '@', ")",
			baseOffsetX + 80 + width, baseOffsetY + 17, Font_NormalBlack);
	}

	Graphics_drawImage(GraphicId(ID_Graphic_RatingsBackground),
		baseOffsetX + 60, baseOffsetY + 48);

	// culture
	Widget_Panel_drawButtonBorder(baseOffsetX + 80, baseOffsetY + 286,
		110, 66, focusButtonId == 1);
	Widget_GameText_drawCentered(53, 1, baseOffsetX + 80, baseOffsetY + 294,
		110, Font_NormalBlack);
	Widget_Text_drawNumberCentered(Data_CityInfo.ratingCulture, '@', " ",
		baseOffsetX + 80, baseOffsetY + 309, 100, Font_LargeBlack);
	if (Data_Scenario.winCriteria.cultureEnabled) {
		width = Widget_Text_drawNumber(Data_Scenario.winCriteria.culture, '@', " ",
			baseOffsetX + 85, baseOffsetY + 334, Font_NormalBlack);
	} else {
		width = Widget_Text_drawNumber(0, '@', " ",
			baseOffsetX + 85, baseOffsetY + 334, Font_NormalBlack);
	}
	Widget_GameText_draw(53, 5, baseOffsetX + 85 + width, baseOffsetY + 334, Font_NormalBlack);
	hasReached = !Data_Scenario.winCriteria.cultureEnabled ||
		Data_CityInfo.ratingCulture > Data_Scenario.winCriteria.culture ||
		Data_CityInfo.ratingCulture == 100; // FIXME: bug fixed
	drawRatingColumn(baseOffsetX + 110, baseOffsetY + 274, Data_CityInfo.ratingCulture, hasReached);

	// prosperity
	Widget_Panel_drawButtonBorder(baseOffsetX + 200, baseOffsetY + 286,
		110, 66, focusButtonId == 2);
	Widget_GameText_drawCentered(53, 2, baseOffsetX + 200, baseOffsetY + 294,
		110, Font_NormalBlack);
	Widget_Text_drawNumberCentered(Data_CityInfo.ratingProsperity, '@', " ",
		baseOffsetX + 200, baseOffsetY + 309, 100, Font_LargeBlack);
	if (Data_Scenario.winCriteria.prosperityEnabled) {
		width = Widget_Text_drawNumber(Data_Scenario.winCriteria.prosperity, '@', " ",
			baseOffsetX + 205, baseOffsetY + 334, Font_NormalBlack);
	} else {
		width = Widget_Text_drawNumber(0, '@', " ",
			baseOffsetX + 205, baseOffsetY + 334, Font_NormalBlack);
	}
	Widget_GameText_draw(53, 5, baseOffsetX + 205 + width, baseOffsetY + 334, Font_NormalBlack);
	hasReached = !Data_Scenario.winCriteria.prosperityEnabled ||
		Data_CityInfo.ratingProsperity > Data_Scenario.winCriteria.prosperity ||
		Data_CityInfo.ratingProsperity == 100; // FIXME: bug fixed
	drawRatingColumn(baseOffsetX + 230, baseOffsetY + 274, Data_CityInfo.ratingProsperity, hasReached);

	// peace
	Widget_Panel_drawButtonBorder(baseOffsetX + 320, baseOffsetY + 286,
		110, 66, focusButtonId == 3);
	Widget_GameText_drawCentered(53, 3, baseOffsetX + 320, baseOffsetY + 294,
		110, Font_NormalBlack);
	Widget_Text_drawNumberCentered(Data_CityInfo.ratingPeace, '@', " ",
		baseOffsetX + 320, baseOffsetY + 309, 100, Font_LargeBlack);
	if (Data_Scenario.winCriteria.peaceEnabled) {
		width = Widget_Text_drawNumber(Data_Scenario.winCriteria.peace, '@', " ",
			baseOffsetX + 325, baseOffsetY + 334, Font_NormalBlack);
	} else {
		width = Widget_Text_drawNumber(0, '@', " ",
			baseOffsetX + 325, baseOffsetY + 334, Font_NormalBlack);
	}
	Widget_GameText_draw(53, 5, baseOffsetX + 325 + width, baseOffsetY + 334, Font_NormalBlack);
	hasReached = !Data_Scenario.winCriteria.peaceEnabled ||
		Data_CityInfo.ratingPeace > Data_Scenario.winCriteria.peace ||
		Data_CityInfo.ratingPeace == 100; // FIXME: bug fixed: no capital
	drawRatingColumn(baseOffsetX + 350, baseOffsetY + 274, Data_CityInfo.ratingPeace, hasReached);

	// favor
	Widget_Panel_drawButtonBorder(baseOffsetX + 440, baseOffsetY + 286,
		110, 66, focusButtonId == 4);
	Widget_GameText_drawCentered(53, 4, baseOffsetX + 440, baseOffsetY + 294,
		110, Font_NormalBlack);
	Widget_Text_drawNumberCentered(Data_CityInfo.ratingFavor, '@', " ",
		baseOffsetX + 440, baseOffsetY + 309, 100, Font_LargeBlack);
	if (Data_Scenario.winCriteria.peaceEnabled) {
		width = Widget_Text_drawNumber(Data_Scenario.winCriteria.favor, '@', " ",
			baseOffsetX + 445, baseOffsetY + 334, Font_NormalBlack);
	} else {
		width = Widget_Text_drawNumber(0, '@', " ",
			baseOffsetX + 445, baseOffsetY + 334, Font_NormalBlack);
	}
	Widget_GameText_draw(53, 5, baseOffsetX + 445 + width, baseOffsetY + 334, Font_NormalBlack);
	hasReached = !Data_Scenario.winCriteria.favorEnabled ||
		Data_CityInfo.ratingFavor > Data_Scenario.winCriteria.favor ||
		Data_CityInfo.ratingFavor == 100; // FIXME: bug fixed
	drawRatingColumn(baseOffsetX + 470, baseOffsetY + 274, Data_CityInfo.ratingPeace, hasReached);

	// bottom info box
	Widget_Panel_drawInnerPanel(baseOffsetX + 64, baseOffsetY + 356, 32, 4);
	switch (Data_CityInfo.ratingAdvisorSelection) {
		case 1:
			Widget_GameText_draw(53, 1,
				baseOffsetX + 72, baseOffsetY + 359, Font_NormalWhite);
			if (Data_CityInfo.ratingCulture <= 90) {
				Widget_GameText_drawMultiline(53, 9 + Data_CityInfo.ratingAdvisorExplanationCulture,
					baseOffsetX + 72, baseOffsetY + 374, 496, Font_NormalWhite);
			} else {
				Widget_GameText_drawMultiline(53, 50,
					baseOffsetX + 72, baseOffsetY + 374, 496, Font_NormalWhite);
			}
			break;
		case 2:
			Widget_GameText_draw(53, 2,
				baseOffsetX + 72, baseOffsetY + 359, Font_NormalWhite);
			if (Data_CityInfo.ratingProsperity <= 90) {
				Widget_GameText_drawMultiline(53, 16 + Data_CityInfo.ratingAdvisorExplanationProsperity,
					baseOffsetX + 72, baseOffsetY + 374, 496, Font_NormalWhite);
			} else {
				Widget_GameText_drawMultiline(53, 51,
					baseOffsetX + 72, baseOffsetY + 374, 496, Font_NormalWhite);
			}
			break;
		case 3:
			Widget_GameText_draw(53, 3,
				baseOffsetX + 72, baseOffsetY + 359, Font_NormalWhite);
			if (Data_CityInfo.ratingPeace <= 90) {
				Widget_GameText_drawMultiline(53, 41 + Data_CityInfo.ratingAdvisorExplanationPeace,
					baseOffsetX + 72, baseOffsetY + 374, 496, Font_NormalWhite);
			} else {
				Widget_GameText_drawMultiline(53, 52,
					baseOffsetX + 72, baseOffsetY + 374, 496, Font_NormalWhite);
			}
			break;
		case 4:
			Widget_GameText_draw(53, 4,
				baseOffsetX + 72, baseOffsetY + 359, Font_NormalWhite);
			if (Data_CityInfo.ratingFavor <= 90) {
				Widget_GameText_drawMultiline(53, 27 + Data_CityInfo.ratingAdvisorExplanationFavor,
					baseOffsetX + 72, baseOffsetY + 374, 496, Font_NormalWhite);
			} else {
				Widget_GameText_drawMultiline(53, 53,
					baseOffsetX + 72, baseOffsetY + 374, 496, Font_NormalWhite);
			}
			break;
		default:
			Widget_GameText_drawCentered(53, 8,
				baseOffsetX + 72, baseOffsetY + 380, 496, Font_NormalWhite);
			break;
	}
}

static void drawRatingColumn(int xOffset, int yOffset, int value, int hasReached)
{
	int graphicBase = GraphicId(ID_Graphic_RatingsColumn);
	int y = yOffset - Data_Graphics_Main.index[graphicBase].height;
	Graphics_drawImage(graphicBase, xOffset, y);
	for (int i = 0; i < 2 * value; i++) {
		Graphics_drawImage(graphicBase + 1, xOffset + 11, --y);
	}
	if (value > 30 && hasReached) {
		Graphics_drawImage(graphicBase + 2, xOffset - 6, y);
	}
}

void UI_Advisor_Ratings_drawForeground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	// culture
	Widget_Panel_drawButtonBorder(baseOffsetX + 80, baseOffsetY + 286,
		110, 66, focusButtonId == 1);
	// prosperity
	Widget_Panel_drawButtonBorder(baseOffsetX + 200, baseOffsetY + 286,
		110, 66, focusButtonId == 2);
	// peace
	Widget_Panel_drawButtonBorder(baseOffsetX + 320, baseOffsetY + 286,
		110, 66, focusButtonId == 3);
	// favor
	Widget_Panel_drawButtonBorder(baseOffsetX + 440, baseOffsetY + 286,
		110, 66, focusButtonId == 4);
}

void UI_Advisor_Ratings_handleMouse()
{
	Widget_Button_handleCustomButtons(
		Data_Screen.offset640x480.x, Data_Screen.offset640x480.y,
		ratingButtons, 4, &focusButtonId);
}

static void buttonRating(int param1, int param2)
{
	Data_CityInfo.ratingAdvisorSelection = param1;
	UI_Window_requestRefresh();
}
