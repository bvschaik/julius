#include "religion.h"

#include "building/count.h"
#include "city/gods.h"
#include "game/settings.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"

#include "Data/CityInfo.h"

static int get_religion_advice()
{
    if (Data_CityInfo.godLeastHappy > 0 && Data_CityInfo.godWrathBolts[Data_CityInfo.godLeastHappy - 1] > 4) {
        return 5 + Data_CityInfo.godLeastHappy;
    } else if (Data_CityInfo.religionDemand == 1) {
        return Data_CityInfo.housesRequiringReligion ? 1 : 0;
    } else if (Data_CityInfo.religionDemand == 2) {
        return 2;
    } else if (Data_CityInfo.religionDemand == 3) {
        return 3;
    } else if (!Data_CityInfo.housesRequiringReligion) {
        return 4;
    } else if (Data_CityInfo.godLeastHappy) {
        return 5 + Data_CityInfo.godLeastHappy;
    } else {
        return 5;
    }
}

static void draw_god_row(god_type god, int y_offset, building_type small_temple, building_type large_temple)
{
    lang_text_draw(59, 11 + god, 40, y_offset, FONT_NORMAL_WHITE);
    lang_text_draw(59, 16 + god, 120, y_offset + 1, FONT_SMALL_PLAIN);
    text_draw_number_centered(building_count_total(small_temple), 230, y_offset, 50, FONT_NORMAL_WHITE);
    text_draw_number_centered(building_count_total(large_temple), 290, y_offset, 50, FONT_NORMAL_WHITE);
    text_draw_number_centered(Data_CityInfo.godMonthsSinceFestival[god], 360, y_offset, 50, FONT_NORMAL_WHITE);
    int width = lang_text_draw(59, 32 + Data_CityInfo.godHappiness[god] / 10, 460, y_offset, FONT_NORMAL_WHITE);
    for (int i = 0; i < Data_CityInfo.godWrathBolts[god] / 10; i++) {
        image_draw(image_group(GROUP_GOD_BOLT), 10 * i + width + 460, y_offset - 4);
    }
}

static int draw_background()
{
    int height_blocks;
    if (setting_gods_enabled()) {
        height_blocks = 17;
        outer_panel_draw(0, 0, 40, height_blocks);
    } else {
        height_blocks = 20;
        outer_panel_draw(0, 0, 40, height_blocks);
        lang_text_draw_multiline(59, 43, 60, 256, 520, FONT_NORMAL_BLACK);
    }
    
    image_draw(image_group(GROUP_ADVISOR_ICONS) + 9, 10, 10);
    
    lang_text_draw(59, 0, 60, 12, FONT_LARGE_BLACK);
    
    // table header
    lang_text_draw(59, 5, 270, 32, FONT_SMALL_PLAIN);
    lang_text_draw(59, 1, 240, 46, FONT_SMALL_PLAIN);
    lang_text_draw(59, 2, 300, 46, FONT_SMALL_PLAIN);
    lang_text_draw(59, 3, 450, 46, FONT_SMALL_PLAIN);
    lang_text_draw(59, 6, 370, 18, FONT_SMALL_PLAIN);
    lang_text_draw(59, 9, 370, 32, FONT_SMALL_PLAIN);
    lang_text_draw(59, 7, 370, 46, FONT_SMALL_PLAIN);
    
    inner_panel_draw(32, 60, 36, 8);
    
    // god rows
    draw_god_row(GOD_CERES, 66, BUILDING_SMALL_TEMPLE_CERES, BUILDING_LARGE_TEMPLE_CERES);
    draw_god_row(GOD_NEPTUNE, 86, BUILDING_SMALL_TEMPLE_NEPTUNE, BUILDING_LARGE_TEMPLE_NEPTUNE);
    draw_god_row(GOD_MERCURY, 106, BUILDING_SMALL_TEMPLE_MERCURY, BUILDING_LARGE_TEMPLE_MERCURY);
    draw_god_row(GOD_MARS, 126, BUILDING_SMALL_TEMPLE_MARS, BUILDING_LARGE_TEMPLE_MARS);
    draw_god_row(GOD_VENUS, 146, BUILDING_SMALL_TEMPLE_VENUS, BUILDING_LARGE_TEMPLE_VENUS);

    // oracles
    lang_text_draw(59, 8, 40, 166, FONT_NORMAL_WHITE);
    text_draw_number_centered(building_count_total(BUILDING_ORACLE), 230, 166, 50, FONT_NORMAL_WHITE);
    
    city_gods_calculate_least_happy();

    lang_text_draw_multiline(59, 21 + get_religion_advice(), 60, 196, 512, FONT_NORMAL_BLACK);

    return height_blocks;
}

const advisor_window_type *window_advisor_religion()
{
    static const advisor_window_type window = {
        draw_background,
        0,
        0,
        0
    };
    return &window;
}
