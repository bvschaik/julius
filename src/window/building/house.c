#include "house.h"

#include "building/building.h"
#include "building/model.h"
#include "core/calc.h"
#include "game/resource.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "map/road_access.h"
#include "window/building/figures.h"

#include "Data/CityInfo.h"

static void draw_vacant_lot(BuildingInfoContext *c)
{
    window_building_prepare_figure_list(c);
    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    lang_text_draw_centered(128, 0, c->xOffset, c->yOffset + 8, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    lang_text_draw_centered(13, 1, c->xOffset, c->yOffset + 16 * c->heightBlocks - 22, 16 * c->widthBlocks, FONT_NORMAL_BLACK);
    window_building_draw_figure_list(c);

    int text_id = 2;
    building *b = building_get(c->buildingId);
    if (map_closest_road_within_radius(b->x, b->y, 1, 2, 0, 0)) {
        text_id = 1;
    }
    window_building_draw_description_at(c, 16 * c->heightBlocks - 113, 128, text_id);
}

static void draw_population_info(BuildingInfoContext *c, int y_offset)
{
    building *b = building_get(c->buildingId);
    image_draw(image_group(GROUP_CONTEXT_ICONS) + 13, c->xOffset + 34, y_offset + 4);
    int width = text_draw_number(b->housePopulation, '@', " ", c->xOffset + 50, y_offset + 14, FONT_SMALL_BLACK);
    width += lang_text_draw(127, 20, c->xOffset + 50 + width, y_offset + 14, FONT_SMALL_BLACK);
    
    if (b->housePopulationRoom < 0) {
        width += text_draw_number(-b->housePopulationRoom, '@', " ", c->xOffset + 50 + width, y_offset + 14, FONT_SMALL_BLACK);
        lang_text_draw(127, 21, c->xOffset + 50 + width, y_offset + 14, FONT_SMALL_BLACK);
    } else if (b->housePopulationRoom > 0) {
        width += lang_text_draw(127, 22, c->xOffset + 50 + width, y_offset + 14, FONT_SMALL_BLACK);
        text_draw_number(b->housePopulationRoom, '@', " ", c->xOffset + 50 + width, y_offset + 14, FONT_SMALL_BLACK);
    }
}

static void draw_tax_info(BuildingInfoContext *c, int y_offset)
{
    building *b = building_get(c->buildingId);
    if (b->houseTaxCoverage) {
        int pct = calc_adjust_with_percentage(b->taxIncomeOrStorage / 2, Data_CityInfo.taxPercentage);
        int width = lang_text_draw(127, 24, c->xOffset + 36, y_offset, FONT_SMALL_BLACK);
        width += lang_text_draw_amount(8, 0, pct, c->xOffset + 36 + width, y_offset, FONT_SMALL_BLACK);
        lang_text_draw(127, 25, c->xOffset + 36 + width, y_offset, FONT_SMALL_BLACK);
    } else {
        lang_text_draw(127, 23, c->xOffset + 36, y_offset, FONT_SMALL_BLACK);
    }
}

static void draw_happiness_info(BuildingInfoContext *c, int y_offset)
{
    int happiness = building_get(c->buildingId)->sentiment.houseHappiness;
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
    lang_text_draw(127, text_id, c->xOffset + 36, y_offset, FONT_SMALL_BLACK);
}

void window_building_draw_house(BuildingInfoContext *c)
{
    c->helpId = 56;
    window_building_play_sound(c, "wavs/housing.wav");
    building *b = building_get(c->buildingId);
    if (b->housePopulation <= 0) {
        draw_vacant_lot(c);
        return;
    }
    int level = b->type - 10;
    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    lang_text_draw_centered(29, level, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    inner_panel_draw(c->xOffset + 16, c->yOffset + 148, c->widthBlocks - 2, 10);
    
    draw_population_info(c, c->yOffset + 154);
    draw_tax_info(c, c->yOffset + 194);
    draw_happiness_info(c, c->yOffset + 214);
    
    int resource_image = image_group(GROUP_RESOURCE_ICONS);
    // food inventory
    if (model_get_house(b->subtype.houseLevel)->food_types) {
        // wheat
        image_draw(resource_image + RESOURCE_WHEAT, c->xOffset + 32, c->yOffset + 234);
        text_draw_number(b->data.house.inventory[INVENTORY_WHEAT], '@', " ", c->xOffset + 64, c->yOffset + 238, FONT_SMALL_BLACK);
        // vegetables
        image_draw(resource_image + RESOURCE_VEGETABLES, c->xOffset + 142, c->yOffset + 234);
        text_draw_number(b->data.house.inventory[INVENTORY_VEGETABLES], '@', " ", c->xOffset + 174, c->yOffset + 238, FONT_SMALL_BLACK);
        // fruit
        image_draw(resource_image + RESOURCE_FRUIT, c->xOffset + 252, c->yOffset + 234);
        text_draw_number(b->data.house.inventory[INVENTORY_FRUIT], '@', " ", c->xOffset + 284, c->yOffset + 238, FONT_SMALL_BLACK);
        // meat/fish
        image_draw(resource_image + RESOURCE_MEAT + resource_image_offset(RESOURCE_MEAT, RESOURCE_IMAGE_ICON),
            c->xOffset + 362, c->yOffset + 234);
        text_draw_number(b->data.house.inventory[INVENTORY_MEAT], '@', " ", c->xOffset + 394, c->yOffset + 238, FONT_SMALL_BLACK);
    } else {
        // no food necessary
        lang_text_draw_multiline(127, 33, c->xOffset + 36, c->yOffset + 234, 16 * (c->widthBlocks - 6), FONT_SMALL_BLACK);
    }
    // goods inventory
    // pottery
    image_draw(resource_image + RESOURCE_POTTERY, c->xOffset + 32, c->yOffset + 274);
    text_draw_number(b->data.house.inventory[INVENTORY_POTTERY], '@', " ", c->xOffset + 64, c->yOffset + 278, FONT_SMALL_BLACK);
    // furniture
    image_draw(resource_image + RESOURCE_FURNITURE, c->xOffset + 142, c->yOffset + 274);
    text_draw_number(b->data.house.inventory[INVENTORY_FURNITURE], '@', " ", c->xOffset + 174, c->yOffset + 278, FONT_SMALL_BLACK);
    // oil
    image_draw(resource_image + RESOURCE_OIL, c->xOffset + 252, c->yOffset + 274);
    text_draw_number(b->data.house.inventory[INVENTORY_OIL], '@', " ", c->xOffset + 284, c->yOffset + 278, FONT_SMALL_BLACK);
    // wine
    image_draw(resource_image + RESOURCE_WINE, c->xOffset + 362, c->yOffset + 274);
    text_draw_number(b->data.house.inventory[INVENTORY_WINE], '@', " ", c->xOffset + 394, c->yOffset + 278, FONT_SMALL_BLACK);
    
    if (b->data.house.evolveTextId == 62) {
        int width = lang_text_draw(127, 40 + b->data.house.evolveTextId, c->xOffset + 32, c->yOffset + 60, FONT_NORMAL_BLACK);
        width += lang_text_draw_colored(41, building_get(c->worstDesirabilityBuildingId)->type,
            c->xOffset + 32 + width, c->yOffset + 60, FONT_NORMAL_PLAIN, COLOR_RED);
        text_draw((uint8_t*)")", c->xOffset + 32 + width, c->yOffset + 60, FONT_NORMAL_BLACK, 0);
        lang_text_draw_multiline(127, 41 + b->data.house.evolveTextId,
            c->xOffset + 32, c->yOffset + 76, 16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
    } else {
        lang_text_draw_multiline(127, 40 + b->data.house.evolveTextId,
            c->xOffset + 32, c->yOffset + 70, 16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
    }
}
