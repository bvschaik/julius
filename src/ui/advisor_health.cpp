#include "advisors_private.h"

#include "building/count.h"
#include "city/culture.h"

void UI_Advisor_Health_drawBackground(int *advisorHeight)
{
    int baseOffsetX = Data_Screen.offset640x480.x;
    int baseOffsetY = Data_Screen.offset640x480.y;

    *advisorHeight = 18;
    Widget_Panel_drawOuterPanel(baseOffsetX, baseOffsetY, 40, *advisorHeight);
    Graphics_drawImage(image_group(ID_Graphic_AdvisorIcons) + 6, baseOffsetX + 10, baseOffsetY + 10);

    Widget_GameText_draw(56, 0, baseOffsetX + 60, baseOffsetY + 12, FONT_LARGE_BLACK);
    if (Data_CityInfo.population >= 200)
    {
        Widget_GameText_drawMultiline(56,
                                      Data_CityInfo.healthRate / 10 + 16,
                                      baseOffsetX + 60, baseOffsetY + 46,
                                      512, FONT_NORMAL_BLACK
                                     );
    }
    else
    {
        Widget_GameText_drawMultiline(56, 15,
                                      baseOffsetX + 60, baseOffsetY + 46,
                                      512, FONT_NORMAL_BLACK
                                     );
    }
    Widget_GameText_draw(56, 3, baseOffsetX + 180, baseOffsetY + 94, FONT_SMALL_PLAIN);
    Widget_GameText_draw(56, 4, baseOffsetX + 290, baseOffsetY + 94, FONT_SMALL_PLAIN);
    Widget_GameText_draw(56, 5, baseOffsetX + 478, baseOffsetY + 94, FONT_SMALL_PLAIN);

    Widget_Panel_drawInnerPanel(baseOffsetX + 32, baseOffsetY + 108, 36, 5);

    // bathhouses
    Widget_GameText_drawNumberWithDescription(8, 24,
            building_count_total(BUILDING_BATHHOUSE),
            baseOffsetX + 40, baseOffsetY + 112, FONT_NORMAL_GREEN
                                             );
    Widget::Text::drawNumberCentered(
        building_count_active(BUILDING_BATHHOUSE),
        baseOffsetX + 150, baseOffsetY + 112, 100, FONT_NORMAL_GREEN
    );
    Widget_GameText_draw(56, 2, baseOffsetX + 300, baseOffsetY + 112, FONT_NORMAL_GREEN);
    Widget_GameText_draw(56, 2, baseOffsetX + 504, baseOffsetY + 112, FONT_NORMAL_GREEN);

    // barbers
    Widget_GameText_drawNumberWithDescription(8, 26,
            building_count_total(BUILDING_BARBER),
            baseOffsetX + 40, baseOffsetY + 132, FONT_NORMAL_GREEN
                                             );
    Widget::Text::drawNumberCentered(
        building_count_active(BUILDING_BARBER),
        baseOffsetX + 150, baseOffsetY + 132, 100, FONT_NORMAL_GREEN
    );
    Widget_GameText_draw(56, 2, baseOffsetX + 300, baseOffsetY + 132, FONT_NORMAL_GREEN);
    Widget_GameText_draw(56, 2, baseOffsetX + 504, baseOffsetY + 132, FONT_NORMAL_GREEN);

    // clinics
    Widget_GameText_drawNumberWithDescription(8, 28,
            building_count_total(BUILDING_DOCTOR),
            baseOffsetX + 40, baseOffsetY + 152, FONT_NORMAL_GREEN
                                             );
    Widget::Text::drawNumberCentered(
        building_count_active(BUILDING_DOCTOR),
        baseOffsetX + 150, baseOffsetY + 152, 100, FONT_NORMAL_GREEN
    );
    Widget_GameText_draw(56, 2, baseOffsetX + 300, baseOffsetY + 152, FONT_NORMAL_GREEN);
    Widget_GameText_draw(56, 2, baseOffsetX + 504, baseOffsetY + 152, FONT_NORMAL_GREEN);

    // hospitals
    Widget_GameText_drawNumberWithDescription(8, 30,
            building_count_total(BUILDING_HOSPITAL),
            baseOffsetX + 40, baseOffsetY + 172, FONT_NORMAL_GREEN
                                             );
    Widget::Text::drawNumberCentered(
        building_count_active(BUILDING_HOSPITAL),
        baseOffsetX + 150, baseOffsetY + 172, 100, FONT_NORMAL_GREEN
    );

    int width = Widget::Text::drawNumber(
                    1000 * building_count_active(BUILDING_HOSPITAL),
                    '@', " ", baseOffsetX + 280, baseOffsetY + 172, FONT_NORMAL_GREEN
                );
    Widget_GameText_draw(56, 6, baseOffsetX + 280 + width, baseOffsetY + 172, FONT_NORMAL_GREEN);

    int pct_hospital = city_culture_coverage_hospital();
    if (pct_hospital == 0)
    {
        Widget_GameText_drawCentered(57, 10,
                                     baseOffsetX + 420, baseOffsetY + 172, 200, FONT_NORMAL_GREEN
                                    );
    }
    else if (pct_hospital < 100)
    {
        Widget_GameText_drawCentered(57, pct_hospital / 10 + 11,
                                     baseOffsetX + 420, baseOffsetY + 172, 200, FONT_NORMAL_GREEN
                                    );
    }
    else
    {
        Widget_GameText_drawCentered(57, 21,
                                     baseOffsetX + 420, baseOffsetY + 172, 200, FONT_NORMAL_GREEN
                                    );
    }

    int adviceId;
    switch (Data_CityInfo.healthDemand)
    {
    case 1:
        adviceId = Data_CityInfo.housesRequiringBathhouse ? 1 : 0;
        break;
    case 2:
        adviceId = Data_CityInfo.housesRequiringBarber ? 3 : 2;
        break;
    case 3:
        adviceId = Data_CityInfo.housesRequiringClinic ? 5 : 4;
        break;
    case 4:
        adviceId = 6;
        break;
    default:
        adviceId = 7;
        break;
    }
    Widget_GameText_drawMultiline(56, 7 + adviceId,
                                  baseOffsetX + 60, baseOffsetY + 194, 512, FONT_NORMAL_BLACK
                                 );
}
