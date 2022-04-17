#include "religion.h"

#include "assets/assets.h"
#include "building/count.h"
#include "city/festival.h"
#include "city/gods.h"
#include "city/houses.h"
#include "game/settings.h"
#include "graphics/generic_button.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "window/hold_festival.h"

static void button_hold_festival(int param1, int param2);

static generic_button hold_festival_button[] = {
    {102, 340, 300, 20, button_hold_festival, button_none, 0, 0},
};

static int focus_button_id;

static int get_religion_advice(void)
{
    int least_happy = city_god_least_happy();
    const house_demands *demands = city_houses_demands();
    if (least_happy >= 0 && city_god_wrath_bolts(least_happy) > 4) {
        return 6 + least_happy;
    } else if (demands->religion == 1) {
        return demands->requiring.religion ? 1 : 0;
    } else if (demands->religion == 2) {
        return 2;
    } else if (demands->religion == 3) {
        return 3;
    } else if (!demands->requiring.religion) {
        return 4;
    } else if (least_happy >= 0) {
        return 6 + least_happy;
    } else {
        return 5;
    }
}

static void draw_god_row(god_type god, int y_offset, building_type small_temple,
    building_type large_temple, building_type grand_temple)
{
    lang_text_draw(59, 11 + god, 40, y_offset, FONT_NORMAL_WHITE);
    lang_text_draw(59, 16 + god, 120, y_offset + 1, FONT_SMALL_PLAIN);
    text_draw_number_centered(building_count_active(small_temple), 230, y_offset, 50, FONT_NORMAL_WHITE);
    if (building_count_active(grand_temple)) {
        text_draw_number_centered(building_count_active(large_temple) + building_count_active(grand_temple),
            290, y_offset, 50, FONT_NORMAL_GREEN);
    } else {
        text_draw_number_centered(building_count_active(large_temple), 290, y_offset, 50, FONT_NORMAL_WHITE);
    }
    text_draw_number_centered(city_god_months_since_festival(god), 360, y_offset, 50, FONT_NORMAL_WHITE);
    int width = lang_text_draw(59, 32 + city_god_happiness(god) / 10, 460, y_offset, FONT_NORMAL_WHITE);
    int bolts = city_god_wrath_bolts(god);
    for (int i = 0; i < bolts / 10; i++) {
        image_draw(image_group(GROUP_GOD_BOLT), 10 * i + width + 460, y_offset - 4, COLOR_MASK_NONE, SCALE_NONE);
    }
    int happy_bolts = city_god_happy_bolts(god);
    for (int i = 0; i < happy_bolts; i++) {
        image_draw(assets_get_image_id("UI", "Happy God Icon"),
            10 * i + width + 460, y_offset - 4, COLOR_MASK_NONE, SCALE_NONE);
    }
}

static void draw_oracle_row(void)
{
    int oracle_count = building_count_active(BUILDING_ORACLE) + building_count_active(BUILDING_SMALL_MAUSOLEUM);
    int large_oracle_count = building_count_active(BUILDING_NYMPHAEUM) +
        building_count_active(BUILDING_PANTHEON) + building_count_active(BUILDING_LARGE_MAUSOLEUM);
    lang_text_draw(59, 8, 40, 166, FONT_NORMAL_WHITE);
    text_draw_number_centered(oracle_count, 230, 166, 50, FONT_NORMAL_WHITE);
    if (building_count_active(BUILDING_PANTHEON)) {
        text_draw_number_centered(large_oracle_count, 290, 166, 50, FONT_NORMAL_GREEN);
    } else {
        text_draw_number_centered(large_oracle_count, 290, 166, 50, FONT_NORMAL_WHITE);
    }
}

static void draw_lararium_row(void)
{
    text_draw(translation_for(TR_WINDOW_ADVISOR_RELIGION_LARARIUMS), 40, 186, FONT_NORMAL_WHITE, 0);
    text_draw_number_centered(building_count_total(BUILDING_LARARIUM), 230, 186, 50, FONT_NORMAL_WHITE);
}

static int get_festival_advice(void)
{
    int months_since_festival = city_festival_months_since_last();
    if (months_since_festival <= 1) {
        return 0;
    } else if (months_since_festival <= 6) {
        return 1;
    } else if (months_since_festival <= 12) {
        return 2;
    } else if (months_since_festival <= 18) {
        return 3;
    } else if (months_since_festival <= 24) {
        return 4;
    } else if (months_since_festival <= 30) {
        return 5;
    } else {
        return 6;
    }
}

static void draw_festival_info(void)
{
    inner_panel_draw(48, 302, 34, 6);
    image_draw(image_group(GROUP_PANEL_WINDOWS) + 15, 460, 305, COLOR_MASK_NONE, SCALE_NONE);
    lang_text_draw(58, 17, 52, 274, FONT_LARGE_BLACK);

    int width = lang_text_draw_amount(8, 4, city_festival_months_since_last(), 112, 315, FONT_NORMAL_WHITE);
    lang_text_draw(58, 15, 112 + width, 315, FONT_NORMAL_WHITE);
    if (city_festival_is_planned()) {
        lang_text_draw_centered(58, 34, 102, 339, 300, FONT_NORMAL_WHITE);
    } else {
        lang_text_draw_centered(58, 16, 102, 339, 300, FONT_NORMAL_WHITE);
    }
    lang_text_draw_multiline(58, 18 + get_festival_advice(), 56, 360, 400, FONT_NORMAL_WHITE);
}

static int draw_background(void)
{
    int height_blocks;
    height_blocks = 27;
    outer_panel_draw(0, 0, 40, height_blocks);

    image_draw(image_group(GROUP_ADVISOR_ICONS) + 9, 10, 10, COLOR_MASK_NONE, SCALE_NONE);

    lang_text_draw(59, 0, 60, 12, FONT_LARGE_BLACK);

    // table header
    lang_text_draw(59, 5, 270, 32, FONT_SMALL_PLAIN);
    lang_text_draw(59, 1, 240, 46, FONT_SMALL_PLAIN);
    lang_text_draw(59, 2, 300, 46, FONT_SMALL_PLAIN);
    lang_text_draw(59, 3, 450, 46, FONT_SMALL_PLAIN);
    lang_text_draw(59, 6, 370, 18, FONT_SMALL_PLAIN);
    lang_text_draw(59, 9, 370, 32, FONT_SMALL_PLAIN);
    lang_text_draw(59, 7, 370, 46, FONT_SMALL_PLAIN);

    inner_panel_draw(32, 60, 36, 9);

    // god rows
    draw_god_row(GOD_CERES, 66, BUILDING_SMALL_TEMPLE_CERES,
        BUILDING_LARGE_TEMPLE_CERES, BUILDING_GRAND_TEMPLE_CERES);
    draw_god_row(GOD_NEPTUNE, 86, BUILDING_SMALL_TEMPLE_NEPTUNE,
        BUILDING_LARGE_TEMPLE_NEPTUNE, BUILDING_GRAND_TEMPLE_NEPTUNE);
    draw_god_row(GOD_MERCURY, 106, BUILDING_SMALL_TEMPLE_MERCURY,
        BUILDING_LARGE_TEMPLE_MERCURY, BUILDING_GRAND_TEMPLE_MERCURY);
    draw_god_row(GOD_MARS, 126, BUILDING_SMALL_TEMPLE_MARS,
        BUILDING_LARGE_TEMPLE_MARS, BUILDING_GRAND_TEMPLE_MARS);
    draw_god_row(GOD_VENUS, 146, BUILDING_SMALL_TEMPLE_VENUS,
        BUILDING_LARGE_TEMPLE_VENUS, BUILDING_GRAND_TEMPLE_VENUS);

    // oracles
    draw_oracle_row();

    // larariums
    draw_lararium_row();

    city_gods_calculate_least_happy();

    lang_text_draw_multiline(59, 21 + get_religion_advice(), 60, 216, 512, FONT_NORMAL_BLACK);

    draw_festival_info();

    return height_blocks;
}

static void draw_foreground(void)
{
    if (!city_festival_is_planned()) {
        button_border_draw(102, 335, 300, 20, focus_button_id == 1);
    }
}

static int handle_mouse(const mouse *m)
{
    return generic_buttons_handle_mouse(m, 0, 0, hold_festival_button, 1, &focus_button_id);
}

static void button_hold_festival(int param1, int param2)
{
    if (!city_festival_is_planned()) {
        window_hold_festival_show();
    }
}

static void get_tooltip_text(advisor_tooltip_result *r)
{
    if (focus_button_id) {
        r->text_id = 112;
    }
}

const advisor_window_type *window_advisor_religion(void)
{
    static const advisor_window_type window = {
        draw_background,
        draw_foreground,
        handle_mouse,
        get_tooltip_text
    };
    focus_button_id = 0;
    return &window;
}
