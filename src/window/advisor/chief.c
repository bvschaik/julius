#include "chief.h"

#include "city/finance.h"
#include "core/calc.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "scenario/invasion.h"
#include "scenario/property.h"

#include "Data/CityInfo.h"

#define ADVISOR_HEIGHT 23
#define X_OFFSET 240

static void draw_title(int y, int text_id)
{
    image_draw(image_group(GROUP_BULLET), 40, y + 1);
    lang_text_draw(61, text_id, 60, y, FONT_NORMAL_WHITE);
}

static int draw_background()
{
    int width;

    outer_panel_draw(0, 0, 40, ADVISOR_HEIGHT);
    image_draw(image_group(GROUP_ADVISOR_ICONS) + 11, 10, 10);

    lang_text_draw(61, 0, 60, 12, FONT_LARGE_BLACK);
    inner_panel_draw(32, 60, 36, 16);

    // workers
    draw_title(66, 1);
    if (Data_CityInfo.unemploymentPercentage > 0) {
        width = lang_text_draw(61, 12, X_OFFSET, 66, FONT_NORMAL_RED);
        text_draw_percentage(Data_CityInfo.unemploymentPercentage, X_OFFSET + width, 66, FONT_NORMAL_RED);
    } else if (Data_CityInfo.workersNeeded > 0) {
        width = lang_text_draw(61, 13, X_OFFSET, 66, FONT_NORMAL_RED);
        lang_text_draw_amount(8, 12, Data_CityInfo.workersNeeded, X_OFFSET + width, 66, FONT_NORMAL_RED);
    } else {
        lang_text_draw(61, 14, X_OFFSET, 66, FONT_NORMAL_GREEN);
    }

    // finance
    draw_title(86, 2);
    int treasury = city_finance_treasury();
    if (treasury > Data_CityInfo.financeBalanceLastYear) {
        width = lang_text_draw(61, 15, X_OFFSET, 86, FONT_NORMAL_GREEN);
        text_draw_money(treasury - Data_CityInfo.financeBalanceLastYear, X_OFFSET + width, 86, FONT_NORMAL_GREEN);
    } else if (treasury < Data_CityInfo.financeBalanceLastYear) {
        width = lang_text_draw(61, 16, X_OFFSET, 86, FONT_NORMAL_RED);
        text_draw_money(Data_CityInfo.financeBalanceLastYear - treasury, X_OFFSET + width, 86, FONT_NORMAL_RED);
    } else {
        lang_text_draw(61, 17, X_OFFSET, 86, FONT_NORMAL_GREEN);
    }

    // migration
    draw_title(106, 3);
    if (Data_CityInfo.numEnemiesInCity + Data_CityInfo.numImperialSoldiersInCity > 3) {
        lang_text_draw(61, 79, X_OFFSET, 106, FONT_NORMAL_GREEN);
    } else if (Data_CityInfo.populationNewcomersThisMonth >= 5) {
        lang_text_draw(61, 25, X_OFFSET, 106, FONT_NORMAL_GREEN);
    } else if (Data_CityInfo.populationRefusedImmigrantsNoRoom || Data_CityInfo.populationRoomInHouses <= 0) {
        lang_text_draw(61, 18, X_OFFSET, 106, FONT_NORMAL_RED);
    } else if (Data_CityInfo.populationMigrationPercentage >= 80) {
        lang_text_draw(61, 25, X_OFFSET, 106, FONT_NORMAL_GREEN);
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
            lang_text_draw(61, textId, X_OFFSET, 106, FONT_NORMAL_GREEN);
        }
    }

    // food stocks
    draw_title(126, 4);
    if (scenario_property_rome_supplies_wheat()) {
        lang_text_draw(61, 26, X_OFFSET, 126, FONT_NORMAL_GREEN);
    } else if (Data_CityInfo.foodInfoFoodSupplyMonths > 0) {
        width = lang_text_draw(61, 28, X_OFFSET, 126, FONT_NORMAL_GREEN);
        lang_text_draw_amount(8, 4, Data_CityInfo.foodInfoFoodSupplyMonths, X_OFFSET + width, 126, FONT_NORMAL_GREEN);
    } else {
        lang_text_draw(61, 27, X_OFFSET, 126, FONT_NORMAL_RED);
    }

    // food consumption
    draw_title(146, 62);
    if (scenario_property_rome_supplies_wheat()) {
        lang_text_draw(61, 26, X_OFFSET, 146, FONT_NORMAL_GREEN);
    } else {
        int pct = calc_percentage(Data_CityInfo.foodInfoFoodStoredLastMonth, Data_CityInfo.foodInfoFoodConsumedLastMonth);
        if (pct > 150) {
            lang_text_draw(61, 63, X_OFFSET, 146, FONT_NORMAL_GREEN);
        } else if (pct > 105) {
            lang_text_draw(61, 64, X_OFFSET, 146, FONT_NORMAL_GREEN);
        } else if (pct > 95) {
            lang_text_draw(61, 65, X_OFFSET, 146, FONT_NORMAL_GREEN);
        } else if (pct > 75) {
            lang_text_draw(61, 66, X_OFFSET, 146, FONT_NORMAL_RED);
        } else if (pct > 30) {
            lang_text_draw(61, 67, X_OFFSET, 146, FONT_NORMAL_RED);
        } else if (pct > 0) {
            lang_text_draw(61, 68, X_OFFSET, 146, FONT_NORMAL_RED);
        } else {
            lang_text_draw(61, 69, X_OFFSET, 146, FONT_NORMAL_RED);
        }
    }

    // military
    draw_title(166, 5);
    if (Data_CityInfo.numImperialSoldiersInCity) {
        lang_text_draw(61, 76, X_OFFSET, 166, FONT_NORMAL_RED);
    } else if (Data_CityInfo.numEnemiesInCity) {
        lang_text_draw(61, 75, X_OFFSET, 166, FONT_NORMAL_RED);
    } else if (scenario_invasion_exists_upcoming()) {
        lang_text_draw(61, 74, X_OFFSET, 166, FONT_NORMAL_RED);
    } else if (Data_CityInfo.distantBattleRomanMonthsToReturn > 0 ||
            Data_CityInfo.distantBattleRomanMonthsToTravel > 0) {
        lang_text_draw(61, 78, X_OFFSET, 166, FONT_NORMAL_GREEN);
    } else if (Data_CityInfo.distantBattleMonthsToBattle > 0) {
        lang_text_draw(61, 77, X_OFFSET, 166, FONT_NORMAL_RED);
    } else if (Data_CityInfo.numSoldiersInCity > 0) { // FIXED was ">=0" (always true)
        lang_text_draw(61, 73, X_OFFSET, 166, FONT_NORMAL_GREEN);
    } else {
        lang_text_draw(61, 72, X_OFFSET, 166, FONT_NORMAL_GREEN);
    }

    // crime
    draw_title(186, 6);
    if (Data_CityInfo.numRiotersInCity) {
        lang_text_draw(61, 33, X_OFFSET, 186, FONT_NORMAL_RED);
    } else if (Data_CityInfo.numCriminalsThisMonth > 10) {
        lang_text_draw(61, 32, X_OFFSET, 186, FONT_NORMAL_RED);
    } else if (Data_CityInfo.numCriminalsThisMonth) {
        lang_text_draw(61, 31, X_OFFSET, 186, FONT_NORMAL_RED);
    } else if (Data_CityInfo.numProtestersThisMonth) {
        lang_text_draw(61, 30, X_OFFSET, 186, FONT_NORMAL_RED);
    } else {
        lang_text_draw(61, 29, X_OFFSET, 186, FONT_NORMAL_GREEN);
    }

    // health
    draw_title(206, 7);
    if (Data_CityInfo.healthRate >= 40) {
        lang_text_draw(56, Data_CityInfo.healthRate / 10 + 27, X_OFFSET, 206, FONT_NORMAL_GREEN);
    } else {
        lang_text_draw(56, Data_CityInfo.healthRate / 10 + 27, X_OFFSET, 206, FONT_NORMAL_RED);
    }

    // education
    draw_title(226, 8);
    if (Data_CityInfo.educationDemand == 1) {
        lang_text_draw(61, 39, X_OFFSET, 226, FONT_NORMAL_RED);
    } else if (Data_CityInfo.educationDemand == 2) {
        lang_text_draw(61, 40, X_OFFSET, 226, FONT_NORMAL_RED);
    } else if (Data_CityInfo.educationDemand == 3) {
        lang_text_draw(61, 41, X_OFFSET, 226, FONT_NORMAL_RED);
    } else {
        lang_text_draw(61, 42, X_OFFSET, 226, FONT_NORMAL_GREEN);
    }

    // religion
    draw_title(246, 7);
    if (Data_CityInfo.religionDemand == 1) {
        lang_text_draw(61, 46, X_OFFSET, 246, FONT_NORMAL_RED);
    } else if (Data_CityInfo.religionDemand == 2) {
        lang_text_draw(61, 47, X_OFFSET, 246, FONT_NORMAL_RED);
    } else if (Data_CityInfo.religionDemand == 3) {
        lang_text_draw(61, 48, X_OFFSET, 246, FONT_NORMAL_RED);
    } else {
        lang_text_draw(61, 49, X_OFFSET, 246, FONT_NORMAL_GREEN);
    }

    // entertainment
    draw_title(266, 10);
    if (Data_CityInfo.entertainmentDemand == 1) {
        lang_text_draw(61, 43, X_OFFSET, 266, FONT_NORMAL_RED);
    } else if (Data_CityInfo.entertainmentDemand == 2) {
        lang_text_draw(61, 44, X_OFFSET, 266, FONT_NORMAL_RED);
    } else {
        lang_text_draw(61, 45, X_OFFSET, 266, FONT_NORMAL_GREEN);
    }

    // sentiment
    draw_title(286, 11);
    if (Data_CityInfo.citySentiment <= 0) {
        lang_text_draw(61, 50, X_OFFSET, 286, FONT_NORMAL_RED);
    } else if (Data_CityInfo.citySentiment >= 100) {
        lang_text_draw(61, 61, X_OFFSET, 286, FONT_NORMAL_GREEN);
    } else {
        lang_text_draw(61, Data_CityInfo.citySentiment / 10 + 51, X_OFFSET, 286, FONT_NORMAL_GREEN);
    }
    
    return ADVISOR_HEIGHT;
}

const advisor_window_type *window_advisor_chief()
{
    static const advisor_window_type window = {
        draw_background,
        0,
        0,
        0
    };
    return &window;
}
