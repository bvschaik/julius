#include "advisors_private.h"
#include "cityinfo.h"

#include "building/count.h"
#include "game/settings.h"

#include <ui>

void UI_Advisor_Religion_drawBackground(int *advisorHeight)
{
    int baseOffsetX = Data_Screen.offset640x480.x;
    int baseOffsetY = Data_Screen.offset640x480.y;

    if (setting_gods_enabled())
    {
        *advisorHeight = 17;
        Widget::Panel::drawOuterPanel(baseOffsetX, baseOffsetY, 40, *advisorHeight);
    }
    else
    {
        *advisorHeight = 20;
        Widget::Panel::drawOuterPanel(baseOffsetX, baseOffsetY, 40, *advisorHeight);
        Widget_GameText_drawMultiline(59, 43,
                                      baseOffsetX + 60, baseOffsetY + 256, 520, FONT_NORMAL_BLACK
                                     );
    }

    Graphics_drawImage(image_group(ID_Graphic_AdvisorIcons) + 9, baseOffsetX + 10, baseOffsetY + 10);

    Widget_GameText_draw(59, 0, baseOffsetX + 60, baseOffsetY + 12, FONT_LARGE_BLACK);

    // table header
    Widget_GameText_draw(59, 5, baseOffsetX + 270, baseOffsetY + 32, FONT_SMALL_PLAIN);
    Widget_GameText_draw(59, 1, baseOffsetX + 240, baseOffsetY + 46, FONT_SMALL_PLAIN);
    Widget_GameText_draw(59, 2, baseOffsetX + 300, baseOffsetY + 46, FONT_SMALL_PLAIN);
    Widget_GameText_draw(59, 3, baseOffsetX + 450, baseOffsetY + 46, FONT_SMALL_PLAIN);
    Widget_GameText_draw(59, 6, baseOffsetX + 370, baseOffsetY + 18, FONT_SMALL_PLAIN);
    Widget_GameText_draw(59, 9, baseOffsetX + 370, baseOffsetY + 32, FONT_SMALL_PLAIN);
    Widget_GameText_draw(59, 7, baseOffsetX + 370, baseOffsetY + 46, FONT_SMALL_PLAIN);

    Widget::Panel::drawInnerPanel(baseOffsetX + 32, baseOffsetY + 60, 36, 8);

    int graphicIdBolt = image_group(ID_Graphic_GodBolt);
    int width;

    // Ceres
    Widget_GameText_draw(59, 11, baseOffsetX + 40, baseOffsetY + 66, FONT_NORMAL_WHITE);
    Widget_GameText_draw(59, 16, baseOffsetX + 120, baseOffsetY + 67, FONT_SMALL_PLAIN);
    Widget::Text::drawNumberCentered(
        building_count_total(BUILDING_SMALL_TEMPLE_CERES),
        baseOffsetX + 230, baseOffsetY + 66, 50, FONT_NORMAL_WHITE
    );
    Widget::Text::drawNumberCentered(
        building_count_total(BUILDING_LARGE_TEMPLE_CERES),
        baseOffsetX + 290, baseOffsetY + 66, 50, FONT_NORMAL_WHITE
    );
    Widget::Text::drawNumberCentered(
        Data_CityInfo.godMonthsSinceFestival[God_Ceres],
        baseOffsetX + 360, baseOffsetY + 66, 50, FONT_NORMAL_WHITE
    );
    width = Widget_GameText_draw(59, 32 + Data_CityInfo.godHappiness[God_Ceres] / 10,
                                 baseOffsetX + 460, baseOffsetY + 66, FONT_NORMAL_WHITE
                                );
    for (int i = 0; i < Data_CityInfo.godWrathBolts[God_Ceres] / 10; i++)
    {
        Graphics_drawImage(graphicIdBolt,
                           10 * i + baseOffsetX + width + 460, baseOffsetY + 62
                          );
    }

    // Neptune
    Widget_GameText_draw(59, 12, baseOffsetX + 40, baseOffsetY + 86, FONT_NORMAL_WHITE);
    Widget_GameText_draw(59, 17, baseOffsetX + 120, baseOffsetY + 87, FONT_SMALL_PLAIN);
    Widget::Text::drawNumberCentered(
        building_count_total(BUILDING_SMALL_TEMPLE_NEPTUNE),
        baseOffsetX + 230, baseOffsetY + 86, 50, FONT_NORMAL_WHITE
    );
    Widget::Text::drawNumberCentered(
        building_count_total(BUILDING_LARGE_TEMPLE_NEPTUNE),
        baseOffsetX + 290, baseOffsetY + 86, 50, FONT_NORMAL_WHITE
    );
    Widget::Text::drawNumberCentered(
        Data_CityInfo.godMonthsSinceFestival[God_Neptune],
        baseOffsetX + 360, baseOffsetY + 86, 50, FONT_NORMAL_WHITE
    );
    width = Widget_GameText_draw(59, 32 + Data_CityInfo.godHappiness[God_Neptune] / 10,
                                 baseOffsetX + 460, baseOffsetY + 86, FONT_NORMAL_WHITE
                                );
    for (int i = 0; i < Data_CityInfo.godWrathBolts[God_Neptune] / 10; i++)
    {
        Graphics_drawImage(graphicIdBolt,
                           10 * i + baseOffsetX + width + 460, baseOffsetY + 82
                          );
    }

    // Mercury
    Widget_GameText_draw(59, 13, baseOffsetX + 40, baseOffsetY + 106, FONT_NORMAL_WHITE);
    Widget_GameText_draw(59, 18, baseOffsetX + 120, baseOffsetY + 107, FONT_SMALL_PLAIN);
    Widget::Text::drawNumberCentered(
        building_count_total(BUILDING_SMALL_TEMPLE_MERCURY),
        baseOffsetX + 230, baseOffsetY + 106, 50, FONT_NORMAL_WHITE
    );
    Widget::Text::drawNumberCentered(
        building_count_total(BUILDING_LARGE_TEMPLE_MERCURY),
        baseOffsetX + 290, baseOffsetY + 106, 50, FONT_NORMAL_WHITE
    );
    Widget::Text::drawNumberCentered(
        Data_CityInfo.godMonthsSinceFestival[God_Mercury],
        baseOffsetX + 360, baseOffsetY + 106, 50, FONT_NORMAL_WHITE
    );
    width = Widget_GameText_draw(59, 32 + Data_CityInfo.godHappiness[God_Mercury] / 10,
                                 baseOffsetX + 460, baseOffsetY + 106, FONT_NORMAL_WHITE
                                );
    for (int i = 0; i < Data_CityInfo.godWrathBolts[God_Mercury] / 10; i++)
    {
        Graphics_drawImage(graphicIdBolt,
                           10 * i + baseOffsetX + width + 460, baseOffsetY + 102
                          );
    }

    // Mars
    Widget_GameText_draw(59, 14, baseOffsetX + 40, baseOffsetY + 126, FONT_NORMAL_WHITE);
    Widget_GameText_draw(59, 19, baseOffsetX + 120, baseOffsetY + 127, FONT_SMALL_PLAIN);
    Widget::Text::drawNumberCentered(
        building_count_total(BUILDING_SMALL_TEMPLE_MARS),
        baseOffsetX + 230, baseOffsetY + 126, 50, FONT_NORMAL_WHITE
    );
    Widget::Text::drawNumberCentered(
        building_count_total(BUILDING_LARGE_TEMPLE_MARS),
        baseOffsetX + 290, baseOffsetY + 126, 50, FONT_NORMAL_WHITE
    );
    Widget::Text::drawNumberCentered(
        Data_CityInfo.godMonthsSinceFestival[God_Mars],
        baseOffsetX + 360, baseOffsetY + 126, 50, FONT_NORMAL_WHITE
    );
    width = Widget_GameText_draw(59, 32 + Data_CityInfo.godHappiness[God_Mars] / 10,
                                 baseOffsetX + 460, baseOffsetY + 126, FONT_NORMAL_WHITE
                                );
    for (int i = 0; i < Data_CityInfo.godWrathBolts[God_Mars] / 10; i++)
    {
        Graphics_drawImage(graphicIdBolt,
                           10 * i + baseOffsetX + width + 460, baseOffsetY + 122
                          );
    }

    // Venus
    Widget_GameText_draw(59, 15, baseOffsetX + 40, baseOffsetY + 146, FONT_NORMAL_WHITE);
    Widget_GameText_draw(59, 20, baseOffsetX + 120, baseOffsetY + 147, FONT_SMALL_PLAIN);
    Widget::Text::drawNumberCentered(
        building_count_total(BUILDING_SMALL_TEMPLE_VENUS),
        baseOffsetX + 230, baseOffsetY + 146, 50, FONT_NORMAL_WHITE
    );
    Widget::Text::drawNumberCentered(
        building_count_total(BUILDING_LARGE_TEMPLE_VENUS),
        baseOffsetX + 290, baseOffsetY + 146, 50, FONT_NORMAL_WHITE
    );
    Widget::Text::drawNumberCentered(
        Data_CityInfo.godMonthsSinceFestival[God_Venus],
        baseOffsetX + 360, baseOffsetY + 146, 50, FONT_NORMAL_WHITE
    );
    width = Widget_GameText_draw(59, 32 + Data_CityInfo.godHappiness[God_Venus] / 10,
                                 baseOffsetX + 460, baseOffsetY + 146, FONT_NORMAL_WHITE
                                );
    for (int i = 0; i < Data_CityInfo.godWrathBolts[God_Venus] / 10; i++)
    {
        Graphics_drawImage(graphicIdBolt,
                           10 * i + baseOffsetX + width + 460, baseOffsetY + 142
                          );
    }

    // oracles
    Widget_GameText_draw(59, 8, baseOffsetX + 40, baseOffsetY + 166, FONT_NORMAL_WHITE);
    Widget::Text::drawNumberCentered(
        building_count_total(BUILDING_ORACLE),
        baseOffsetX + 230, baseOffsetY + 166, 50, FONT_NORMAL_WHITE
    );

    CityInfo_Gods_calculateLeastHappy();

    int adviceId;
    if (Data_CityInfo.godLeastHappy > 0 && Data_CityInfo.godWrathBolts[Data_CityInfo.godLeastHappy - 1] > 4)
    {
        adviceId = 5 + Data_CityInfo.godLeastHappy;
    }
    else if (Data_CityInfo.religionDemand == 1)
    {
        adviceId = Data_CityInfo.housesRequiringReligion ? 1 : 0;
    }
    else if (Data_CityInfo.religionDemand == 2)
    {
        adviceId = 2;
    }
    else if (Data_CityInfo.religionDemand == 3)
    {
        adviceId = 3;
    }
    else if (!Data_CityInfo.housesRequiringReligion)
    {
        adviceId = 4;
    }
    else if (Data_CityInfo.godLeastHappy)
    {
        adviceId = 5 + Data_CityInfo.godLeastHappy;
    }
    else
    {
        adviceId = 5;
    }
    Widget_GameText_drawMultiline(59, 21 + adviceId,
                                  baseOffsetX + 60, baseOffsetY + 196, 512, FONT_NORMAL_BLACK);
}
