#include "housing.h"

#include "assets/assets.h"
#include "building/count.h"
#include "building/house.h"
#include "building/house_population.h"
#include "building/model.h"
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
    int y_offset = 68;
    int rows = 0;

    resource_list list = { 0 };
    for (resource_type r = RESOURCE_MIN_NON_FOOD; r < RESOURCE_MAX_NON_FOOD; r++) {
        if (resource_is_inventory(r)) {
            list.items[list.size++] = r;
        }
    }

    int x = 40;
    int total_residences = 0;
    int houses_using_goods[RESOURCE_MAX] = { 0 };

    for (house_level level = HOUSE_MIN; level <= HOUSE_MAX; level++) {
        int residences_at_level = building_count_active(BUILDING_HOUSE_SMALL_TENT + level);
        if (!residences_at_level) {
            continue;
        }
        total_residences += residences_at_level;

        for (int i = 0; i < list.size; i++) {
            if (model_house_uses_inventory(level, list.items[i])) {
                houses_using_goods[list.items[i]] += residences_at_level;
            }
        }

        lang_text_draw(29, level, x, y_offset + (20 * rows), FONT_NORMAL_GREEN);
        text_draw_number(residences_at_level, '@', " ", x + 180, y_offset + (20 * rows), FONT_NORMAL_WHITE, 0);
        if (rows == 11) {
            x += 280;
            rows = 0;
        } else {
            rows++;
        }
    }

    text_draw(translation_for(TR_ADVISOR_TOTAL_NUM_HOUSES), 320, y_offset + 180, FONT_NORMAL_GREEN, 0);
    text_draw_number(total_residences, '@', " ", 500, y_offset + 180, FONT_NORMAL_WHITE, 0);

    text_draw(translation_for(TR_ADVISOR_AVAILABLE_HOUSING_CAPACITY), 320, y_offset + 200, FONT_NORMAL_GREEN, 0);
    text_draw_number(city_population_open_housing_capacity(), '@', " ", 500, y_offset + 200, FONT_NORMAL_WHITE, 0);

    text_draw(translation_for(TR_ADVISOR_TOTAL_HOUSING_CAPACITY), 320, y_offset + 220, FONT_NORMAL_GREEN, 0);
    text_draw_number(city_population_total_housing_capacity(), '@', " ", 500, y_offset + 220, FONT_NORMAL_WHITE, 0);

    for (int i = 0; i < list.size; i++) {
        image_draw(resource_get_data(list.items[i])->image.icon, 54, y_offset + 260 + (23 * i),
            COLOR_MASK_NONE, SCALE_NONE);
        text_draw(translation_for(TR_ADVISOR_RESIDENCES_USING_POTTERY + i), 90, y_offset + 263 + (23 * i),
            FONT_NORMAL_BLACK, 0);
        text_draw_number(houses_using_goods[list.items[i]], '@', " ", 450, y_offset + 263 + (23 * i),
            FONT_NORMAL_BLACK, 0);
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
    house_population_update_room();
    return &window;
}
