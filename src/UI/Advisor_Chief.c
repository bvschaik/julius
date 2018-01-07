#include "Advisors_private.h"

#include "city/finance.h"
#include "core/calc.h"
#include "graphics/graphics.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "scenario/invasion.h"
#include "scenario/property.h"

void UI_Advisor_Chief_drawBackground(int *advisorHeight)
{
    graphics_in_dialog();
	int width;

	*advisorHeight = 23;
	outer_panel_draw(0, 0, 40, *advisorHeight);
	Graphics_drawImage(image_group(GROUP_ADVISOR_ICONS) + 11, 10, 10);

	Widget_GameText_draw(61, 0, 60, 12, FONT_LARGE_BLACK);
	inner_panel_draw(32, 60, 36, 16);

	// workers
	Graphics_drawImage(image_group(GROUP_BULLET), 40, 67);
	Widget_GameText_draw(61, 1, 60, 66, FONT_NORMAL_WHITE);
	if (Data_CityInfo.unemploymentPercentage > 0) {
		width = Widget_GameText_draw(61, 12, 240, 66, FONT_NORMAL_RED);
		Widget_Text_drawPercentage(Data_CityInfo.unemploymentPercentage,
			240 + width, 66, FONT_NORMAL_RED);
	} else if (Data_CityInfo.workersNeeded > 0) {
		width = Widget_GameText_draw(61, 13, 240, 66, FONT_NORMAL_RED);
		Widget_GameText_drawNumberWithDescription(8, 12, Data_CityInfo.workersNeeded,
			240 + width, 66, FONT_NORMAL_RED);
	} else {
		Widget_GameText_draw(61, 14, 240, 66, FONT_NORMAL_GREEN);
	}

	// finance
	Graphics_drawImage(image_group(GROUP_BULLET), 40, 87);
	Widget_GameText_draw(61, 2, 60, 86, FONT_NORMAL_WHITE);
    int treasury = city_finance_treasury();
	if (treasury > Data_CityInfo.financeBalanceLastYear) {
		width = Widget_GameText_draw(61, 15, 240, 86, FONT_NORMAL_GREEN);
		Widget_Text_drawMoney(treasury - Data_CityInfo.financeBalanceLastYear,
			240 + width, 86, FONT_NORMAL_GREEN);
	} else if (treasury < Data_CityInfo.financeBalanceLastYear) {
		width = Widget_GameText_draw(61, 16, 240, 86, FONT_NORMAL_RED);
		Widget_Text_drawMoney(Data_CityInfo.financeBalanceLastYear - treasury,
			240 + width, 86, FONT_NORMAL_RED);
	} else {
		Widget_GameText_draw(61, 17, 240, 86, FONT_NORMAL_GREEN);
	}

	// migration
	Graphics_drawImage(image_group(GROUP_BULLET), 40, 107);
	Widget_GameText_draw(61, 3, 60, 106, FONT_NORMAL_WHITE);
	if (Data_CityInfo.numEnemiesInCity + Data_CityInfo.numImperialSoldiersInCity > 3) {
		Widget_GameText_draw(61, 79, 240, 106, FONT_NORMAL_GREEN);
	} else if (Data_CityInfo.populationNewcomersThisMonth >= 5) {
		Widget_GameText_draw(61, 25, 240, 106, FONT_NORMAL_GREEN);
	} else if (Data_CityInfo.populationRefusedImmigrantsNoRoom || Data_CityInfo.populationRoomInHouses <= 0) {
		Widget_GameText_draw(61, 18, 240, 106, FONT_NORMAL_RED);
	} else if (Data_CityInfo.populationMigrationPercentage >= 80) {
		Widget_GameText_draw(61, 25, 240, 106, FONT_NORMAL_GREEN);
	} else {
		int textId;
		switch (Data_CityInfo.populationEmigrationCauseTextId) {
			case 0: textId = 19; break;
			case 1: textId = 20; break;
			case 2: textId = 21; break;
			case 3: textId = 22; break;
			case 4: textId = 70; break;
			case 5: textId = 71; break;
			default: textId = 0; break;
		}
		if (textId) {
			Widget_GameText_draw(61, textId, 240, 106, FONT_NORMAL_GREEN);
		}
	}

	// food stocks
	Graphics_drawImage(image_group(GROUP_BULLET), 40, 127);
	Widget_GameText_draw(61, 4, 60, 126, FONT_NORMAL_WHITE);
	if (scenario_property_rome_supplies_wheat()) {
		Widget_GameText_draw(61, 26, 240, 126, FONT_NORMAL_GREEN);
	} else if (Data_CityInfo.foodInfoFoodSupplyMonths > 0) {
		width = Widget_GameText_draw(61, 28, 240, 126, FONT_NORMAL_GREEN);
		Widget_GameText_drawNumberWithDescription(8, 4, Data_CityInfo.foodInfoFoodSupplyMonths,
			240 + width, 126, FONT_NORMAL_GREEN);
	} else {
		Widget_GameText_draw(61, 27, 240, 126, FONT_NORMAL_RED);
	}

	// food consumption
	Graphics_drawImage(image_group(GROUP_BULLET), 40, 147);
	Widget_GameText_draw(61, 62, 60, 146, FONT_NORMAL_WHITE);
	if (scenario_property_rome_supplies_wheat()) {
		Widget_GameText_draw(61, 26, 240, 146, FONT_NORMAL_GREEN);
	} else {
		int pct = calc_percentage(Data_CityInfo.foodInfoFoodStoredLastMonth, Data_CityInfo.foodInfoFoodConsumedLastMonth);
		if (pct > 150) {
			Widget_GameText_draw(61, 63, 240, 146, FONT_NORMAL_GREEN);
		} else if (pct > 105) {
			Widget_GameText_draw(61, 64, 240, 146, FONT_NORMAL_GREEN);
		} else if (pct > 95) {
			Widget_GameText_draw(61, 65, 240, 146, FONT_NORMAL_GREEN);
		} else if (pct > 75) {
			Widget_GameText_draw(61, 66, 240, 146, FONT_NORMAL_RED);
		} else if (pct > 30) {
			Widget_GameText_draw(61, 67, 240, 146, FONT_NORMAL_RED);
		} else if (pct > 0) {
			Widget_GameText_draw(61, 68, 240, 146, FONT_NORMAL_RED);
		} else {
			Widget_GameText_draw(61, 69, 240, 146, FONT_NORMAL_RED);
		}
	}

	// military
	Graphics_drawImage(image_group(GROUP_BULLET), 40, 167);
	Widget_GameText_draw(61, 5, 60, 166, FONT_NORMAL_WHITE);
	if (Data_CityInfo.numImperialSoldiersInCity) {
		Widget_GameText_draw(61, 76, 240, 166, FONT_NORMAL_RED);
	} else if (Data_CityInfo.numEnemiesInCity) {
		Widget_GameText_draw(61, 75, 240, 166, FONT_NORMAL_RED);
	} else if (scenario_invasion_exists_upcoming()) {
		Widget_GameText_draw(61, 74, 240, 166, FONT_NORMAL_RED);
	} else if (Data_CityInfo.distantBattleRomanMonthsToReturn > 0 ||
			Data_CityInfo.distantBattleRomanMonthsToTravel > 0) {
		Widget_GameText_draw(61, 78, 240, 166, FONT_NORMAL_GREEN);
	} else if (Data_CityInfo.distantBattleMonthsToBattle > 0) {
		Widget_GameText_draw(61, 77, 240, 166, FONT_NORMAL_RED);
	} else if (Data_CityInfo.numSoldiersInCity > 0) { // FIXED was ">=0" (always true)
		Widget_GameText_draw(61, 73, 240, 166, FONT_NORMAL_GREEN);
	} else {
		Widget_GameText_draw(61, 72, 240, 166, FONT_NORMAL_GREEN);
	}

	// crime
	Graphics_drawImage(image_group(GROUP_BULLET), 40, 187);
	Widget_GameText_draw(61, 6, 60, 186, FONT_NORMAL_WHITE);
	if (Data_CityInfo.numRiotersInCity) {
		Widget_GameText_draw(61, 33, 240, 186, FONT_NORMAL_RED);
	} else if (Data_CityInfo.numCriminalsThisMonth > 10) {
		Widget_GameText_draw(61, 32, 240, 186, FONT_NORMAL_RED);
	} else if (Data_CityInfo.numCriminalsThisMonth) {
		Widget_GameText_draw(61, 31, 240, 186, FONT_NORMAL_RED);
	} else if (Data_CityInfo.numProtestersThisMonth) {
		Widget_GameText_draw(61, 30, 240, 186, FONT_NORMAL_RED);
	} else {
		Widget_GameText_draw(61, 29, 240, 186, FONT_NORMAL_GREEN);
	}

	// health
	Graphics_drawImage(image_group(GROUP_BULLET), 40, 207);
	Widget_GameText_draw(61, 7, 60, 206, FONT_NORMAL_WHITE);
	if (Data_CityInfo.healthRate >= 40) {
		Widget_GameText_draw(56, Data_CityInfo.healthRate / 10 + 27,
			240, 206, FONT_NORMAL_GREEN);
	} else {
		Widget_GameText_draw(56, Data_CityInfo.healthRate / 10 + 27,
			240, 206, FONT_NORMAL_RED);
	}

	// education
	Graphics_drawImage(image_group(GROUP_BULLET), 40, 227);
	Widget_GameText_draw(61, 8, 60, 226, FONT_NORMAL_WHITE);
	if (Data_CityInfo.educationDemand == 1) {
		Widget_GameText_draw(61, 39, 240, 226, FONT_NORMAL_RED);
	} else if (Data_CityInfo.educationDemand == 2) {
		Widget_GameText_draw(61, 40, 240, 226, FONT_NORMAL_RED);
	} else if (Data_CityInfo.educationDemand == 3) {
		Widget_GameText_draw(61, 41, 240, 226, FONT_NORMAL_RED);
	} else {
		Widget_GameText_draw(61, 42, 240, 226, FONT_NORMAL_GREEN);
	}

	// religion
	Graphics_drawImage(image_group(GROUP_BULLET), 40, 247);
	Widget_GameText_draw(61, 9, 60, 246, FONT_NORMAL_WHITE);
	if (Data_CityInfo.religionDemand == 1) {
		Widget_GameText_draw(61, 46, 240, 246, FONT_NORMAL_RED);
	} else if (Data_CityInfo.religionDemand == 2) {
		Widget_GameText_draw(61, 47, 240, 246, FONT_NORMAL_RED);
	} else if (Data_CityInfo.religionDemand == 3) {
		Widget_GameText_draw(61, 48, 240, 246, FONT_NORMAL_RED);
	} else {
		Widget_GameText_draw(61, 49, 240, 246, FONT_NORMAL_GREEN);
	}

	// entertainment
	Graphics_drawImage(image_group(GROUP_BULLET), 40, 267);
	Widget_GameText_draw(61, 10, 60, 266, FONT_NORMAL_WHITE);
	if (Data_CityInfo.entertainmentDemand == 1) {
		Widget_GameText_draw(61, 43, 240, 266, FONT_NORMAL_RED);
	} else if (Data_CityInfo.entertainmentDemand == 2) {
		Widget_GameText_draw(61, 44, 240, 266, FONT_NORMAL_RED);
	} else {
		Widget_GameText_draw(61, 45, 240, 266, FONT_NORMAL_GREEN);
	}

	// sentiment
	Graphics_drawImage(image_group(GROUP_BULLET), 40, 287);
	Widget_GameText_draw(61, 11, 60, 286, FONT_NORMAL_WHITE);
	if (Data_CityInfo.citySentiment <= 0) {
		Widget_GameText_draw(61, 50, 240, 286, FONT_NORMAL_RED);
	} else if (Data_CityInfo.citySentiment >= 100) {
		Widget_GameText_draw(61, 61, 240, 286, FONT_NORMAL_GREEN);
	} else {
		Widget_GameText_draw(61, Data_CityInfo.citySentiment / 10 + 51,
			240, 286, FONT_NORMAL_GREEN);
	}
	graphics_reset_dialog();
}
