#include "housing.h"

#include "assets/assets.h"
#include "city/migration.h"
#include "city/population.h"
#include "city/resource.h"
#include "core/string.h"
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

static int housing_advisor_image;

static void draw_housing_table(void)
{
    int *housing_type_counts;
    int *houses_demanding_goods;
    int y_offset = 68;
    int rows = 0;
    translation_key goods_demand_strings[4] = { TR_ADVISOR_RESIDENCES_DEMANDING_POTTERY, TR_ADVISOR_RESIDENCES_DEMANDING_FURNITURE, TR_ADVISOR_RESIDENCES_DEMANDING_OIL, TR_ADVISOR_RESIDENCES_DEMANDING_WINE };
    int goods_icons[4] = { RESOURCE_POTTERY, RESOURCE_FURNITURE, RESOURCE_OIL, RESOURCE_WINE };

    housing_type_counts = calculate_number_of_each_housing_type();
    houses_demanding_goods = calculate_houses_demanding_goods(housing_type_counts);

    int x = 40;

    for (int i = 0; i <= 19; i++) {
        if (housing_type_counts[i]) {
            lang_text_draw(29, i, x, y_offset + (20 * rows), FONT_NORMAL_GREEN);
            text_draw_number(housing_type_counts[i], '@', " ", x + 180, y_offset + (20 * rows), FONT_NORMAL_WHITE, 0);
            if (rows == 11) {
                x += 280;
                rows = 0;
            } else {
                rows++;
            }
        }
    }

    text_draw(translation_for(TR_ADVISOR_TOTAL_NUM_HOUSES), 320, y_offset + 180, FONT_NORMAL_GREEN, 0);
    text_draw_number(calculate_total_housing_buildings(), '@', " ", 500, y_offset + 180, FONT_NORMAL_WHITE, 0);

    text_draw(translation_for(TR_ADVISOR_AVAILABLE_HOUSING_CAPACITY), 320, y_offset + 200, FONT_NORMAL_GREEN, 0);
    text_draw_number(city_population_open_housing_capacity(), '@', " ", 500, y_offset + 200, FONT_NORMAL_WHITE, 0);

    text_draw(translation_for(TR_ADVISOR_TOTAL_HOUSING_CAPACITY), 320, y_offset + 220, FONT_NORMAL_GREEN, 0);
    text_draw_number(city_population_total_housing_capacity(), '@', " ", 500, y_offset + 220, FONT_NORMAL_WHITE, 0);

    for (int i = 0; i <= 3; i++) {
        image_draw(image_group(GROUP_RESOURCE_ICONS) + goods_icons[i], 54, y_offset + 260 + (23 * i),
            COLOR_MASK_NONE, SCALE_NONE);
        text_draw(translation_for(goods_demand_strings[i]), 90, y_offset + 263 + (23 * i), FONT_NORMAL_BLACK, 0);
        text_draw_number(houses_demanding_goods[i], '@', " ", 450, y_offset + 263 + (23 * i), FONT_NORMAL_BLACK, 0);
    }
}

static int draw_background(void)
{
    if (!housing_advisor_image) {
        housing_advisor_image = assets_get_image_id("UI", "Housing Advisor Button");
    }

    outer_panel_draw(0, 0, 40, ADVISOR_HEIGHT);
    inner_panel_draw(24, 60, 37, 16);

    text_draw(translation_for(TR_HEADER_HOUSING), 60, 12, FONT_LARGE_BLACK, 0);
    image_draw(housing_advisor_image, 10, 10, COLOR_MASK_NONE, SCALE_NONE);

    static uint8_t pop[32];
    pop[0] = ' ';
    string_from_int(pop + 1, city_population(), 0);

    int x_offset = text_get_width(pop, FONT_NORMAL_BLACK);
    x_offset += lang_text_get_width(CUSTOM_TRANSLATION, TR_ADVISOR_TOTAL_POPULATION, FONT_NORMAL_BLACK);
    x_offset = 620 - x_offset;

    int width = text_draw_number(city_population(), 0, "", x_offset, 25, FONT_NORMAL_BLACK, 0);
    text_draw(translation_for(TR_ADVISOR_TOTAL_POPULATION), x_offset + width, 25, FONT_NORMAL_BLACK, 0);

    draw_housing_table();

    return ADVISOR_HEIGHT;
}

const advisor_window_type *window_advisor_housing(void)
{
    static const advisor_window_type window = {
        draw_background
    };
    return &window;
}
