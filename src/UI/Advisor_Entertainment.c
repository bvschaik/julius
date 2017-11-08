#include "Window.h"
#include "Advisors_private.h"
#include "../CityInfo.h"

#include "building/count.h"
#include "city/culture.h"

static void drawFestivalBackground();

static void buttonHoldFestival(int param1, int param2);

static CustomButton holdFestivalButtons[] = {
	{102, 280, 402, 300, CustomButton_Immediate, buttonHoldFestival, Widget_Button_doNothing, 0, 0},
};

static int focusButtonId;

void UI_Advisor_Entertainment_init()
{
	focusButtonId = 0;
}

void UI_Advisor_Entertainment_drawBackground(int *advisorHeight)
{
	CityInfo_Gods_calculateMoods(0);
	CityInfo_Culture_calculateEntertainment();

	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	*advisorHeight = 23;
	Widget_Panel_drawOuterPanel(baseOffsetX, baseOffsetY, 40, *advisorHeight);
	Graphics_drawImage(image_group(ID_Graphic_AdvisorIcons) + 8,
		baseOffsetX + 10, baseOffsetY + 10);

	Widget_GameText_draw(58, 0, baseOffsetX + 60, baseOffsetY + 12, FONT_LARGE_BLACK);

	Widget_GameText_draw(58, 1, baseOffsetX + 180, baseOffsetY + 46, FONT_SMALL_PLAIN);
	Widget_GameText_draw(58, 2, baseOffsetX + 260, baseOffsetY + 46, FONT_SMALL_PLAIN);
	Widget_GameText_draw(58, 3, baseOffsetX + 340, baseOffsetY + 46, FONT_SMALL_PLAIN);
	Widget_GameText_draw(58, 4, baseOffsetX + 480, baseOffsetY + 46, FONT_SMALL_PLAIN);

	Widget_Panel_drawInnerPanel(baseOffsetX + 32, baseOffsetY + 60, 36, 5);

	// theaters
	Widget_GameText_drawNumberWithDescription(8, 34, building_count_total(BUILDING_THEATER),
		baseOffsetX + 40, baseOffsetY + 64, FONT_NORMAL_WHITE
	);
	Widget_Text_drawNumberCentered(building_count_active(BUILDING_THEATER),
		baseOffsetX + 150, baseOffsetY + 64, 100, FONT_NORMAL_WHITE
	);
	Widget_Text_drawNumberCentered(Data_CityInfo.entertainmentTheaterShows,
		baseOffsetX + 230, baseOffsetY + 64, 100, FONT_NORMAL_WHITE
	);
	int width = Widget_Text_drawNumber(500 * building_count_active(BUILDING_THEATER), '@', " ",
		baseOffsetX + 340, baseOffsetY + 64, FONT_NORMAL_WHITE
	);
	Widget_GameText_draw(58, 5, baseOffsetX + 340 + width, baseOffsetY + 64, FONT_NORMAL_WHITE);
	int pct_theater = city_culture_coverage_theater();
	if (pct_theater == 0) {
		Widget_GameText_drawCentered(57, 10, baseOffsetX + 470, baseOffsetY + 64, 100, FONT_NORMAL_WHITE);
	} else if (pct_theater < 100) {
		Widget_GameText_drawCentered(57, 11 + pct_theater / 10,
			baseOffsetX + 470, baseOffsetY + 64, 100, FONT_NORMAL_WHITE
		);
	} else {
		Widget_GameText_drawCentered(57, 21, baseOffsetX + 470, baseOffsetY + 64, 100, FONT_NORMAL_WHITE);
	}

	// amphitheaters
	Widget_GameText_drawNumberWithDescription(8, 36, building_count_total(BUILDING_AMPHITHEATER),
		baseOffsetX + 40, baseOffsetY + 84, FONT_NORMAL_WHITE
	);
	Widget_Text_drawNumberCentered(building_count_active(BUILDING_AMPHITHEATER),
		baseOffsetX + 150, baseOffsetY + 84, 100, FONT_NORMAL_WHITE
	);
	Widget_Text_drawNumberCentered(Data_CityInfo.entertainmentAmphitheaterShows,
		baseOffsetX + 230, baseOffsetY + 84, 100, FONT_NORMAL_WHITE
	);
	width = Widget_Text_drawNumber(800 * building_count_active(BUILDING_AMPHITHEATER), '@', " ",
		baseOffsetX + 340, baseOffsetY + 84, FONT_NORMAL_WHITE
	);
	Widget_GameText_draw(58, 5, baseOffsetX + 340 + width, baseOffsetY + 84, FONT_NORMAL_WHITE);
	int pct_amphitheater = city_culture_coverage_amphitheater();
	if (pct_amphitheater == 0) {
		Widget_GameText_drawCentered(57, 10, baseOffsetX + 470, baseOffsetY + 84, 100, FONT_NORMAL_WHITE);
	} else if (pct_amphitheater < 100) {
		Widget_GameText_drawCentered(57, 11 + pct_amphitheater / 10,
			baseOffsetX + 470, baseOffsetY + 84, 100, FONT_NORMAL_WHITE
		);
	} else {
		Widget_GameText_drawCentered(57, 21, baseOffsetX + 470, baseOffsetY + 84, 100, FONT_NORMAL_WHITE);
	}

	// colosseums
	Widget_GameText_drawNumberWithDescription(8, 38, building_count_total(BUILDING_COLOSSEUM),
		baseOffsetX + 40, baseOffsetY + 104, FONT_NORMAL_WHITE
	);
	Widget_Text_drawNumberCentered(building_count_active(BUILDING_COLOSSEUM),
		baseOffsetX + 150, baseOffsetY + 104, 100, FONT_NORMAL_WHITE
	);
	Widget_Text_drawNumberCentered(Data_CityInfo.entertainmentColosseumShows,
		baseOffsetX + 230, baseOffsetY + 104, 100, FONT_NORMAL_WHITE
	);
	width = Widget_Text_drawNumber(1500 * building_count_active(BUILDING_COLOSSEUM), '@', " ",
		baseOffsetX + 340, baseOffsetY + 104, FONT_NORMAL_WHITE
	);
	Widget_GameText_draw(58, 5, baseOffsetX + 340 + width, baseOffsetY + 104, FONT_NORMAL_WHITE);
	int pct_colosseum = city_culture_coverage_colosseum();
	if (pct_colosseum == 0) {
		Widget_GameText_drawCentered(57, 10, baseOffsetX + 470, baseOffsetY + 104, 100, FONT_NORMAL_WHITE);
	} else if (pct_colosseum < 100) {
		Widget_GameText_drawCentered(57, 11 + pct_colosseum / 10,
			baseOffsetX + 470, baseOffsetY + 104, 100, FONT_NORMAL_WHITE
		);
	} else {
		Widget_GameText_drawCentered(57, 21, baseOffsetX + 470, baseOffsetY + 104, 100, FONT_NORMAL_WHITE);
	}

	// hippodromes
	Widget_GameText_drawNumberWithDescription(8, 40, building_count_total(BUILDING_HIPPODROME),
		baseOffsetX + 40, baseOffsetY + 123, FONT_NORMAL_WHITE
	);
	Widget_Text_drawNumberCentered(building_count_active(BUILDING_HIPPODROME),
		baseOffsetX + 150, baseOffsetY + 123, 100, FONT_NORMAL_WHITE
	);
	Widget_Text_drawNumberCentered(Data_CityInfo.entertainmentHippodromeShows,
		baseOffsetX + 230, baseOffsetY + 123, 100, FONT_NORMAL_WHITE
	);
	Widget_GameText_draw(58, 6, baseOffsetX + 360, baseOffsetY + 123, FONT_NORMAL_WHITE);
	if (city_culture_coverage_hippodrome() == 0) {
		Widget_GameText_drawCentered(57, 10, baseOffsetX + 470, baseOffsetY + 123, 100, FONT_NORMAL_WHITE);
	} else {
		Widget_GameText_drawCentered(57, 21, baseOffsetX + 470, baseOffsetY + 123, 100, FONT_NORMAL_WHITE);
	}

	int adviceId;

	if (Data_CityInfo.housesRequiringEntertainmentToEvolve > Data_CityInfo.housesRequiringMoreEntertainmentToEvolve) {
		adviceId = 3;
	} else if (!Data_CityInfo.housesRequiringMoreEntertainmentToEvolve) {
		adviceId = Data_CityInfo.citywideAverageEntertainment ? 1 : 0;
	} else if (Data_CityInfo.entertainmentNeedingShowsMost) {
		adviceId = 3 + Data_CityInfo.entertainmentNeedingShowsMost;
	} else {
		adviceId = 2;
	}
	Widget_GameText_drawMultiline(58, 7 + adviceId,
		baseOffsetX + 60, baseOffsetY + 148, 512, FONT_NORMAL_BLACK);

	drawFestivalBackground();
}

static void drawFestivalBackground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	Widget_Panel_drawInnerPanel(baseOffsetX + 48, baseOffsetY + 252, 34, 6);
	Graphics_drawImage(image_group(ID_Graphic_PanelWindows) + 15,
		baseOffsetX + 460, baseOffsetY + 255);
	Widget_GameText_draw(58, 17, baseOffsetX + 52, baseOffsetY + 224, FONT_LARGE_BLACK);

	int width = Widget_GameText_drawNumberWithDescription(
		8, 4, Data_CityInfo.monthsSinceFestival,
		baseOffsetX + 112, baseOffsetY + 260, FONT_NORMAL_WHITE
	);
	Widget_GameText_draw(58, 15, baseOffsetX + 112 + width, baseOffsetY + 260, FONT_NORMAL_WHITE);
	if (Data_CityInfo.plannedFestivalSize) {
		Widget_GameText_drawCentered(58, 34,
			baseOffsetX + 102, baseOffsetY + 284,
			300, FONT_NORMAL_WHITE);
	} else {
		Widget_GameText_drawCentered(58, 16,
			baseOffsetX + 102, baseOffsetY + 284,
			300, FONT_NORMAL_WHITE);
	}
	int adviceId;
	if (Data_CityInfo.monthsSinceFestival <= 1) {
		adviceId = 0;
	} else if (Data_CityInfo.monthsSinceFestival <= 6) {
		adviceId = 1;
	} else if (Data_CityInfo.monthsSinceFestival <= 12) {
		adviceId = 2;
	} else if (Data_CityInfo.monthsSinceFestival <= 18) {
		adviceId = 3;
	} else if (Data_CityInfo.monthsSinceFestival <= 24) {
		adviceId = 4;
	} else if (Data_CityInfo.monthsSinceFestival <= 30) {
		adviceId = 5;
	} else {
		adviceId = 6;
	}
	Widget_GameText_drawMultiline(58, 18 + adviceId,
		baseOffsetX + 56, baseOffsetY + 305,
		380, FONT_NORMAL_WHITE);
}


void UI_Advisor_Entertainment_drawForeground()
{
	if (!Data_CityInfo.plannedFestivalSize) {
		Widget_Panel_drawButtonBorder(
			Data_Screen.offset640x480.x + 102, Data_Screen.offset640x480.y + 280,
			300, 20, focusButtonId == 1);
	}
}

void UI_Advisor_Entertainment_handleMouse()
{
	Widget_Button_handleCustomButtons(
		Data_Screen.offset640x480.x, Data_Screen.offset640x480.y,
		holdFestivalButtons, 1, &focusButtonId);
}

void buttonHoldFestival(int param1, int param2)
{
	if (!Data_CityInfo.plannedFestivalSize) {
		UI_Window_goTo(Window_HoldFestivalDialog);
	}
}

int UI_Advisor_Entertainment_getTooltip()
{
	if (focusButtonId) {
		return 112;
	} else {
		return 0;
	}
}
