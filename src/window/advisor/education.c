#include "education.h"

#include "building/count.h"
#include "city/culture.h"
#include "city/houses.h"
#include "city/population.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"

#define ADVISOR_HEIGHT 16

static int get_education_advice(void)
{
    const house_demands *demands = city_houses_demands();
    if (demands->education == 1) {
        return demands->requiring.school ? 1 : 0;
    } else if (demands->education == 2) {
        return demands->requiring.library ? 3 : 2;
    } else if (demands->education == 3) {
        return 4;
    }
    int advice_id;
    int coverage_school = city_culture_coverage_school();
    int coverage_academy = city_culture_coverage_academy();
    int coverage_library = city_culture_coverage_library();
    if (!demands->requiring.school) {
        advice_id = 5; // no demands yet
    } else if (!demands->requiring.library) {
        if (coverage_school >= 100 && coverage_academy >= 100) {
            advice_id = 6; // education is perfect
        } else if (coverage_school <= coverage_academy) {
            advice_id = 7; // build more schools
        } else {
            advice_id = 8; // build more academies
        }
    } else {
        // all education needed
        if (coverage_school >= 100 && coverage_academy >= 100 && coverage_library >= 100) {
            advice_id = 6;
        } else if (coverage_school <= coverage_academy && coverage_school <= coverage_library) {
            advice_id = 7; // build more schools
        } else if (coverage_academy <= coverage_school && coverage_academy <= coverage_library) {
            advice_id = 8; // build more academies
        } else if (coverage_library <= coverage_school && coverage_library <= coverage_academy) {
            advice_id = 9; // build more libraries
        } else {
            advice_id = 6; // unlikely event that all coverages are equal
        }
    }
    return advice_id;
}

static int draw_background(void)
{
    outer_panel_draw(0, 0, 40, ADVISOR_HEIGHT);
    image_draw(image_group(GROUP_ADVISOR_ICONS) + 7, 10, 10);
    lang_text_draw(57, 0, 60, 12, FONT_LARGE_BLACK);

    // x population, y school age, z academy age
    int width = text_draw_number(city_population(), '@', " ", 60, 50, FONT_NORMAL_BLACK);
    width += lang_text_draw(57, 1, 60 + width, 50, FONT_NORMAL_BLACK);
    width += text_draw_number(city_population_school_age(), '@', " ", 60 + width, 50, FONT_NORMAL_BLACK);
    width += lang_text_draw(57, 2, 60 + width, 50, FONT_NORMAL_BLACK);
    width += text_draw_number(city_population_academy_age(), '@', " ", 60 + width, 50, FONT_NORMAL_BLACK);
    lang_text_draw(57, 3, 60 + width, 50, FONT_NORMAL_BLACK);

    // table headers
    lang_text_draw(57, 4, 180, 86, FONT_SMALL_PLAIN);
    lang_text_draw(57, 5, 290, 86, FONT_SMALL_PLAIN);
    lang_text_draw(57, 6, 478, 86, FONT_SMALL_PLAIN);

    inner_panel_draw(32, 100, 36, 4);

    // schools
    lang_text_draw_amount(8, 18, building_count_total(BUILDING_SCHOOL), 40, 105, FONT_NORMAL_WHITE);
    text_draw_number_centered(building_count_active(BUILDING_SCHOOL), 150, 105, 100, FONT_NORMAL_WHITE);

    width = text_draw_number(75 * building_count_active(BUILDING_SCHOOL), '@', " ", 280, 105, FONT_NORMAL_WHITE);
    lang_text_draw(57, 7, 280 + width, 105, FONT_NORMAL_WHITE);

    int pct_school = city_culture_coverage_school();
    if (pct_school == 0) {
        lang_text_draw_centered(57, 10, 420, 105, 200, FONT_NORMAL_WHITE);
    } else if (pct_school < 100) {
        lang_text_draw_centered(57, pct_school / 10 + 11, 420, 105, 200, FONT_NORMAL_WHITE);
    } else {
        lang_text_draw_centered(57, 21, 420, 105, 200, FONT_NORMAL_WHITE);
    }

    // academies
    lang_text_draw_amount(8, 20, building_count_total(BUILDING_ACADEMY), 40, 125, FONT_NORMAL_WHITE);
    text_draw_number_centered(building_count_active(BUILDING_ACADEMY), 150, 125, 100, FONT_NORMAL_WHITE);

    width = text_draw_number(100 * building_count_active(BUILDING_ACADEMY), '@', " ", 280, 125, FONT_NORMAL_WHITE);
    lang_text_draw(57, 8, 280 + width, 125, FONT_NORMAL_WHITE);

    int pct_academy = city_culture_coverage_academy();
    if (pct_academy == 0) {
        lang_text_draw_centered(57, 10, 420, 125, 200, FONT_NORMAL_WHITE);
    } else if (pct_academy < 100) {
        lang_text_draw_centered(57, pct_academy / 10 + 11, 420, 125, 200, FONT_NORMAL_WHITE);
    } else {
        lang_text_draw_centered(57, 21, 420, 125, 200, FONT_NORMAL_WHITE);
    }

    // libraries
    lang_text_draw_amount(8, 22, building_count_total(BUILDING_LIBRARY), 40, 145, FONT_NORMAL_WHITE);
    text_draw_number_centered(building_count_active(BUILDING_LIBRARY), 150, 145, 100, FONT_NORMAL_WHITE);

    width = text_draw_number(800 * building_count_active(BUILDING_LIBRARY), '@', " ", 280, 145, FONT_NORMAL_WHITE);
    lang_text_draw(57, 9, 280 + width, 145, FONT_NORMAL_WHITE);

    int pct_library = city_culture_coverage_library();
    if (pct_library == 0) {
        lang_text_draw_centered(57, 10, 420, 145, 200, FONT_NORMAL_WHITE);
    } else if (pct_library < 100) {
        lang_text_draw_centered(57, pct_library / 10 + 11, 420, 145, 200, FONT_NORMAL_WHITE);
    } else {
        lang_text_draw_centered(57, 21, 420, 145, 200, FONT_NORMAL_WHITE);
    }

    lang_text_draw_multiline(57, 22 + get_education_advice(), 60, 180, 512, FONT_NORMAL_BLACK);

    return ADVISOR_HEIGHT;
}

const advisor_window_type *window_advisor_education(void)
{
    static const advisor_window_type window = {
        draw_background,
        0,
        0,
        0
    };
    return &window;
}
