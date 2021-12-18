#include "health.h"
#include "building/count.h"
#include "city/culture.h"
#include "city/health.h"
#include "city/houses.h"
#include "city/population.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"

#define ADVISOR_HEIGHT 26

static int get_health_advice(void)
{
    house_demands *demands = city_houses_demands();
    switch (demands->health) {
        case 1:
            return demands->requiring.bathhouse ? 1 : 0;
        case 2:
            return demands->requiring.barber ? 3 : 2;
        case 3:
            return demands->requiring.clinic ? 5 : 4;
        case 4:
            return 6;
        default:
            return 7;
    }
}

static int draw_background(void)
{
    outer_panel_draw(0, 0, 40, ADVISOR_HEIGHT);
    image_draw(image_group(GROUP_ADVISOR_ICONS) + 6, 10, 10);

    int sickness_level = city_health_get_global_sickness_level();
    int text_id = city_health() / 10 + 16;

    if (sickness_level == SICKNESS_LEVEL_HIGH) {
        text_id = 18;
    } else if (sickness_level == SICKNESS_LEVEL_PLAGUE) {
        text_id = 16;
    }

    lang_text_draw(56, 0, 60, 12, FONT_LARGE_BLACK);
    if (city_population() >= 200) {
        lang_text_draw_multiline(56, text_id, 60, 46, 512, FONT_NORMAL_BLACK);
    } else {
        lang_text_draw_multiline(56, 15, 60, 46, 512, FONT_NORMAL_BLACK);
    }
    lang_text_draw(56, 3, 180, 94, FONT_SMALL_PLAIN);
    lang_text_draw(56, 4, 290, 94, FONT_SMALL_PLAIN);
    lang_text_draw_centered(56, 5, 440, 94, 160, FONT_SMALL_PLAIN);

    inner_panel_draw(32, 108, 36, 5);

    // bathhouses
    lang_text_draw_amount(8, 24, building_count_total(BUILDING_BATHHOUSE), 40, 112, FONT_NORMAL_WHITE);
    text_draw_number_centered(building_count_active(BUILDING_BATHHOUSE), 150, 112, 100, FONT_NORMAL_WHITE);
    lang_text_draw_centered(56, 2, 290, 112, 120, FONT_NORMAL_WHITE);
    lang_text_draw_centered(56, 2, 440, 112, 160, FONT_NORMAL_WHITE);

    // barbers
    lang_text_draw_amount(8, 26, building_count_total(BUILDING_BARBER), 40, 132, FONT_NORMAL_WHITE);
    text_draw_number_centered(building_count_active(BUILDING_BARBER), 150, 132, 100, FONT_NORMAL_WHITE);
    lang_text_draw_centered(56, 2, 290, 132, 120, FONT_NORMAL_WHITE);
    lang_text_draw_centered(56, 2, 440, 132, 160, FONT_NORMAL_WHITE);

    // clinics
    lang_text_draw_amount(8, 28, building_count_total(BUILDING_DOCTOR), 40, 152, FONT_NORMAL_WHITE);
    text_draw_number_centered(building_count_active(BUILDING_DOCTOR), 150, 152, 100, FONT_NORMAL_WHITE);
    lang_text_draw_centered(56, 2, 290, 152, 120, FONT_NORMAL_WHITE);
    lang_text_draw_centered(56, 2, 440, 152, 160, FONT_NORMAL_WHITE);

    // hospitals
    lang_text_draw_amount(8, 30, building_count_total(BUILDING_HOSPITAL), 40, 172, FONT_NORMAL_WHITE);
    text_draw_number_centered(building_count_active(BUILDING_HOSPITAL), 150, 172, 100, FONT_NORMAL_WHITE);

    int width = text_draw_number(1000 * building_count_active(BUILDING_HOSPITAL), '@', " ",
        280, 172, FONT_NORMAL_WHITE, 0);
    lang_text_draw(56, 6, 280 + width, 172, FONT_NORMAL_WHITE);

    int pct_hospital = city_culture_coverage_hospital();

    if (pct_hospital == 0) {
        lang_text_draw_centered(57, 10, 440, 172, 160, FONT_NORMAL_WHITE);
    } else if (pct_hospital < 100) {
        lang_text_draw_centered(57, pct_hospital / 10 + 11, 440, 172, 160, FONT_NORMAL_WHITE);
    } else {
        lang_text_draw_centered(57, 21, 440, 172, 160, FONT_NORMAL_WHITE);
    }

    int text_height = lang_text_draw_multiline(56, 7 + get_health_advice(), 60, 210, 512, FONT_NORMAL_BLACK);

    if (sickness_level == SICKNESS_LEVEL_LOW) {
        text_draw_multiline(translation_for(TR_ADVISOR_SICKNESS_LEVEL_LOW), 60, 230 + text_height, 512, FONT_NORMAL_BLACK, 0);
    } else if (sickness_level == SICKNESS_LEVEL_MEDIUM) {
        text_draw_multiline(translation_for(TR_ADVISOR_SICKNESS_LEVEL_MEDIUM), 60, 230 + text_height,512,FONT_NORMAL_BLACK, 0);
    } else if (sickness_level == SICKNESS_LEVEL_HIGH) {
        text_draw_multiline(translation_for(TR_ADVISOR_SICKNESS_LEVEL_HIGH), 60, 230 + text_height, 512, FONT_NORMAL_BLACK, 0);
    } else { // plague
        text_draw_multiline(translation_for(TR_ADVISOR_SICKNESS_LEVEL_PLAGUE), 60, 230 + text_height,512, FONT_NORMAL_BLACK, 0);
    }

    return ADVISOR_HEIGHT;
}

const advisor_window_type *window_advisor_health(void)
{
    static const advisor_window_type window = {
        draw_background,
        0,
        0,
        0
    };
    return &window;
}
