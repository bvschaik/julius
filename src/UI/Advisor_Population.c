#include "Advisors_private.h"
#include "Window.h"

#include "game/time.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "scenario/property.h"

static void drawHistoryGraph(int fullSize, int x, int y);
static void drawCensusGraph(int fullSize, int x, int y);
static void drawSocietyGraph(int fullSize, int x, int y);
static void getYAxis(int maxValue, int *yMax, int *yShift);
static int getPopulationAtMonth(int max, int month);
static void getMinMaxMonthYear(int maxMonths, int *startMonth, int *startYear, int *endMonth, int *endYear);
static void buttonGraph(int param1, int param2);

static generic_button graphButtons[] = {
	{503,  61, 607, 116, GB_IMMEDIATE, buttonGraph, button_none, 0, 0},
	{503, 161, 607, 216, GB_IMMEDIATE, buttonGraph, button_none, 1, 0},
};

static int focusButtonId;

void UI_Advisor_Population_drawBackground(int *advisorHeight)
{
    graphics_in_dialog();

	*advisorHeight = 27;
	outer_panel_draw(0, 0, 40, *advisorHeight);
	image_draw(image_group(GROUP_ADVISOR_ICONS) + 5, 10, 10);

	// Title: depends on big graph shown
	if (Data_CityInfo_Extra.populationGraphOrder < 2) {
		lang_text_draw(55, 0, 60, 12, FONT_LARGE_BLACK);
	} else if (Data_CityInfo_Extra.populationGraphOrder < 4) {
		lang_text_draw(55, 1, 60, 12, FONT_LARGE_BLACK);
	} else {
		lang_text_draw(55, 2, 60, 12, FONT_LARGE_BLACK);
	}

	image_draw(image_group(GROUP_PANEL_WINDOWS) + 14, 56, 60);
	
	int bigText, topText, botText;
	void (*bigGraph)(int, int, int);
	void (*topGraph)(int, int, int);
	void (*botGraph)(int, int, int);
	switch (Data_CityInfo_Extra.populationGraphOrder) {
		default:
		case 0:
			bigText = 6;
			topText = 4;
			botText = 5;
			bigGraph = drawHistoryGraph;
			topGraph = drawCensusGraph;
			botGraph = drawSocietyGraph;
			break;
		case 1:
			bigText = 6;
			topText = 5;
			botText = 4;
			bigGraph = drawHistoryGraph;
			topGraph = drawSocietyGraph;
			botGraph = drawCensusGraph;
			break;
		case 2:
			bigText = 7;
			topText = 3;
			botText = 5;
			bigGraph = drawCensusGraph;
			topGraph = drawHistoryGraph;
			botGraph = drawSocietyGraph;
			break;
		case 3:
			bigText = 7;
			topText = 5;
			botText = 3;
			bigGraph = drawCensusGraph;
			topGraph = drawSocietyGraph;
			botGraph = drawHistoryGraph;
			break;
		case 4:
			bigText = 8;
			topText = 3;
			botText = 4;
			bigGraph = drawSocietyGraph;
			topGraph = drawHistoryGraph;
			botGraph = drawCensusGraph;
			break;
		case 5:
			bigText = 8;
			topText = 4;
			botText = 3;
			bigGraph = drawSocietyGraph;
			topGraph = drawCensusGraph;
			botGraph = drawHistoryGraph;
			break;
	}
	lang_text_draw_centered(55, bigText, 60, 295, 400, FONT_NORMAL_BLACK);
	lang_text_draw_centered(55, topText, 504, 120, 100, FONT_NORMAL_BLACK);
	lang_text_draw_centered(55, botText, 504, 220, 100, FONT_NORMAL_BLACK);
	bigGraph(1, 64, 64);
	topGraph(0, 505, 63);
	botGraph(0, 505, 163);

	// food/migration info panel
	inner_panel_draw(48, 336, 34, 5);
	int graphicId = image_group(GROUP_BULLET);
	int width;
	image_draw(graphicId, 56, 344);
	image_draw(graphicId, 56, 362);
	image_draw(graphicId, 56, 380);
	image_draw(graphicId, 56, 398);

	// food stores
	if (scenario_property_rome_supplies_wheat()) {
		lang_text_draw(55, 11, 75, 342, FONT_NORMAL_WHITE);
	} else {
		width = lang_text_draw_amount(8, 6, Data_CityInfo.foodInfoGranariesOperating, 75, 342, FONT_NORMAL_WHITE);
		if (Data_CityInfo.foodInfoFoodSupplyMonths > 0) {
			width += lang_text_draw(55, 12, 75 + width, 342, FONT_NORMAL_WHITE);
			lang_text_draw_amount(8, 4, Data_CityInfo.foodInfoFoodSupplyMonths, 75 + width, 342, FONT_NORMAL_WHITE);
		} else if (Data_CityInfo.foodInfoFoodStoredInGranaries > Data_CityInfo.foodInfoFoodNeededPerMonth / 2) {
			lang_text_draw(55, 13, 75 + width, 342, FONT_NORMAL_WHITE);
		} else if (Data_CityInfo.foodInfoFoodStoredInGranaries > 0) {
			lang_text_draw(55, 15, 75 + width, 342, FONT_NORMAL_WHITE);
		} else {
			lang_text_draw(55, 14, 75 + width, 342, FONT_NORMAL_WHITE);
		}
	}

	// food types eaten
	width = lang_text_draw(55, 16, 75, 360, FONT_NORMAL_WHITE);
	text_draw_number(Data_CityInfo.foodInfoFoodTypesAvailable, '@', " ", 75 + width, 360, FONT_NORMAL_WHITE);

	// immigration
	if (Data_CityInfo.populationNewcomersThisMonth >= 5) {
		lang_text_draw(55, 24, 75, 378, FONT_NORMAL_WHITE);
		width = text_draw_number(Data_CityInfo.populationNewcomersThisMonth, '@', " ", 75, 396, FONT_NORMAL_WHITE);
		lang_text_draw(55, 17, 75 + width, 396, FONT_NORMAL_WHITE);
	} else if (Data_CityInfo.populationRefusedImmigrantsNoRoom || Data_CityInfo.populationRoomInHouses <= 0) {
		lang_text_draw(55, 24, 75, 378, FONT_NORMAL_WHITE);
		lang_text_draw(55, 19, 75, 396, FONT_NORMAL_WHITE);
	} else if (Data_CityInfo.populationMigrationPercentage < 80) {
		lang_text_draw(55, 25, 75, 378, FONT_NORMAL_WHITE);
		int textId;
		switch (Data_CityInfo.populationEmigrationCauseTextId) {
			case 0: textId = 20; break;
			case 1: textId = 21; break;
			case 2: textId = 22; break;
			case 3: textId = 23; break;
			case 4: textId = 31; break;
			case 5: textId = 32; break;
			default: textId = 0; break;
		}
		if (textId) {
			lang_text_draw(55, textId, 75, 396, FONT_NORMAL_WHITE);
		}
	} else {
		lang_text_draw(55, 24, 75, 378, FONT_NORMAL_WHITE);
		width = text_draw_number(Data_CityInfo.populationNewcomersThisMonth, '@', " ", 75, 396, FONT_NORMAL_WHITE);
		if (Data_CityInfo.populationNewcomersThisMonth == 1) {
			lang_text_draw(55, 18, 75 + width, 396, FONT_NORMAL_WHITE);
		} else {
			lang_text_draw(55, 17, 75 + width, 396, FONT_NORMAL_WHITE);
		}
	}
	graphics_reset_dialog();
}

void UI_Advisor_Population_drawForeground()
{
    graphics_in_dialog();

	if (focusButtonId == 0) {
		button_border_draw(501, 60, 106, 57, 0);
		button_border_draw(501, 160, 106, 57, 0);
	} else if (focusButtonId == 1) {
		button_border_draw(501, 60, 106, 57, 1);
		button_border_draw(501, 160, 106, 57, 0);
	} else if (focusButtonId == 2) {
		button_border_draw(501, 60, 106, 57, 0);
		button_border_draw(501, 160, 106, 57, 1);
	}
	graphics_reset_dialog();
}

void UI_Advisor_Population_handleMouse(const mouse *m)
{
	generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, graphButtons, 2, &focusButtonId);
}

static void drawHistoryGraph(int fullSize, int x, int y)
{
	int maxMonths;
	if (Data_CityInfo.monthsSinceStart <= 20) {
		maxMonths = 20;
	} else if (Data_CityInfo.monthsSinceStart <= 40) {
		maxMonths = 40;
	} else if (Data_CityInfo.monthsSinceStart <= 100) {
		maxMonths = 100;
	} else if (Data_CityInfo.monthsSinceStart <= 200) {
		maxMonths = 200;
	} else {
		maxMonths = 400;
	}
	if (!fullSize) {
		if (maxMonths <= 40) {
			maxMonths = 20;
		} else {
			maxMonths = 100;
		}
	}
	// determine max value
	int maxValue = 0;
	for (int m = 0; m < maxMonths; m++) {
		int value = getPopulationAtMonth(maxMonths, m);
		if (value > maxValue) {
			maxValue = value;
		}
	}
	int yMax, yShift;
	getYAxis(maxValue, &yMax, &yShift);
	if (fullSize) {
		// y axis
		text_draw_number_centered(yMax, x - 66, y - 3, 60, FONT_SMALL_PLAIN);
		text_draw_number_centered(yMax / 2, x - 66, y + 96, 60, FONT_SMALL_PLAIN);
		text_draw_number_centered(0, x - 66, y + 196, 60, FONT_SMALL_PLAIN);
		// x axis
		int startMonth, startYear, endMonth, endYear;
		getMinMaxMonthYear(maxMonths, &startMonth, &startYear, &endMonth, &endYear);

		int width = lang_text_draw(25, startMonth, x - 20, y + 210, FONT_SMALL_PLAIN);
		lang_text_draw_year(startYear, x + width - 20, y + 210, FONT_SMALL_PLAIN);

		width = lang_text_draw(25, endMonth, x + 380, y + 210, FONT_SMALL_PLAIN);
		lang_text_draw_year(startYear, x + width + 380, y + 210, FONT_SMALL_PLAIN);
	}

	if (fullSize) {
		graphics_set_clip_rectangle(0, 0, 640, y + 200);
		for (int m = 0; m < maxMonths; m++) {
			int pop = getPopulationAtMonth(maxMonths, m);
			int val;
			if (yShift == -1) {
				val = 2 * pop;
			} else {
				val = pop >> yShift;
			}
			if (val > 0) {
				switch (maxMonths) {
					case 20:
						image_draw(image_group(GROUP_POPULATION_GRAPH_BAR), x + 20 * m, y + 200 - val);
						break;
					case 40:
						image_draw(image_group(GROUP_POPULATION_GRAPH_BAR) + 1, x + 10 * m, y + 200 - val);
						break;
					case 100:
						image_draw(image_group(GROUP_POPULATION_GRAPH_BAR) + 2, x + 4 * m, y + 200 - val);
						break;
					case 200:
						image_draw(image_group(GROUP_POPULATION_GRAPH_BAR) + 3, x + 2 * m, y + 200 - val);
						break;
					default:
						graphics_draw_line(x + m, y + 200 - val, x + m, y + 199, COLOR_RED);
						break;
				}
			}
		}
		graphics_reset_clip_rectangle();
	} else {
		yShift += 2;
		for (int m = 0; m < maxMonths; m++) {
			int val = getPopulationAtMonth(maxMonths, m) >> yShift;
			if (val > 0) {
				if (maxMonths == 20) {
					graphics_fill_rect(x + m, y + 50 - val, 4, val + 1, COLOR_RED);
				} else {
					graphics_draw_line(x + m, y + 50 - val, x + m, y + 50, COLOR_RED);
				}
			}
		}
	}
}

static void drawCensusGraph(int fullSize, int x, int y)
{
	int maxValue = 0;
	for (int i = 0; i < 100; i++) {
		if (Data_CityInfo.populationPerAge[i] > maxValue) {
			maxValue = Data_CityInfo.populationPerAge[i];
		}
	}
	int yMax, yShift;
	getYAxis(maxValue, &yMax, &yShift);
	if (fullSize) {
		// y axis
		text_draw_number_centered(yMax, x - 66, y - 3, 60, FONT_SMALL_PLAIN);
		text_draw_number_centered(yMax / 2, x - 66, y + 96, 60, FONT_SMALL_PLAIN);
		text_draw_number_centered(0, x - 66, y + 196, 60, FONT_SMALL_PLAIN);
		// x axis
		for (int i = 0; i <= 10; i++) {
			text_draw_number_centered(i * 10, x + 40 * i - 22, y + 210, 40, FONT_SMALL_PLAIN);
		}
	}

	if (fullSize) {
		graphics_set_clip_rectangle(0, 0, 640, y + 200);
		for (int i = 0; i < 100; i++) {
			int pop = Data_CityInfo.populationPerAge[i];
			int val;
			if (yShift == -1) {
				val = 2 * pop;
			} else {
				val = pop >> yShift;
			}
			if (val > 0) {
				image_draw(image_group(GROUP_POPULATION_GRAPH_BAR) + 2, x + 4 * i, y + 200 - val);
			}
		}
		graphics_reset_clip_rectangle();
	} else {
		yShift += 2;
		for (int i = 0; i < 100; i++) {
			int val = Data_CityInfo.populationPerAge[i] >> yShift;
			if (val > 0) {
				graphics_draw_line(x + i, y + 50 - val, x + i, y + 50, COLOR_RED);
			}
		}
	}
}

static void drawSocietyGraph(int fullSize, int x, int y)
{
	int maxValue = 0;
	for (int i = 0; i < 20; i++) {
		if (Data_CityInfo.populationPerLevel[i] > maxValue) {
			maxValue = Data_CityInfo.populationPerLevel[i];
		}
	}
	int yMax, yShift;
	getYAxis(maxValue, &yMax, &yShift);
	if (fullSize) {
		// y axis
		text_draw_number_centered(yMax, x - 66, y - 3, 60, FONT_SMALL_PLAIN);
		text_draw_number_centered(yMax / 2, x - 66, y + 96, 60, FONT_SMALL_PLAIN);
		text_draw_number_centered(0, x - 66, y + 196, 60, FONT_SMALL_PLAIN);
		// x axis
		lang_text_draw_centered(55, 9, x - 80, y + 210, 200, FONT_SMALL_PLAIN);
		lang_text_draw_centered(55, 10, x + 280, y + 210, 200, FONT_SMALL_PLAIN);
	}

	if (fullSize) {
		graphics_set_clip_rectangle(0, 0, 640, y + 200);
		for (int i = 0; i < 20; i++) {
			int pop = Data_CityInfo.populationPerLevel[i];
			int val;
			if (yShift == -1) {
				val = 2 * pop;
			} else {
				val = pop >> yShift;
			}
			if (val > 0) {
				image_draw(image_group(GROUP_POPULATION_GRAPH_BAR), x + 20 * i, y + 200 - val);
			}
		}
		graphics_reset_clip_rectangle();
	} else {
		yShift += 2;
		for (int i = 0; i < 20; i++) {
			int val = Data_CityInfo.populationPerLevel[i] >> yShift;
			if (val > 0) {
				graphics_fill_rect(x + 5 * i, y + 50 - val, 4, val + 1, COLOR_RED);
			}
		}
	}
}

static void getYAxis(int maxValue, int *yMax, int *yShift)
{
	if (maxValue <= 100) {
		*yMax = 100;
		*yShift = -1;
	} else if (maxValue <= 200) {
		*yMax = 200;
		*yShift = 0;
	} else if (maxValue <= 400) {
		*yMax = 400;
		*yShift = 1;
	} else if (maxValue <= 800) {
		*yMax = 800;
		*yShift = 2;
	} else if (maxValue <= 1600) {
		*yMax = 1600;
		*yShift = 3;
	} else if (maxValue <= 3200) {
		*yMax = 3200;
		*yShift = 4;
	} else if (maxValue <= 6400) {
		*yMax = 6400;
		*yShift = 5;
	} else if (maxValue <= 12800) {
		*yMax = 12800;
		*yShift = 6;
	} else if (maxValue <= 25600) {
		*yMax = 25600;
		*yShift = 7;
	} else {
		*yMax = 51200;
		*yShift = 8;
	}
}

static int getPopulationAtMonth(int max, int month)
{
	int startOffset = 0;
	if (Data_CityInfo.monthsSinceStart > max) {
		startOffset = Data_CityInfo.monthsSinceStart + 2400 - max;
	}
	int index = (startOffset + month) % 2400;
	return Data_CityInfo.monthlyPopulation[index];
}

static void getMinMaxMonthYear(int maxMonths, int *startMonth, int *startYear, int *endMonth, int *endYear)
{
	if (Data_CityInfo.monthsSinceStart > maxMonths) {
		*endMonth = game_time_month() - 1;
		*endYear = game_time_year();
		if (*endMonth < 0) {
			*endMonth += 12;
			*endYear -= 1;
		}
		*startMonth = 11 - (maxMonths % 12);
		*startYear = *endYear - maxMonths / 12;
	} else {
		*startMonth = 0;
		*startYear = scenario_property_start_year();
		*endMonth = (maxMonths + *startMonth) % 12;
		*endYear = (maxMonths + *startMonth) / 12 + *startYear;
	}
}

static void buttonGraph(int param1, int param2)
{
	int newOrder;
	switch (Data_CityInfo_Extra.populationGraphOrder) {
		default:
		case 0:
			newOrder = param1 ? 5 : 2;
			break;
		case 1:
			newOrder = param1 ? 3 : 4;
			break;
		case 2:
			newOrder = param1 ? 4 : 0;
			break;
		case 3:
			newOrder = param1 ? 1 : 5;
			break;
		case 4:
			newOrder = param1 ? 2 : 1;
			break;
		case 5:
			newOrder = param1 ? 0 : 3;
			break;
	}
	Data_CityInfo_Extra.populationGraphOrder = newOrder;
	UI_Window_requestRefresh();
}

int UI_Advisor_Population_getTooltip()
{
	if (focusButtonId) {
		return 111;
	} else {
		return 0;
	}
}
