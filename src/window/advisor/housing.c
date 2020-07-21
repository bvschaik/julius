#include "housing.h"

#include "city/migration.h"
#include "city/population.h"
#include "city/resource.h"
#include "game/time.h"
#include "game/resource.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "scenario/property.h"
#include "translation/translation.h"


#define ADVISOR_HEIGHT 27

static void go_back(int param1, int param2);

static generic_button back_button[] = {
    {545, 260, 60, 51, go_back, button_none, 0, 1}
};

static int focus_button_id;

static void draw_housing_table()
{
    int *housing_type_counts;
    int *houses_demanding_goods;
    int width;
    int y_offset = 68;
    int rows = 0;
    translation_key goods_demand_strings[4] = {TR_ADVISOR_RESIDENCES_DEMANDING_POTTERY, TR_ADVISOR_RESIDENCES_DEMANDING_FURNITURE, TR_ADVISOR_RESIDENCES_DEMANDING_OIL, TR_ADVISOR_RESIDENCES_DEMANDING_WINE};
    int goods_icons[4] = {RESOURCE_POTTERY, RESOURCE_FURNITURE, RESOURCE_OIL, RESOURCE_WINE};

    housing_type_counts = calculate_number_of_each_housing_type();
    houses_demanding_goods = calculate_houses_demanding_goods(housing_type_counts);

    for (int i = 0; i <= 11; i++) {
        if (housing_type_counts[i]) {
            width = lang_text_draw(29, i, 70, y_offset + (20*rows), FONT_NORMAL_GREEN);
            text_draw_number(housing_type_counts[i], '@', " ", 215, y_offset + (20*rows), FONT_NORMAL_WHITE);
            rows += 1;
        }
    }

    rows = 0;

    for (int i = 12; i <= 19; i++) {
        if (housing_type_counts[i]) {
            width = lang_text_draw(29, i, 270, y_offset + (20*rows), FONT_NORMAL_GREEN);
            text_draw_number(housing_type_counts[i], '@', " ", 450, y_offset + (20*rows), FONT_NORMAL_WHITE);
            rows += 1;
        }
    }

    text_draw(translation_for(TR_ADVISOR_TOTAL_NUM_HOUSES), 270, y_offset+180, FONT_NORMAL_GREEN, 0);
    text_draw_number(calculate_total_housing_buildings(), '@', " ", 450, y_offset + 180, FONT_NORMAL_WHITE);

    text_draw(translation_for(TR_ADVISOR_AVAILABLE_HOUSING_CAPACITY), 270, y_offset+200, FONT_NORMAL_GREEN, 0);
    text_draw_number(city_population_open_housing_capacity(), '@', " ", 450, y_offset + 200, FONT_NORMAL_WHITE);

    text_draw(translation_for(TR_ADVISOR_TOTAL_HOUSING_CAPACITY), 270, y_offset+220, FONT_NORMAL_GREEN, 0);
    text_draw_number(city_population_total_housing_capacity(), '@', " ", 450, y_offset + 220, FONT_NORMAL_WHITE);

    for (int i = 0; i <= 3; i++) {
        image_draw(image_group(GROUP_RESOURCE_ICONS) + goods_icons[i], 54, y_offset + 260 + (23*i));
        text_draw(translation_for(goods_demand_strings[i]), 90, y_offset+263+ (23*i), FONT_NORMAL_BLACK, 0);
        text_draw_number(houses_demanding_goods[i], '@', " ", 450, y_offset + 263 + (23*i), FONT_NORMAL_BLACK);
    }

}

static int draw_background(void)
{
    int val;
    int width;

    outer_panel_draw(0, 0, 40, ADVISOR_HEIGHT);
    inner_panel_draw(24, 60, 32, 16);
    image_draw(image_group(GROUP_ADVISOR_ICONS) + 5, 10, 10);
    image_draw(image_group(GROUP_ADVISOR_ICONS) + 5, 555, 265);

    width = text_draw_number(city_population(), '@', " ", 450, 25, FONT_NORMAL_BLACK);
    text_draw(translation_for(TR_ADVISOR_TOTAL_POPULATION), 450+width, 25, FONT_NORMAL_BLACK, 0);

    for (int i = 0; i < 58; i++) {
        val = i/2;
        graphics_draw_vertical_line(545 + i, 260 + 50 - val, 260 + 50, COLOR_RED);
    }

    text_draw(translation_for(TR_ADVISOR_ADVISOR_HEADER_HOUSING), 60, 12, FONT_LARGE_BLACK, 0);
    text_draw_centered(translation_for(TR_ADVISOR_BUTTON_GRAPHS), 545, 315, 61, FONT_NORMAL_BLACK, 0);

    draw_housing_table();

    return ADVISOR_HEIGHT;
}

static void draw_foreground(void)
{
    if (focus_button_id == 0) {
        button_border_draw(545, 260, 60, 51, 0);
    } else if (focus_button_id == 1) {
        button_border_draw(545, 260, 60, 51, 1);
    }
}

static int handle_mouse(const mouse *m)
{
    return generic_buttons_handle_mouse(m, 0, 0, back_button, 1, &focus_button_id);
}

static void go_back(int param1, int param2)
{
    window_advisors_show_advisor(6);
    window_invalidate();
}

static int get_tooltip_text(void) {
    return 0;
}

const advisor_window_type *window_advisor_housing(void)
{
    static const advisor_window_type window = {
        draw_background,
        draw_foreground,
        handle_mouse,
        get_tooltip_text
    };
    return &window;
}
