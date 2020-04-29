#include "house.h"

#include "building/building.h"
#include "building/model.h"
#include "city/finance.h"
#include "core/calc.h"
#include "game/resource.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "map/road_access.h"
#include "window/building/figures.h"

static void draw_vacant_lot(building_info_context *c)
{
    window_building_prepare_figure_list(c);
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(128, 0, c->x_offset, c->y_offset + 8, 16 * c->width_blocks, FONT_LARGE_BLACK);
    window_building_draw_figure_list(c);

    int text_id = 2;
    building *b = building_get(c->building_id);
    if (map_closest_road_within_radius(b->x, b->y, 1, 2, 0, 0)) {
        text_id = 1;
    }
    window_building_draw_description_at(c, 16 * c->height_blocks - 113, 128, text_id);
}

static void draw_population_info(building_info_context *c, int y_offset)
{
    building *b = building_get(c->building_id);
    image_draw(image_group(GROUP_CONTEXT_ICONS) + 13, c->x_offset + 34, y_offset + 4);
    int width = text_draw_number(b->house_population, '@', " ", c->x_offset + 50, y_offset + 14, FONT_SMALL_BLACK);
    width += lang_text_draw(127, 20, c->x_offset + 50 + width, y_offset + 14, FONT_SMALL_BLACK);
    
    if (b->house_population_room < 0) {
        width += text_draw_number(-b->house_population_room, '@', " ", c->x_offset + 50 + width, y_offset + 14, FONT_SMALL_BLACK);
        lang_text_draw(127, 21, c->x_offset + 50 + width, y_offset + 14, FONT_SMALL_BLACK);
    } else if (b->house_population_room > 0) {
        width += lang_text_draw(127, 22, c->x_offset + 50 + width, y_offset + 14, FONT_SMALL_BLACK);
        text_draw_number(b->house_population_room, '@', " ", c->x_offset + 50 + width, y_offset + 14, FONT_SMALL_BLACK);
    }
}

static void draw_tax_info(building_info_context *c, int y_offset)
{
    building *b = building_get(c->building_id);
    if (b->house_tax_coverage) {
        int pct = calc_adjust_with_percentage(b->tax_income_or_storage / 2, city_finance_tax_percentage());
        int width = lang_text_draw(127, 24, c->x_offset + 36, y_offset, FONT_SMALL_BLACK);
        width += lang_text_draw_amount(8, 0, pct, c->x_offset + 36 + width, y_offset, FONT_SMALL_BLACK);
        lang_text_draw(127, 25, c->x_offset + 36 + width, y_offset, FONT_SMALL_BLACK);
    } else {
        lang_text_draw(127, 23, c->x_offset + 36, y_offset, FONT_SMALL_BLACK);
    }
}

static void draw_happiness_info(building_info_context *c, int y_offset)
{
    int happiness = building_get(c->building_id)->sentiment.house_happiness;
    int text_id;
    if (happiness >= 50) {
        text_id = 26;
    } else if (happiness >= 40) {
        text_id = 27;
    } else if (happiness >= 30) {
        text_id = 28;
    } else if (happiness >= 20) {
        text_id = 29;
    } else if (happiness >= 10) {
        text_id = 30;
    } else if (happiness >= 1) {
        text_id = 31;
    } else {
        text_id = 32;
    }
    lang_text_draw(127, text_id, c->x_offset + 36, y_offset, FONT_SMALL_BLACK);
}

void window_building_draw_house(building_info_context *c)
{
    c->help_id = 56;
    window_building_play_sound(c, "wavs/housing.wav");
    building *b = building_get(c->building_id);
    if (b->house_population <= 0) {
        draw_vacant_lot(c);
        return;
    }
    int level = b->type - 10;
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(29, level, c->x_offset, c->y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);
    inner_panel_draw(c->x_offset + 16, c->y_offset + 148, c->width_blocks - 2, 10);
    
    draw_population_info(c, c->y_offset + 154);
    draw_tax_info(c, c->y_offset + 194);
    draw_happiness_info(c, c->y_offset + 214);
    
    int resource_image = image_group(GROUP_RESOURCE_ICONS);
    // food inventory
    if (model_get_house(b->subtype.house_level)->food_types) {
        // wheat
        image_draw(resource_image + RESOURCE_WHEAT, c->x_offset + 32, c->y_offset + 234);
        text_draw_number(b->data.house.inventory[INVENTORY_WHEAT], '@', " ", c->x_offset + 64, c->y_offset + 238, FONT_SMALL_BLACK);
        // vegetables
        image_draw(resource_image + RESOURCE_VEGETABLES, c->x_offset + 142, c->y_offset + 234);
        text_draw_number(b->data.house.inventory[INVENTORY_VEGETABLES], '@', " ", c->x_offset + 174, c->y_offset + 238, FONT_SMALL_BLACK);
        // fruit
        image_draw(resource_image + RESOURCE_FRUIT, c->x_offset + 252, c->y_offset + 234);
        text_draw_number(b->data.house.inventory[INVENTORY_FRUIT], '@', " ", c->x_offset + 284, c->y_offset + 238, FONT_SMALL_BLACK);
        // meat/fish
        image_draw(resource_image + RESOURCE_MEAT + resource_image_offset(RESOURCE_MEAT, RESOURCE_IMAGE_ICON),
            c->x_offset + 362, c->y_offset + 234);
        text_draw_number(b->data.house.inventory[INVENTORY_MEAT], '@', " ", c->x_offset + 394, c->y_offset + 238, FONT_SMALL_BLACK);
    } else {
        // no food necessary
        lang_text_draw_multiline(127, 33, c->x_offset + 36, c->y_offset + 234, 16 * (c->width_blocks - 6), FONT_SMALL_BLACK);
    }
    // goods inventory
    // pottery
    image_draw(resource_image + RESOURCE_POTTERY, c->x_offset + 32, c->y_offset + 274);
    text_draw_number(b->data.house.inventory[INVENTORY_POTTERY], '@', " ", c->x_offset + 64, c->y_offset + 278, FONT_SMALL_BLACK);
    // furniture
    image_draw(resource_image + RESOURCE_FURNITURE, c->x_offset + 142, c->y_offset + 274);
    text_draw_number(b->data.house.inventory[INVENTORY_FURNITURE], '@', " ", c->x_offset + 174, c->y_offset + 278, FONT_SMALL_BLACK);
    // oil
    image_draw(resource_image + RESOURCE_OIL, c->x_offset + 252, c->y_offset + 274);
    text_draw_number(b->data.house.inventory[INVENTORY_OIL], '@', " ", c->x_offset + 284, c->y_offset + 278, FONT_SMALL_BLACK);
    // wine
    image_draw(resource_image + RESOURCE_WINE, c->x_offset + 362, c->y_offset + 274);
    text_draw_number(b->data.house.inventory[INVENTORY_WINE], '@', " ", c->x_offset + 394, c->y_offset + 278, FONT_SMALL_BLACK);
    
    if (b->data.house.evolve_text_id == 62) {
        int width = lang_text_draw(127, 40 + b->data.house.evolve_text_id, c->x_offset + 32, c->y_offset + 60, FONT_NORMAL_BLACK);
        width += lang_text_draw_colored(41, building_get(c->worst_desirability_building_id)->type,
            c->x_offset + 32 + width, c->y_offset + 60, FONT_NORMAL_PLAIN, COLOR_FONT_RED);
        text_draw((uint8_t*)")", c->x_offset + 32 + width, c->y_offset + 60, FONT_NORMAL_BLACK, 0);
        lang_text_draw_multiline(127, 41 + b->data.house.evolve_text_id,
            c->x_offset + 32, c->y_offset + 76, 16 * (c->width_blocks - 4), FONT_NORMAL_BLACK);
    } else {
        lang_text_draw_multiline(127, 40 + b->data.house.evolve_text_id,
            c->x_offset + 32, c->y_offset + 70, 16 * (c->width_blocks - 4), FONT_NORMAL_BLACK);
    }
}
