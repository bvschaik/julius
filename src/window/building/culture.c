#include "culture.h"

#include "building/building.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"

static void draw_culture_info(building_info_context *c, int help_id, const char *sound_file, int group_id)
{
    c->help_id = help_id;
    window_building_play_sound(c, sound_file);
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(group_id, 0, c->x_offset, c->y_offset + 10,
        BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);

    if (!c->has_road_access) {
        window_building_draw_description(c, 69, 25);
    } else if (building_get(c->building_id)->num_workers <= 0) {
        window_building_draw_description(c, group_id, 2);
    } else {
        window_building_draw_description(c, group_id, 3);
    }
    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 142);
}

void window_building_draw_clinic(building_info_context *c)
{
    draw_culture_info(c, 65, "wavs/clinic.wav", 81);
}

void window_building_draw_hospital(building_info_context *c)
{
    draw_culture_info(c, 66, "wavs/hospital.wav", 82);
}

void window_building_draw_bathhouse(building_info_context *c)
{
    c->help_id = 64;
    window_building_play_sound(c, "wavs/baths.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(83, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);

    building *b = building_get(c->building_id);
    if (!b->has_water_access) {
        window_building_draw_description(c, 83, 4);
    } else if (!c->has_road_access) {
        window_building_draw_description(c, 69, 25);
    } else if (b->num_workers <= 0) {
        window_building_draw_description(c, 83, 2);
    } else {
        window_building_draw_description(c, 83, 3);
    }
    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 142);
}

void window_building_draw_barber(building_info_context *c)
{
    draw_culture_info(c, 63, "wavs/barber.wav", 84);
}

void window_building_draw_school(building_info_context *c)
{
    draw_culture_info(c, 68, "wavs/school.wav", 85);
}

void window_building_draw_academy(building_info_context *c)
{
    draw_culture_info(c, 69, "wavs/academy.wav", 86);
}

void window_building_draw_library(building_info_context *c)
{
    draw_culture_info(c, 70, "wavs/library.wav", 87);
}

static void draw_temple(building_info_context *c, const char *sound_file, int group_id, int image_offset)
{
    c->help_id = 67;
    window_building_play_sound(c, sound_file);
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(group_id, 0, c->x_offset, c->y_offset + 12,
        BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    inner_panel_draw(c->x_offset + 16, c->y_offset + 56, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 62);
    if (c->has_road_access) {
        image_draw(image_offset + image_group(GROUP_PANEL_WINDOWS),
            c->x_offset + 190, c->y_offset + BLOCK_SIZE * c->height_blocks - 118);
    } else {
        window_building_draw_description_at(c, BLOCK_SIZE * c->height_blocks - 128, 69, 25);
    }
}

void window_building_draw_temple_ceres(building_info_context *c)
{
    draw_temple(c, "wavs/temple_farm.wav", 92, 21);
}

void window_building_draw_temple_neptune(building_info_context *c)
{
    draw_temple(c, "wavs/temple_ship.wav", 93, 22);
}

void window_building_draw_temple_mercury(building_info_context *c)
{
    draw_temple(c, "wavs/temple_comm.wav", 94, 23);
}

void window_building_draw_temple_mars(building_info_context *c)
{
    draw_temple(c, "wavs/temple_war.wav", 95, 24);
}

void window_building_draw_temple_venus(building_info_context *c)
{
    draw_temple(c, "wavs/temple_love.wav", 96, 25);
}

void window_building_draw_oracle(building_info_context *c)
{
    c->help_id = 67;
    window_building_play_sound(c, "wavs/oracle.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(110, 0, c->x_offset, c->y_offset + 12, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    window_building_draw_description_at(c, BLOCK_SIZE * c->height_blocks - 158, 110, 1);
}

void window_building_draw_theater(building_info_context *c)
{
    c->help_id = 71;
    window_building_play_sound(c, "wavs/theatre.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(72, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    building *b = building_get(c->building_id);
    if (!c->has_road_access) {
        window_building_draw_description(c, 69, 25);
    } else if (b->num_workers <= 0) {
        window_building_draw_description(c, 72, 4);
    } else if (!b->data.entertainment.num_shows) {
        window_building_draw_description(c, 72, 2);
    } else if (b->data.entertainment.days1) {
        window_building_draw_description(c, 72, 3);
    }

    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 6);
    window_building_draw_employment(c, 138);
    if (b->data.entertainment.days1 > 0) {
        int width = lang_text_draw(72, 6, c->x_offset + 32, c->y_offset + 182, FONT_NORMAL_BROWN);
        lang_text_draw_amount(8, 44, 2 * b->data.entertainment.days1,
            c->x_offset + width + 32, c->y_offset + 182, FONT_NORMAL_BROWN);
        lang_text_draw(72, 7 + b->data.entertainment.play,
            c->x_offset + 32, c->y_offset + 202, FONT_NORMAL_BROWN);
    } else {
        lang_text_draw(72, 5, c->x_offset + 32, c->y_offset + 182, FONT_NORMAL_BROWN);
    }
}

void window_building_draw_amphitheater(building_info_context *c)
{
    c->help_id = 72;
    window_building_play_sound(c, "wavs/ampitheatre.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(71, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    building *b = building_get(c->building_id);
    if (!c->has_road_access) {
        window_building_draw_description(c, 69, 25);
    } else if (b->num_workers <= 0) {
        window_building_draw_description(c, 71, 6);
    } else if (!b->data.entertainment.num_shows) {
        window_building_draw_description(c, 71, 2);
    } else if (b->data.entertainment.num_shows == 2) {
        window_building_draw_description(c, 71, 3);
    } else if (b->data.entertainment.days1) {
        window_building_draw_description(c, 71, 4);
    } else if (b->data.entertainment.days2) {
        window_building_draw_description(c, 71, 5);
    }

    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 7);
    window_building_draw_employment(c, 138);
    if (b->data.entertainment.days1 > 0) {
        int width = lang_text_draw(71, 8, c->x_offset + 32, c->y_offset + 182, FONT_NORMAL_BROWN);
        lang_text_draw_amount(8, 44, 2 * b->data.entertainment.days1,
            c->x_offset + width + 32, c->y_offset + 182, FONT_NORMAL_BROWN);
    } else {
        lang_text_draw(71, 7, c->x_offset + 32, c->y_offset + 182, FONT_NORMAL_BROWN);
    }
    if (b->data.entertainment.days2 > 0) {
        int width = lang_text_draw(71, 10, c->x_offset + 32, c->y_offset + 202, FONT_NORMAL_BROWN);
        lang_text_draw_amount(8, 44, 2 * b->data.entertainment.days2,
            c->x_offset + width + 32, c->y_offset + 202, FONT_NORMAL_BROWN);
        lang_text_draw(72, 7 + b->data.entertainment.play,
            c->x_offset + 32, c->y_offset + 222, FONT_NORMAL_BROWN);
    } else {
        lang_text_draw(71, 9, c->x_offset + 32, c->y_offset + 202, FONT_NORMAL_BROWN);
    }
}

void window_building_draw_colosseum(building_info_context *c)
{
    c->help_id = 73;
    window_building_play_sound(c, "wavs/colloseum.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(74, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    building *b = building_get(c->building_id);
    if (!c->has_road_access) {
        window_building_draw_description(c, 69, 25);
    } else if (b->num_workers <= 0) {
        window_building_draw_description(c, 74, 6);
    } else if (!b->data.entertainment.num_shows) {
        window_building_draw_description(c, 74, 2);
    } else if (b->data.entertainment.num_shows == 2) {
        window_building_draw_description(c, 74, 3);
    } else if (b->data.entertainment.days1) {
        window_building_draw_description(c, 74, 5);
    } else if (b->data.entertainment.days2) {
        window_building_draw_description(c, 74, 4);
    }

    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 6);
    window_building_draw_employment(c, 138);
    if (b->data.entertainment.days1 > 0) {
        int width = lang_text_draw(74, 8, c->x_offset + 32, c->y_offset + 182, FONT_NORMAL_BROWN);
        lang_text_draw_amount(8, 44, 2 * b->data.entertainment.days1,
            c->x_offset + width + 32, c->y_offset + 182, FONT_NORMAL_BROWN);
    } else {
        lang_text_draw(74, 7, c->x_offset + 32, c->y_offset + 182, FONT_NORMAL_BROWN);
    }
    if (b->data.entertainment.days2 > 0) {
        int width = lang_text_draw(74, 10, c->x_offset + 32, c->y_offset + 202, FONT_NORMAL_BROWN);
        lang_text_draw_amount(8, 44, 2 * b->data.entertainment.days2,
            c->x_offset + width + 32, c->y_offset + 202, FONT_NORMAL_BROWN);
    } else {
        lang_text_draw(74, 9, c->x_offset + 32, c->y_offset + 202, FONT_NORMAL_BROWN);
    }
}

void window_building_draw_hippodrome(building_info_context *c)
{
    c->help_id = 74;
    window_building_play_sound(c, "wavs/hippodrome.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(73, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    building *b = building_get(c->building_id);
    if (!c->has_road_access) {
        window_building_draw_description(c, 69, 25);
    } else if (b->num_workers <= 0) {
        window_building_draw_description(c, 73, 4);
    } else if (!b->data.entertainment.num_shows) {
        window_building_draw_description(c, 73, 2);
    } else if (b->data.entertainment.days1) {
        window_building_draw_description(c, 73, 3);
    }

    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 6);
    window_building_draw_employment(c, 138);
    if (b->data.entertainment.days1 > 0) {
        int width = lang_text_draw(73, 6, c->x_offset + 32, c->y_offset + 202, FONT_NORMAL_BROWN);
        lang_text_draw_amount(8, 44, 2 * b->data.entertainment.days1,
            c->x_offset + width + 32, c->y_offset + 202, FONT_NORMAL_BROWN);
    } else {
        lang_text_draw(73, 5, c->x_offset + 32, c->y_offset + 202, FONT_NORMAL_BROWN);
    }
}

static void draw_entertainment_school(building_info_context *c, const char *sound_file, int group_id)
{
    c->help_id = 75;
    window_building_play_sound(c, sound_file);

    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(group_id, 0, c->x_offset, c->y_offset + 10,
        BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    if (!c->has_road_access) {
        window_building_draw_description(c, 69, 25);
    } else if (building_get(c->building_id)->num_workers <= 0) {
        window_building_draw_description(c, group_id, 7);
    } else if (c->worker_percentage >= 100) {
        window_building_draw_description(c, group_id, 2);
    } else if (c->worker_percentage >= 75) {
        window_building_draw_description(c, group_id, 3);
    } else if (c->worker_percentage >= 50) {
        window_building_draw_description(c, group_id, 4);
    } else if (c->worker_percentage >= 25) {
        window_building_draw_description(c, group_id, 5);
    } else {
        window_building_draw_description(c, group_id, 6);
    }
    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 142);
}

void window_building_draw_actor_colony(building_info_context *c)
{
    draw_entertainment_school(c, "wavs/art_pit.wav", 77);
}

void window_building_draw_gladiator_school(building_info_context *c)
{
    draw_entertainment_school(c, "wavs/glad_pit.wav", 75);
}

void window_building_draw_lion_house(building_info_context *c)
{
    draw_entertainment_school(c, "wavs/lion_pit.wav", 76);
}

void window_building_draw_chariot_maker(building_info_context *c)
{
    draw_entertainment_school(c, "wavs/char_pit.wav", 78);
}
