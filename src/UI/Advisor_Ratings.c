#include "Advisors_private.h"

#include "Window.h"

#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "scenario/criteria.h"
#include "scenario/property.h"

static void drawRatingColumn(int xOffset, int yOffset, int value, int hasReached);
static void buttonRating(int param1, int param2);

static generic_button ratingButtons[] = {
	{ 80, 286, 190, 352, GB_IMMEDIATE, buttonRating, button_none, 1, 0},
	{200, 286, 310, 352, GB_IMMEDIATE, buttonRating, button_none, 2, 0},
	{320, 286, 430, 352, GB_IMMEDIATE, buttonRating, button_none, 3, 0},
	{440, 286, 550, 352, GB_IMMEDIATE, buttonRating, button_none, 4, 0},
};

static int focusButtonId;

void UI_Advisor_Ratings_drawBackground(int *advisorHeight)
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;
	int width, hasReached;

	*advisorHeight = 27;
	outer_panel_draw(baseOffsetX, baseOffsetY, 40, *advisorHeight);
	Graphics_drawImage(image_group(GROUP_ADVISOR_ICONS) + 3,
		baseOffsetX + 10, baseOffsetY + 10);
	width = Widget_GameText_draw(53, 0,
		baseOffsetX + 60, baseOffsetY + 12, FONT_LARGE_BLACK);
	if (!scenario_criteria_population_enabled() || scenario_is_open_play()) {
		Widget_GameText_draw(53, 7,
			baseOffsetX + 80 + width, baseOffsetY + 17, FONT_NORMAL_BLACK);
	} else {
		width += Widget_GameText_draw(53, 6,
			baseOffsetX + 80 + width, baseOffsetY + 17, FONT_NORMAL_BLACK);
		Widget_Text_drawNumber(scenario_criteria_population(), '@', ")",
			baseOffsetX + 80 + width, baseOffsetY + 17, FONT_NORMAL_BLACK);
	}

	Graphics_drawImage(image_group(GROUP_RATINGS_BACKGROUND),
		baseOffsetX + 60, baseOffsetY + 48);

	// culture
	button_border_draw(baseOffsetX + 80, baseOffsetY + 286,
		110, 66, focusButtonId == 1);
	Widget_GameText_drawCentered(53, 1, baseOffsetX + 80, baseOffsetY + 294,
		110, FONT_NORMAL_BLACK);
	Widget_Text_drawNumberCentered(Data_CityInfo.ratingCulture,
		baseOffsetX + 80, baseOffsetY + 309, 100, FONT_LARGE_BLACK);
	if (scenario_criteria_culture_enabled()) {
		width = Widget_Text_drawNumber(scenario_criteria_culture(), '@', " ",
			baseOffsetX + 85, baseOffsetY + 334, FONT_NORMAL_BLACK);
	} else {
		width = Widget_Text_drawNumber(0, '@', " ",
			baseOffsetX + 85, baseOffsetY + 334, FONT_NORMAL_BLACK);
	}
	Widget_GameText_draw(53, 5, baseOffsetX + 85 + width, baseOffsetY + 334, FONT_NORMAL_BLACK);
	hasReached = !scenario_criteria_culture_enabled() ||
		Data_CityInfo.ratingCulture > scenario_criteria_culture() ||
		Data_CityInfo.ratingCulture == 100; // FIXED: capital bug fixed
	drawRatingColumn(baseOffsetX + 110, baseOffsetY + 274, Data_CityInfo.ratingCulture, hasReached);

	// prosperity
	button_border_draw(baseOffsetX + 200, baseOffsetY + 286,
		110, 66, focusButtonId == 2);
	Widget_GameText_drawCentered(53, 2, baseOffsetX + 200, baseOffsetY + 294,
		110, FONT_NORMAL_BLACK);
	Widget_Text_drawNumberCentered(Data_CityInfo.ratingProsperity,
		baseOffsetX + 200, baseOffsetY + 309, 100, FONT_LARGE_BLACK);
	if (scenario_criteria_prosperity_enabled()) {
		width = Widget_Text_drawNumber(scenario_criteria_prosperity(), '@', " ",
			baseOffsetX + 205, baseOffsetY + 334, FONT_NORMAL_BLACK);
	} else {
		width = Widget_Text_drawNumber(0, '@', " ",
			baseOffsetX + 205, baseOffsetY + 334, FONT_NORMAL_BLACK);
	}
	Widget_GameText_draw(53, 5, baseOffsetX + 205 + width, baseOffsetY + 334, FONT_NORMAL_BLACK);
	hasReached = !scenario_criteria_prosperity_enabled() ||
		Data_CityInfo.ratingProsperity > scenario_criteria_prosperity() ||
		Data_CityInfo.ratingProsperity == 100; // FIXED: capital bug fixed
	drawRatingColumn(baseOffsetX + 230, baseOffsetY + 274, Data_CityInfo.ratingProsperity, hasReached);

	// peace
	button_border_draw(baseOffsetX + 320, baseOffsetY + 286,
		110, 66, focusButtonId == 3);
	Widget_GameText_drawCentered(53, 3, baseOffsetX + 320, baseOffsetY + 294,
		110, FONT_NORMAL_BLACK);
	Widget_Text_drawNumberCentered(Data_CityInfo.ratingPeace,
		baseOffsetX + 320, baseOffsetY + 309, 100, FONT_LARGE_BLACK);
	if (scenario_criteria_peace_enabled()) {
		width = Widget_Text_drawNumber(scenario_criteria_peace(), '@', " ",
			baseOffsetX + 325, baseOffsetY + 334, FONT_NORMAL_BLACK);
	} else {
		width = Widget_Text_drawNumber(0, '@', " ",
			baseOffsetX + 325, baseOffsetY + 334, FONT_NORMAL_BLACK);
	}
	Widget_GameText_draw(53, 5, baseOffsetX + 325 + width, baseOffsetY + 334, FONT_NORMAL_BLACK);
	hasReached = !scenario_criteria_peace_enabled() ||
		Data_CityInfo.ratingPeace > scenario_criteria_peace() ||
		Data_CityInfo.ratingPeace == 100; // FIXED: capital bug fixed
	drawRatingColumn(baseOffsetX + 350, baseOffsetY + 274, Data_CityInfo.ratingPeace, hasReached);

	// favor
	button_border_draw(baseOffsetX + 440, baseOffsetY + 286,
		110, 66, focusButtonId == 4);
	Widget_GameText_drawCentered(53, 4, baseOffsetX + 440, baseOffsetY + 294,
		110, FONT_NORMAL_BLACK);
	Widget_Text_drawNumberCentered(Data_CityInfo.ratingFavor,
		baseOffsetX + 440, baseOffsetY + 309, 100, FONT_LARGE_BLACK);
	if (scenario_criteria_favor_enabled()) {
		width = Widget_Text_drawNumber(scenario_criteria_favor(), '@', " ",
			baseOffsetX + 445, baseOffsetY + 334, FONT_NORMAL_BLACK);
	} else {
		width = Widget_Text_drawNumber(0, '@', " ",
			baseOffsetX + 445, baseOffsetY + 334, FONT_NORMAL_BLACK);
	}
	Widget_GameText_draw(53, 5, baseOffsetX + 445 + width, baseOffsetY + 334, FONT_NORMAL_BLACK);
	hasReached = !scenario_criteria_favor_enabled() ||
		Data_CityInfo.ratingFavor > scenario_criteria_favor() ||
		Data_CityInfo.ratingFavor == 100; // FIXED: capital bug fixed
	drawRatingColumn(baseOffsetX + 470, baseOffsetY + 274, Data_CityInfo.ratingFavor, hasReached);

	// bottom info box
	inner_panel_draw(baseOffsetX + 64, baseOffsetY + 356, 32, 4);
	switch (Data_CityInfo.ratingAdvisorSelection) {
		case 1:
			Widget_GameText_draw(53, 1,
				baseOffsetX + 72, baseOffsetY + 359, FONT_NORMAL_WHITE);
			if (Data_CityInfo.ratingCulture <= 90) {
				Widget_GameText_drawMultiline(53, 9 + Data_CityInfo.ratingAdvisorExplanationCulture,
					baseOffsetX + 72, baseOffsetY + 374, 496, FONT_NORMAL_WHITE);
			} else {
				Widget_GameText_drawMultiline(53, 50,
					baseOffsetX + 72, baseOffsetY + 374, 496, FONT_NORMAL_WHITE);
			}
			break;
		case 2:
			Widget_GameText_draw(53, 2,
				baseOffsetX + 72, baseOffsetY + 359, FONT_NORMAL_WHITE);
			if (Data_CityInfo.ratingProsperity <= 90) {
				Widget_GameText_drawMultiline(53, 16 + Data_CityInfo.ratingAdvisorExplanationProsperity,
					baseOffsetX + 72, baseOffsetY + 374, 496, FONT_NORMAL_WHITE);
			} else {
				Widget_GameText_drawMultiline(53, 51,
					baseOffsetX + 72, baseOffsetY + 374, 496, FONT_NORMAL_WHITE);
			}
			break;
		case 3:
			Widget_GameText_draw(53, 3,
				baseOffsetX + 72, baseOffsetY + 359, FONT_NORMAL_WHITE);
			if (Data_CityInfo.ratingPeace <= 90) {
				Widget_GameText_drawMultiline(53, 41 + Data_CityInfo.ratingAdvisorExplanationPeace,
					baseOffsetX + 72, baseOffsetY + 374, 496, FONT_NORMAL_WHITE);
			} else {
				Widget_GameText_drawMultiline(53, 52,
					baseOffsetX + 72, baseOffsetY + 374, 496, FONT_NORMAL_WHITE);
			}
			break;
		case 4:
			Widget_GameText_draw(53, 4,
				baseOffsetX + 72, baseOffsetY + 359, FONT_NORMAL_WHITE);
			if (Data_CityInfo.ratingFavor <= 90) {
				Widget_GameText_drawMultiline(53, 27 + Data_CityInfo.ratingAdvisorExplanationFavor,
					baseOffsetX + 72, baseOffsetY + 374, 496, FONT_NORMAL_WHITE);
			} else {
				Widget_GameText_drawMultiline(53, 53,
					baseOffsetX + 72, baseOffsetY + 374, 496, FONT_NORMAL_WHITE);
			}
			break;
		default:
			Widget_GameText_drawCentered(53, 8,
				baseOffsetX + 72, baseOffsetY + 380, 496, FONT_NORMAL_WHITE);
			break;
	}
}

static void drawRatingColumn(int xOffset, int yOffset, int value, int hasReached)
{
	int graphicBase = image_group(GROUP_RATINGS_COLUMN);
	int y = yOffset - image_get(graphicBase)->height;
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
    graphics_in_dialog();

	// culture
	button_border_draw(80, 286, 110, 66, focusButtonId == 1);
	// prosperity
	button_border_draw(200, 286, 110, 66, focusButtonId == 2);
	// peace
	button_border_draw(320, 286, 110, 66, focusButtonId == 3);
	// favor
	button_border_draw(440, 286, 110, 66, focusButtonId == 4);

    graphics_reset_dialog();
}

void UI_Advisor_Ratings_handleMouse(const mouse *m)
{
	generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, ratingButtons, 4, &focusButtonId);
}

static void buttonRating(int param1, int param2)
{
	Data_CityInfo.ratingAdvisorSelection = param1;
	UI_Window_requestRefresh();
}

int UI_Advisor_Ratings_getTooltip()
{
	switch (focusButtonId) {
		case 1: return 102;
		case 2: return 103;
		case 3: return 104;
		case 4: return 105;
		default: return 0;
	}
}
