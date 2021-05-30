#include "utility.h"

#include "building/building.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "map/water_supply.h"

void window_building_draw_engineers_post(building_info_context *c)
{
    c->help_id = 81;
    window_building_play_sound(c, "wavs/eng_post.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(104, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);

    building *b = building_get(c->building_id);

    if (!c->has_road_access) {
        window_building_draw_description(c, 69, 25);
    } else if (!b->num_workers) {
        window_building_draw_description(c, 104, 9);
    } else {
        if (b->figure_id) {
            window_building_draw_description(c, 104, 2);
        } else {
            window_building_draw_description(c, 104, 3);
        }
        if (c->worker_percentage >= 100) {
            window_building_draw_description_at(c, 72, 104, 4);
        } else if (c->worker_percentage >= 75) {
            window_building_draw_description_at(c, 72, 104, 5);
        } else if (c->worker_percentage >= 50) {
            window_building_draw_description_at(c, 72, 104, 6);
        } else if (c->worker_percentage >= 25) {
            window_building_draw_description_at(c, 72, 104, 7);
        } else {
            window_building_draw_description_at(c, 72, 104, 8);
        }
    }

    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 142);
}

void window_building_draw_prefect(building_info_context *c)
{
    c->help_id = 86;
    window_building_play_sound(c, "wavs/prefecture.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(88, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);

    building *b = building_get(c->building_id);
    if (!c->has_road_access) {
        window_building_draw_description(c, 69, 25);
    } else if (b->num_workers <= 0) {
        window_building_draw_description(c, 88, 9);
    } else {
        if (b->figure_id) {
            window_building_draw_description(c, 88, 2);
        } else {
            window_building_draw_description(c, 88, 3);
        }
        if (c->worker_percentage >= 100) {
            window_building_draw_description_at(c, 72, 88, 4);
        } else if (c->worker_percentage >= 75) {
            window_building_draw_description_at(c, 72, 88, 5);
        } else if (c->worker_percentage >= 50) {
            window_building_draw_description_at(c, 72, 88, 6);
        } else if (c->worker_percentage >= 25) {
            window_building_draw_description_at(c, 72, 88, 7);
        } else {
            window_building_draw_description_at(c, 72, 88, 8);
        }
    }

    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 142);
}

void window_building_draw_burning_ruin(building_info_context *c)
{
    c->help_id = 0;
    window_building_play_sound(c, "wavs/ruin.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(111, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);

    lang_text_draw(41, c->rubble_building_type,
        c->x_offset + 32, c->y_offset + BLOCK_SIZE * c->height_blocks - 173, FONT_NORMAL_BLACK);
    lang_text_draw_multiline(111, 1, c->x_offset + 32, c->y_offset + BLOCK_SIZE * c->height_blocks - 143,
        BLOCK_SIZE * (c->width_blocks - 4), FONT_NORMAL_BLACK);
}

void window_building_draw_rubble(building_info_context *c)
{
    c->help_id = 0;
    window_building_play_sound(c, "wavs/ruin.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(140, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);

    lang_text_draw(41, c->rubble_building_type,
        c->x_offset + 32, c->y_offset + BLOCK_SIZE * c->height_blocks - 173, FONT_NORMAL_BLACK);
    lang_text_draw_multiline(140, 1, c->x_offset + 32, c->y_offset + BLOCK_SIZE * c->height_blocks - 143,
        BLOCK_SIZE * (c->width_blocks - 4), FONT_NORMAL_BLACK);
}

void window_building_draw_reservoir(building_info_context *c)
{
    c->help_id = 59;
    window_building_play_sound(c, "wavs/resevoir.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(107, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    int text_id = building_get(c->building_id)->has_water_access ? 1 : 3;
    window_building_draw_description_at(c, BLOCK_SIZE * c->height_blocks - 173, 107, text_id);
}

void window_building_draw_aqueduct(building_info_context *c)
{
    c->help_id = 60;
    window_building_play_sound(c, "wavs/aquaduct.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(141, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    window_building_draw_description_at(c, BLOCK_SIZE * c->height_blocks - 144, 141, c->aqueduct_has_water ? 1 : 2);
}

void window_building_draw_fountain(building_info_context *c)
{
    c->help_id = 61;
    window_building_play_sound(c, "wavs/fountain.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(108, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    int text_id;
    building *b = building_get(c->building_id);
    if (b->has_water_access) {
        if (b->num_workers > 0) {
            text_id = 1;
        } else {
            text_id = 2;
        }
    } else if (c->has_reservoir_pipes) {
        text_id = 2;
    } else {
        text_id = 3;
    }
    window_building_draw_description(c, 108, text_id);
    inner_panel_draw(c->x_offset + 16, c->y_offset + 166, c->width_blocks - 2, 4);
    window_building_draw_employment_without_house_cover(c, 172);
}

void window_building_draw_well(building_info_context *c)
{
    c->help_id = 62;
    window_building_play_sound(c, "wavs/well.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(109, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    int well_necessity = map_water_supply_is_well_unnecessary(c->building_id, 2);
    int text_id = 0;
    if (well_necessity == WELL_NECESSARY) { // well is OK
        text_id = 1;
    } else if (well_necessity == WELL_UNNECESSARY_FOUNTAIN) { // all houses have fountain
        text_id = 2;
    } else if (well_necessity == WELL_UNNECESSARY_NO_HOUSES) { // no houses around
        text_id = 3;
    }
    if (text_id) {
        window_building_draw_description_at(c, BLOCK_SIZE * c->height_blocks - 160, 109, text_id);
    }
}

void window_building_draw_mission_post(building_info_context *c)
{
    c->help_id = 8;
    window_building_play_sound(c, "wavs/mission.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(134, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    window_building_draw_description(c, 134, 1);
    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment_without_house_cover(c, 142);
}

static void draw_native(building_info_context *c, int group_id)
{
    c->help_id = 0;
    window_building_play_sound(c, "wavs/empty_land.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(group_id, 0, c->x_offset, c->y_offset + 10,
        BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    window_building_draw_description_at(c, 106, group_id, 1);
}

void window_building_draw_native_hut(building_info_context *c)
{
    draw_native(c, 131);
}

void window_building_draw_native_meeting(building_info_context *c)
{
    draw_native(c, 132);
}

void window_building_draw_native_crops(building_info_context *c)
{
    draw_native(c, 133);
}
