#include "Advisors_private.hpp"
#include "core/calc.hpp"
#include "../Event.hpp"
#include "../Data/Scenario.hpp"

void UI_Advisor_Chief_drawBackground(int *advisorHeight)
{
    int baseOffsetX = Data_Screen.offset640x480.x;
    int baseOffsetY = Data_Screen.offset640x480.y;
    int width;

    *advisorHeight = 23;
    Widget_Panel_drawOuterPanel(baseOffsetX, baseOffsetY, 40, *advisorHeight);
    Graphics_drawImage(image_group(ID_Graphic_AdvisorIcons) + 11, baseOffsetX + 10, baseOffsetY + 10);

    Widget_GameText_draw(61, 0, baseOffsetX + 60, baseOffsetY + 12, FONT_LARGE_BLACK);
    Widget_Panel_drawInnerPanel(baseOffsetX + 32, baseOffsetY + 60, 36, 16);

    // workers
    Graphics_drawImage(image_group(ID_Graphic_Bullet), baseOffsetX + 40, baseOffsetY + 67);
    Widget_GameText_draw(61, 1, baseOffsetX + 60, baseOffsetY + 66, FONT_NORMAL_WHITE);
    if (Data_CityInfo.unemploymentPercentage > 0)
    {
        width = Widget_GameText_draw(61, 12, baseOffsetX + 240, baseOffsetY + 66, FONT_NORMAL_RED);
        Widget_Text_drawPercentage(Data_CityInfo.unemploymentPercentage,
                                   baseOffsetX + 240 + width, baseOffsetY + 66, FONT_NORMAL_RED);
    }
    else if (Data_CityInfo.workersNeeded > 0)
    {
        width = Widget_GameText_draw(61, 13, baseOffsetX + 240, baseOffsetY + 66, FONT_NORMAL_RED);
        Widget_GameText_drawNumberWithDescription(8, 12, Data_CityInfo.workersNeeded,
                baseOffsetX + 240 + width, baseOffsetY + 66, FONT_NORMAL_RED);
    }
    else
    {
        Widget_GameText_draw(61, 14, baseOffsetX + 240, baseOffsetY + 66, FONT_NORMAL_GREEN);
    }

    // finance
    Graphics_drawImage(image_group(ID_Graphic_Bullet), baseOffsetX + 40, baseOffsetY + 87);
    Widget_GameText_draw(61, 2, baseOffsetX + 60, baseOffsetY + 86, FONT_NORMAL_WHITE);
    if (Data_CityInfo.treasury > Data_CityInfo.financeBalanceLastYear)
    {
        width = Widget_GameText_draw(61, 15, baseOffsetX + 240, baseOffsetY + 86, FONT_NORMAL_GREEN);
        Widget_Text_drawMoney(Data_CityInfo.treasury - Data_CityInfo.financeBalanceLastYear,
                              baseOffsetX + 240 + width, baseOffsetY + 86, FONT_NORMAL_GREEN);
    }
    else if (Data_CityInfo.treasury < Data_CityInfo.financeBalanceLastYear)
    {
        width = Widget_GameText_draw(61, 16, baseOffsetX + 240, baseOffsetY + 86, FONT_NORMAL_RED);
        Widget_Text_drawMoney(Data_CityInfo.financeBalanceLastYear - Data_CityInfo.treasury,
                              baseOffsetX + 240 + width, baseOffsetY + 86, FONT_NORMAL_RED);
    }
    else
    {
        Widget_GameText_draw(61, 17, baseOffsetX + 240, baseOffsetY + 86, FONT_NORMAL_GREEN);
    }

    // migration
    Graphics_drawImage(image_group(ID_Graphic_Bullet), baseOffsetX + 40, baseOffsetY + 107);
    Widget_GameText_draw(61, 3, baseOffsetX + 60, baseOffsetY + 106, FONT_NORMAL_WHITE);
    if (Data_CityInfo.numEnemiesInCity + Data_CityInfo.numImperialSoldiersInCity > 3)
    {
        Widget_GameText_draw(61, 79, baseOffsetX + 240, baseOffsetY + 106, FONT_NORMAL_GREEN);
    }
    else if (Data_CityInfo.populationNewcomersThisMonth >= 5)
    {
        Widget_GameText_draw(61, 25, baseOffsetX + 240, baseOffsetY + 106, FONT_NORMAL_GREEN);
    }
    else if (Data_CityInfo.populationRefusedImmigrantsNoRoom || Data_CityInfo.populationRoomInHouses <= 0)
    {
        Widget_GameText_draw(61, 18, baseOffsetX + 240, baseOffsetY + 106, FONT_NORMAL_RED);
    }
    else if (Data_CityInfo.populationMigrationPercentage >= 80)
    {
        Widget_GameText_draw(61, 25, baseOffsetX + 240, baseOffsetY + 106, FONT_NORMAL_GREEN);
    }
    else
    {
        int textId;
        switch (Data_CityInfo.populationEmigrationCauseTextId)
        {
        case 0:
            textId = 19;
            break;
        case 1:
            textId = 20;
            break;
        case 2:
            textId = 21;
            break;
        case 3:
            textId = 22;
            break;
        case 4:
            textId = 70;
            break;
        case 5:
            textId = 71;
            break;
        default:
            textId = 0;
            break;
        }
        if (textId)
        {
            Widget_GameText_draw(61, textId, baseOffsetX + 240, baseOffsetY + 106, FONT_NORMAL_GREEN);
        }
    }

    // food stocks
    Graphics_drawImage(image_group(ID_Graphic_Bullet), baseOffsetX + 40, baseOffsetY + 127);
    Widget_GameText_draw(61, 4, baseOffsetX + 60, baseOffsetY + 126, FONT_NORMAL_WHITE);
    if (Data_Scenario.romeSuppliesWheat)
    {
        Widget_GameText_draw(61, 26, baseOffsetX + 240, baseOffsetY + 126, FONT_NORMAL_GREEN);
    }
    else if (Data_CityInfo.foodInfoFoodSupplyMonths > 0)
    {
        width = Widget_GameText_draw(61, 28, baseOffsetX + 240, baseOffsetY + 126, FONT_NORMAL_GREEN);
        Widget_GameText_drawNumberWithDescription(8, 4, Data_CityInfo.foodInfoFoodSupplyMonths,
                baseOffsetX + 240 + width, baseOffsetY + 126, FONT_NORMAL_GREEN);
    }
    else
    {
        Widget_GameText_draw(61, 27, baseOffsetX + 240, baseOffsetY + 126, FONT_NORMAL_RED);
    }

    // food consumption
    Graphics_drawImage(image_group(ID_Graphic_Bullet), baseOffsetX + 40, baseOffsetY + 147);
    Widget_GameText_draw(61, 62, baseOffsetX + 60, baseOffsetY + 146, FONT_NORMAL_WHITE);
    if (Data_Scenario.romeSuppliesWheat)
    {
        Widget_GameText_draw(61, 26, baseOffsetX + 240, baseOffsetY + 146, FONT_NORMAL_GREEN);
    }
    else
    {
        int pct = calc_percentage(Data_CityInfo.foodInfoFoodStoredLastMonth, Data_CityInfo.foodInfoFoodConsumedLastMonth);
        if (pct > 150)
        {
            Widget_GameText_draw(61, 63, baseOffsetX + 240, baseOffsetY + 146, FONT_NORMAL_GREEN);
        }
        else if (pct > 105)
        {
            Widget_GameText_draw(61, 64, baseOffsetX + 240, baseOffsetY + 146, FONT_NORMAL_GREEN);
        }
        else if (pct > 95)
        {
            Widget_GameText_draw(61, 65, baseOffsetX + 240, baseOffsetY + 146, FONT_NORMAL_GREEN);
        }
        else if (pct > 75)
        {
            Widget_GameText_draw(61, 66, baseOffsetX + 240, baseOffsetY + 146, FONT_NORMAL_RED);
        }
        else if (pct > 30)
        {
            Widget_GameText_draw(61, 67, baseOffsetX + 240, baseOffsetY + 146, FONT_NORMAL_RED);
        }
        else if (pct > 0)
        {
            Widget_GameText_draw(61, 68, baseOffsetX + 240, baseOffsetY + 146, FONT_NORMAL_RED);
        }
        else
        {
            Widget_GameText_draw(61, 69, baseOffsetX + 240, baseOffsetY + 146, FONT_NORMAL_RED);
        }
    }

    // military
    Graphics_drawImage(image_group(ID_Graphic_Bullet), baseOffsetX + 40, baseOffsetY + 167);
    Widget_GameText_draw(61, 5, baseOffsetX + 60, baseOffsetY + 166, FONT_NORMAL_WHITE);
    if (Data_CityInfo.numImperialSoldiersInCity)
    {
        Widget_GameText_draw(61, 76, baseOffsetX + 240, baseOffsetY + 166, FONT_NORMAL_RED);
    }
    else if (Data_CityInfo.numEnemiesInCity)
    {
        Widget_GameText_draw(61, 75, baseOffsetX + 240, baseOffsetY + 166, FONT_NORMAL_RED);
    }
    else if (Event_existsUpcomingInvasion())
    {
        Widget_GameText_draw(61, 74, baseOffsetX + 240, baseOffsetY + 166, FONT_NORMAL_RED);
    }
    else if (Data_CityInfo.distantBattleRomanMonthsToReturn > 0 ||
             Data_CityInfo.distantBattleRomanMonthsToTravel > 0)
    {
        Widget_GameText_draw(61, 78, baseOffsetX + 240, baseOffsetY + 166, FONT_NORMAL_GREEN);
    }
    else if (Data_CityInfo.distantBattleMonthsToBattle > 0)
    {
        Widget_GameText_draw(61, 77, baseOffsetX + 240, baseOffsetY + 166, FONT_NORMAL_RED);
    }
    else if (Data_CityInfo.numSoldiersInCity > 0)     // FIXED was ">=0" (always true)
    {
        Widget_GameText_draw(61, 73, baseOffsetX + 240, baseOffsetY + 166, FONT_NORMAL_GREEN);
    }
    else
    {
        Widget_GameText_draw(61, 72, baseOffsetX + 240, baseOffsetY + 166, FONT_NORMAL_GREEN);
    }

    // crime
    Graphics_drawImage(image_group(ID_Graphic_Bullet), baseOffsetX + 40, baseOffsetY + 187);
    Widget_GameText_draw(61, 6, baseOffsetX + 60, baseOffsetY + 186, FONT_NORMAL_WHITE);
    if (Data_CityInfo.numRiotersInCity)
    {
        Widget_GameText_draw(61, 33, baseOffsetX + 240, baseOffsetY + 186, FONT_NORMAL_RED);
    }
    else if (Data_CityInfo.numCriminalsThisMonth > 10)
    {
        Widget_GameText_draw(61, 32, baseOffsetX + 240, baseOffsetY + 186, FONT_NORMAL_RED);
    }
    else if (Data_CityInfo.numCriminalsThisMonth)
    {
        Widget_GameText_draw(61, 31, baseOffsetX + 240, baseOffsetY + 186, FONT_NORMAL_RED);
    }
    else if (Data_CityInfo.numProtestersThisMonth)
    {
        Widget_GameText_draw(61, 30, baseOffsetX + 240, baseOffsetY + 186, FONT_NORMAL_RED);
    }
    else
    {
        Widget_GameText_draw(61, 29, baseOffsetX + 240, baseOffsetY + 186, FONT_NORMAL_GREEN);
    }

    // health
    Graphics_drawImage(image_group(ID_Graphic_Bullet), baseOffsetX + 40, baseOffsetY + 207);
    Widget_GameText_draw(61, 7, baseOffsetX + 60, baseOffsetY + 206, FONT_NORMAL_WHITE);
    if (Data_CityInfo.healthRate >= 40)
    {
        Widget_GameText_draw(56, Data_CityInfo.healthRate / 10 + 27,
                             baseOffsetX + 240, baseOffsetY + 206, FONT_NORMAL_GREEN);
    }
    else
    {
        Widget_GameText_draw(56, Data_CityInfo.healthRate / 10 + 27,
                             baseOffsetX + 240, baseOffsetY + 206, FONT_NORMAL_RED);
    }

    // education
    Graphics_drawImage(image_group(ID_Graphic_Bullet), baseOffsetX + 40, baseOffsetY + 227);
    Widget_GameText_draw(61, 8, baseOffsetX + 60, baseOffsetY + 226, FONT_NORMAL_WHITE);
    if (Data_CityInfo.educationDemand == 1)
    {
        Widget_GameText_draw(61, 39, baseOffsetX + 240, baseOffsetY + 226, FONT_NORMAL_RED);
    }
    else if (Data_CityInfo.educationDemand == 2)
    {
        Widget_GameText_draw(61, 40, baseOffsetX + 240, baseOffsetY + 226, FONT_NORMAL_RED);
    }
    else if (Data_CityInfo.educationDemand == 3)
    {
        Widget_GameText_draw(61, 41, baseOffsetX + 240, baseOffsetY + 226, FONT_NORMAL_RED);
    }
    else
    {
        Widget_GameText_draw(61, 42, baseOffsetX + 240, baseOffsetY + 226, FONT_NORMAL_GREEN);
    }

    // religion
    Graphics_drawImage(image_group(ID_Graphic_Bullet), baseOffsetX + 40, baseOffsetY + 247);
    Widget_GameText_draw(61, 9, baseOffsetX + 60, baseOffsetY + 246, FONT_NORMAL_WHITE);
    if (Data_CityInfo.religionDemand == 1)
    {
        Widget_GameText_draw(61, 46, baseOffsetX + 240, baseOffsetY + 246, FONT_NORMAL_RED);
    }
    else if (Data_CityInfo.religionDemand == 2)
    {
        Widget_GameText_draw(61, 47, baseOffsetX + 240, baseOffsetY + 246, FONT_NORMAL_RED);
    }
    else if (Data_CityInfo.religionDemand == 3)
    {
        Widget_GameText_draw(61, 48, baseOffsetX + 240, baseOffsetY + 246, FONT_NORMAL_RED);
    }
    else
    {
        Widget_GameText_draw(61, 49, baseOffsetX + 240, baseOffsetY + 246, FONT_NORMAL_GREEN);
    }

    // entertainment
    Graphics_drawImage(image_group(ID_Graphic_Bullet), baseOffsetX + 40, baseOffsetY + 267);
    Widget_GameText_draw(61, 10, baseOffsetX + 60, baseOffsetY + 266, FONT_NORMAL_WHITE);
    if (Data_CityInfo.entertainmentDemand == 1)
    {
        Widget_GameText_draw(61, 43, baseOffsetX + 240, baseOffsetY + 266, FONT_NORMAL_RED);
    }
    else if (Data_CityInfo.entertainmentDemand == 2)
    {
        Widget_GameText_draw(61, 44, baseOffsetX + 240, baseOffsetY + 266, FONT_NORMAL_RED);
    }
    else
    {
        Widget_GameText_draw(61, 45, baseOffsetX + 240, baseOffsetY + 266, FONT_NORMAL_GREEN);
    }

    // sentiment
    Graphics_drawImage(image_group(ID_Graphic_Bullet), baseOffsetX + 40, baseOffsetY + 287);
    Widget_GameText_draw(61, 11, baseOffsetX + 60, baseOffsetY + 286, FONT_NORMAL_WHITE);
    if (Data_CityInfo.citySentiment <= 0)
    {
        Widget_GameText_draw(61, 50, baseOffsetX + 240, baseOffsetY + 286, FONT_NORMAL_RED);
    }
    else if (Data_CityInfo.citySentiment >= 100)
    {
        Widget_GameText_draw(61, 61, baseOffsetX + 240, baseOffsetY + 286, FONT_NORMAL_GREEN);
    }
    else
    {
        Widget_GameText_draw(61, Data_CityInfo.citySentiment / 10 + 51,
                             baseOffsetX + 240, baseOffsetY + 286, FONT_NORMAL_GREEN);
    }
}
