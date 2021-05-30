#include "industry.h"

#include "building/building.h"
#include "city/buildings.h"
#include "city/resource.h"
#include "core/calc.h"
#include "figure/figure.h"
#include "game/resource.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"

static void draw_farm(building_info_context *c, int help_id, const char *sound_file, int group_id, int resource)
{
    c->help_id = help_id;
    window_building_play_sound(c, sound_file);

    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    image_draw(image_group(GROUP_RESOURCE_ICONS) + resource, c->x_offset + 10, c->y_offset + 10);
    lang_text_draw_centered(group_id, 0, c->x_offset, c->y_offset + 10,
        BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);

    building *b = building_get(c->building_id);
    int pct_grown = calc_percentage(b->data.industry.progress, 200);
    int width = lang_text_draw(group_id, 2, c->x_offset + 32, c->y_offset + 44, FONT_NORMAL_BLACK);
    width += text_draw_percentage(pct_grown, c->x_offset + 32 + width, c->y_offset + 44, FONT_NORMAL_BLACK);
    lang_text_draw(group_id, 3, c->x_offset + 32 + width, c->y_offset + 44, FONT_NORMAL_BLACK);

    if (!c->has_road_access) {
        window_building_draw_description_at(c, 70, 69, 25);
    } else if (city_resource_is_mothballed(resource)) {
        window_building_draw_description_at(c, 70, group_id, 4);
    } else if (b->data.industry.curse_days_left > 4) {
        window_building_draw_description_at(c, 70, group_id, 11);
    } else if (b->num_workers <= 0) {
        window_building_draw_description_at(c, 70, group_id, 5);
    } else if (c->worker_percentage >= 100) {
        window_building_draw_description_at(c, 70, group_id, 6);
    } else if (c->worker_percentage >= 75) {
        window_building_draw_description_at(c, 70, group_id, 7);
    } else if (c->worker_percentage >= 50) {
        window_building_draw_description_at(c, 70, group_id, 8);
    } else if (c->worker_percentage >= 25) {
        window_building_draw_description_at(c, 70, group_id, 9);
    } else {
        window_building_draw_description_at(c, 70, group_id, 10);
    }

    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 142);
    window_building_draw_description_at(c, BLOCK_SIZE * c->height_blocks - 136, group_id, 1);
}

void window_building_draw_wheat_farm(building_info_context *c)
{
    draw_farm(c, 89, "wavs/wheat_farm.wav", 112, RESOURCE_WHEAT);
}

void window_building_draw_vegetable_farm(building_info_context *c)
{
    draw_farm(c, 90, "wavs/veg_farm.wav", 113, RESOURCE_VEGETABLES);
}

void window_building_draw_fruit_farm(building_info_context *c)
{
    draw_farm(c, 90, "wavs/figs_farm.wav", 114, RESOURCE_FRUIT);
}

void window_building_draw_olive_farm(building_info_context *c)
{
    draw_farm(c, 91, "wavs/olives_farm.wav", 115, RESOURCE_OLIVES);
}

void window_building_draw_vines_farm(building_info_context *c)
{
    draw_farm(c, 91, "wavs/vines_farm.wav", 116, RESOURCE_VINES);
}

void window_building_draw_pig_farm(building_info_context *c)
{
    draw_farm(c, 90, "wavs/meat_farm.wav", 117, RESOURCE_MEAT);
}

static void draw_raw_material(
    building_info_context *c, int help_id, const char *sound_file, int group_id, int resource)
{
    c->help_id = help_id;
    window_building_play_sound(c, sound_file);

    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    image_draw(image_group(GROUP_RESOURCE_ICONS) + resource, c->x_offset + 10, c->y_offset + 10);
    lang_text_draw_centered(group_id, 0, c->x_offset, c->y_offset + 10,
        BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);

    building *b = building_get(c->building_id);
    int pct_done = calc_percentage(b->data.industry.progress, 200);
    int width = lang_text_draw(group_id, 2, c->x_offset + 32, c->y_offset + 44, FONT_NORMAL_BLACK);
    width += text_draw_percentage(pct_done, c->x_offset + 32 + width, c->y_offset + 44, FONT_NORMAL_BLACK);
    lang_text_draw(group_id, 3, c->x_offset + 32 + width, c->y_offset + 44, FONT_NORMAL_BLACK);

    if (!c->has_road_access) {
        window_building_draw_description_at(c, 70, 69, 25);
    } else if (city_resource_is_mothballed(resource)) {
        window_building_draw_description_at(c, 70, group_id, 4);
    } else if (b->num_workers <= 0) {
        window_building_draw_description_at(c, 70, group_id, 5);
    } else if (c->worker_percentage >= 100) {
        window_building_draw_description_at(c, 70, group_id, 6);
    } else if (c->worker_percentage >= 75) {
        window_building_draw_description_at(c, 70, group_id, 7);
    } else if (c->worker_percentage >= 50) {
        window_building_draw_description_at(c, 70, group_id, 8);
    } else if (c->worker_percentage >= 25) {
        window_building_draw_description_at(c, 70, group_id, 9);
    } else {
        window_building_draw_description_at(c, 70, group_id, 10);
    }

    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 142);
    window_building_draw_description_at(c, BLOCK_SIZE * c->height_blocks - 136, group_id, 1);
}

void window_building_draw_marble_quarry(building_info_context *c)
{
    draw_raw_material(c, 95, "wavs/quarry.wav", 118, RESOURCE_MARBLE);
}

void window_building_draw_iron_mine(building_info_context *c)
{
    draw_raw_material(c, 93, "wavs/mine.wav", 119, RESOURCE_IRON);
}

void window_building_draw_timber_yard(building_info_context *c)
{
    draw_raw_material(c, 94, "wavs/timber.wav", 120, RESOURCE_TIMBER);
}

void window_building_draw_clay_pit(building_info_context *c)
{
    draw_raw_material(c, 92, "wavs/clay.wav", 121, RESOURCE_CLAY);
}

static void draw_workshop(
    building_info_context *c, int help_id, const char *sound_file, int group_id, int resource, int input_resource)
{
    c->help_id = help_id;
    window_building_play_sound(c, sound_file);

    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    image_draw(image_group(GROUP_RESOURCE_ICONS) + resource, c->x_offset + 10, c->y_offset + 10);
    lang_text_draw_centered(group_id, 0, c->x_offset, c->y_offset + 10,
        BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);

    building *b = building_get(c->building_id);
    int pct_done = calc_percentage(b->data.industry.progress, 400);
    int width = lang_text_draw(group_id, 2, c->x_offset + 32, c->y_offset + 40, FONT_NORMAL_BLACK);
    width += text_draw_percentage(pct_done, c->x_offset + 32 + width, c->y_offset + 40, FONT_NORMAL_BLACK);
    lang_text_draw(group_id, 3, c->x_offset + 32 + width, c->y_offset + 40, FONT_NORMAL_BLACK);

    image_draw(image_group(GROUP_RESOURCE_ICONS) + input_resource, c->x_offset + 32, c->y_offset + 56);
    width = lang_text_draw(group_id, 12, c->x_offset + 60, c->y_offset + 60, FONT_NORMAL_BLACK);
    if (b->loads_stored < 1) {
        lang_text_draw_amount(8, 10, 0, c->x_offset + 60 + width, c->y_offset + 60, FONT_NORMAL_BLACK);
    } else {
        lang_text_draw_amount(8, 10, b->loads_stored, c->x_offset + 60 + width, c->y_offset + 60, FONT_NORMAL_BLACK);
    }

    if (!c->has_road_access) {
        window_building_draw_description_at(c, 86, 69, 25);
    } else if (city_resource_is_mothballed(resource)) {
        window_building_draw_description_at(c, 86, group_id, 4);
    } else if (b->num_workers <= 0) {
        window_building_draw_description_at(c, 86, group_id, 5);
    } else if (b->loads_stored <= 0) {
        window_building_draw_description_at(c, 86, group_id, 11);
    } else if (c->worker_percentage >= 100) {
        window_building_draw_description_at(c, 86, group_id, 6);
    } else if (c->worker_percentage >= 75) {
        window_building_draw_description_at(c, 86, group_id, 7);
    } else if (c->worker_percentage >= 50) {
        window_building_draw_description_at(c, 86, group_id, 8);
    } else if (c->worker_percentage >= 25) {
        window_building_draw_description_at(c, 86, group_id, 9);
    } else {
        window_building_draw_description_at(c, 86, group_id, 10);
    }

    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 142);
}

void window_building_draw_wine_workshop(building_info_context *c)
{
    draw_workshop(c, 96, "wavs/wine_workshop.wav", 122, RESOURCE_WINE, RESOURCE_VINES);
}

void window_building_draw_oil_workshop(building_info_context *c)
{
    draw_workshop(c, 97, "wavs/oil_workshop.wav", 123, RESOURCE_OIL, RESOURCE_OLIVES);
}

void window_building_draw_weapons_workshop(building_info_context *c)
{
    draw_workshop(c, 98, "wavs/weapons_workshop.wav", 124, RESOURCE_WEAPONS, RESOURCE_IRON);
}

void window_building_draw_furniture_workshop(building_info_context *c)
{
    draw_workshop(c, 99, "wavs/furniture_workshop.wav", 125, RESOURCE_FURNITURE, RESOURCE_TIMBER);
}

void window_building_draw_pottery_workshop(building_info_context *c)
{
    draw_workshop(c, 1, "wavs/pottery_workshop.wav", 126, RESOURCE_POTTERY, RESOURCE_CLAY);
}

void window_building_draw_shipyard(building_info_context *c)
{
    c->help_id = 82;
    window_building_play_sound(c, "wavs/shipyard.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(100, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);

    building *b = building_get(c->building_id);

    if (!c->has_road_access) {
        window_building_draw_description(c, 69, 25);
    } else {
        int pct_done = calc_percentage(b->data.industry.progress, 160);
        int width = lang_text_draw(100, 2, c->x_offset + 32, c->y_offset + 56, FONT_NORMAL_BLACK);
        width += text_draw_percentage(pct_done, c->x_offset + 32 + width, c->y_offset + 56, FONT_NORMAL_BLACK);
        lang_text_draw(100, 3, c->x_offset + 32 + width, c->y_offset + 56, FONT_NORMAL_BLACK);
        if (city_buildings_shipyard_boats_requested()) {
            lang_text_draw_multiline(100, 5, c->x_offset + 32, c->y_offset + 80,
                BLOCK_SIZE * (c->width_blocks - 6), FONT_NORMAL_BLACK);
        } else {
            lang_text_draw_multiline(100, 4, c->x_offset + 32, c->y_offset + 80,
                BLOCK_SIZE * (c->width_blocks - 6), FONT_NORMAL_BLACK);
        }
    }

    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 142);
}

void window_building_draw_wharf(building_info_context *c)
{
    c->help_id = 84;
    window_building_play_sound(c, "wavs/wharf.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(102, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    image_draw(image_group(GROUP_RESOURCE_ICONS) + RESOURCE_MEAT +
        resource_image_offset(RESOURCE_MEAT, RESOURCE_IMAGE_ICON),
        c->x_offset + 10, c->y_offset + 10);

    building *b = building_get(c->building_id);

    if (!c->has_road_access) {
        window_building_draw_description(c, 69, 25);
    } else if (!b->data.industry.fishing_boat_id) {
        window_building_draw_description(c, 102, 2);
    } else {
        int text_id;
        switch (figure_get(b->data.industry.fishing_boat_id)->action_state) {
            case FIGURE_ACTION_191_FISHING_BOAT_GOING_TO_FISH: text_id = 3; break;
            case FIGURE_ACTION_192_FISHING_BOAT_FISHING: text_id = 4; break;
            case FIGURE_ACTION_193_FISHING_BOAT_GOING_TO_WHARF: text_id = 5; break;
            case FIGURE_ACTION_194_FISHING_BOAT_AT_WHARF: text_id = 6; break;
            case FIGURE_ACTION_195_FISHING_BOAT_RETURNING_WITH_FISH: text_id = 7; break;
            default: text_id = 8; break;
        }
        window_building_draw_description(c, 102, text_id);
    }

    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 142);
}
