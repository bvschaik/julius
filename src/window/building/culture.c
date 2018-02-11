#include "culture.h"

#include "building/building.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"

static void draw_culture_info(BuildingInfoContext *c, int help_id, const char *sound_file, int group_id)
{
    c->helpId = help_id;
    window_building_play_sound(c, sound_file);
    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    lang_text_draw_centered(group_id, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

    if (!c->hasRoadAccess) {
        window_building_draw_description(c, 69, 25);
    } else if (building_get(c->buildingId)->numWorkers <= 0) {
        window_building_draw_description(c, group_id, 2);
    } else {
        window_building_draw_description(c, group_id, 3);
    }
    inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
    window_building_draw_employment(c, 142);
}

void window_building_draw_clinic(BuildingInfoContext *c)
{
    draw_culture_info(c, 65, "wavs/clinic.wav", 81);
}

void window_building_draw_hospital(BuildingInfoContext *c)
{
    draw_culture_info(c, 66, "wavs/hospital.wav", 82);
}

void window_building_draw_bathhouse(BuildingInfoContext *c)
{
    c->helpId = 64;
    window_building_play_sound(c, "wavs/baths.wav");
    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    lang_text_draw_centered(83, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

    building *b = building_get(c->buildingId);
    if (!b->hasWaterAccess) {
        window_building_draw_description(c, 83, 4);
    } else if (!c->hasRoadAccess) {
        window_building_draw_description(c, 69, 25);
    } else if (b->numWorkers <= 0) {
        window_building_draw_description(c, 83, 2);
    } else {
        window_building_draw_description(c, 83, 3);
    }
    inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
    window_building_draw_employment(c, 142);
}

void window_building_draw_barber(BuildingInfoContext *c)
{
    draw_culture_info(c, 63, "wavs/barber.wav", 84);
}

void window_building_draw_school(BuildingInfoContext *c)
{
    draw_culture_info(c, 68, "wavs/school.wav", 85);
}

void window_building_draw_academy(BuildingInfoContext *c)
{
    draw_culture_info(c, 69, "wavs/academy.wav", 86);
}

void window_building_draw_library(BuildingInfoContext *c)
{
    draw_culture_info(c, 70, "wavs/library.wav", 87);
}

static void draw_temple(BuildingInfoContext *c, const char *sound_file, int group_id, int image_offset)
{
    c->helpId = 67;
    window_building_play_sound(c, sound_file);
    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    lang_text_draw_centered(group_id, 0, c->xOffset, c->yOffset + 12, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    inner_panel_draw(c->xOffset + 16, c->yOffset + 56, c->widthBlocks - 2, 4);
    window_building_draw_employment(c, 62);
    if (c->hasRoadAccess) {
        image_draw(image_offset + image_group(GROUP_PANEL_WINDOWS),
            c->xOffset + 190, c->yOffset + 16 * c->heightBlocks - 118);
    } else {
        window_building_draw_description_at(c, 16 * c->heightBlocks - 128, 69, 25);
    }
}

void window_building_draw_temple_ceres(BuildingInfoContext *c)
{
    draw_temple(c, "wavs/temple_farm.wav", 92, 21);
}

void window_building_draw_temple_neptune(BuildingInfoContext *c)
{
    draw_temple(c, "wavs/temple_ship.wav", 93, 22);
}

void window_building_draw_temple_mercury(BuildingInfoContext *c)
{
    draw_temple(c, "wavs/temple_comm.wav", 94, 23);
}

void window_building_draw_temple_mars(BuildingInfoContext *c)
{
    draw_temple(c, "wavs/temple_war.wav", 95, 24);
}

void window_building_draw_temple_venus(BuildingInfoContext *c)
{
    draw_temple(c, "wavs/temple_love.wav", 96, 25);
}

void window_building_draw_oracle(BuildingInfoContext *c)
{
    c->helpId = 67;
    window_building_play_sound(c, "wavs/oracle.wav");
    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    lang_text_draw_centered(110, 0, c->xOffset, c->yOffset + 12, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    lang_text_draw_centered(13, 1,
        c->xOffset, c->yOffset + 16 * c->heightBlocks - 24,
        16 * c->widthBlocks, FONT_NORMAL_BLACK);
    window_building_draw_description_at(c, 16 * c->heightBlocks - 143, 110, 1);
}

void window_building_draw_theater(BuildingInfoContext *c)
{
    c->helpId = 71;
    window_building_play_sound(c, "wavs/theatre.wav");
    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    lang_text_draw_centered(72, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    building *b = building_get(c->buildingId);
    if (!c->hasRoadAccess) {
        window_building_draw_description(c, 69, 25);
    } else if (b->numWorkers <= 0) {
        window_building_draw_description(c, 72, 4);
    } else if (!b->data.entertainment.numShows) {
        window_building_draw_description(c, 72, 2);
    } else if (b->data.entertainment.days1) {
        window_building_draw_description(c, 72, 3);
    }

    inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 6);
    window_building_draw_employment(c, 138);
    if (b->data.entertainment.days1 > 0) {
        int width = lang_text_draw(72, 6, c->xOffset + 32, c->yOffset + 182, FONT_SMALL_BLACK);
        lang_text_draw_amount(8, 44, 2 * b->data.entertainment.days1,
            c->xOffset + width + 32, c->yOffset + 182, FONT_SMALL_BLACK);
        lang_text_draw(72, 7 + b->data.entertainment.play,
            c->xOffset + 32, c->yOffset + 202, FONT_SMALL_BLACK);
    } else {
        lang_text_draw(72, 5, c->xOffset + 32, c->yOffset + 182, FONT_SMALL_BLACK);
    }
}

void window_building_draw_amphitheater(BuildingInfoContext *c)
{
    c->helpId = 72;
    window_building_play_sound(c, "wavs/amphitheatre.wav");
    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    lang_text_draw_centered(71, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    building *b = building_get(c->buildingId);
    if (!c->hasRoadAccess) {
        window_building_draw_description(c, 69, 25);
    } else if (b->numWorkers <= 0) {
        window_building_draw_description(c, 71, 6);
    } else if (!b->data.entertainment.numShows) {
        window_building_draw_description(c, 71, 2);
    } else if (b->data.entertainment.numShows == 2) {
        window_building_draw_description(c, 71, 3);
    } else if (b->data.entertainment.days1) {
        window_building_draw_description(c, 71, 4);
    } else if (b->data.entertainment.days2) {
        window_building_draw_description(c, 71, 5);
    }

    inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 7);
    window_building_draw_employment(c, 138);
    if (b->data.entertainment.days1 > 0) {
        int width = lang_text_draw(71, 8, c->xOffset + 32, c->yOffset + 182, FONT_SMALL_BLACK);
        lang_text_draw_amount(8, 44, 2 * b->data.entertainment.days1,
            c->xOffset + width + 32, c->yOffset + 182, FONT_SMALL_BLACK);
    } else {
        lang_text_draw(71, 7, c->xOffset + 32, c->yOffset + 182, FONT_SMALL_BLACK);
    }
    if (b->data.entertainment.days2 > 0) {
        int width = lang_text_draw(71, 10, c->xOffset + 32, c->yOffset + 202, FONT_SMALL_BLACK);
        lang_text_draw_amount(8, 44, 2 * b->data.entertainment.days2,
            c->xOffset + width + 32, c->yOffset + 202, FONT_SMALL_BLACK);
        lang_text_draw(72, 7 + b->data.entertainment.play,
            c->xOffset + 32, c->yOffset + 222, FONT_SMALL_BLACK);
    } else {
        lang_text_draw(71, 9, c->xOffset + 32, c->yOffset + 202, FONT_SMALL_BLACK);
    }
}

void window_building_draw_colosseum(BuildingInfoContext *c)
{
    c->helpId = 73;
    window_building_play_sound(c, "wavs/colloseum.wav");
    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    lang_text_draw_centered(74, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    building *b = building_get(c->buildingId);
    if (!c->hasRoadAccess) {
        window_building_draw_description(c, 69, 25);
    } else if (b->numWorkers <= 0) {
        window_building_draw_description(c, 74, 6);
    } else if (!b->data.entertainment.numShows) {
        window_building_draw_description(c, 74, 2);
    } else if (b->data.entertainment.numShows == 2) {
        window_building_draw_description(c, 74, 3);
    } else if (b->data.entertainment.days1) {
        window_building_draw_description(c, 74, 5);
    } else if (b->data.entertainment.days2) {
        window_building_draw_description(c, 74, 4);
    }

    inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 6);
    window_building_draw_employment(c, 138);
    if (b->data.entertainment.days1 > 0) {
        int width = lang_text_draw(74, 8, c->xOffset + 32, c->yOffset + 182, FONT_SMALL_BLACK);
        lang_text_draw_amount(8, 44, 2 * b->data.entertainment.days1,
            c->xOffset + width + 32, c->yOffset + 182, FONT_SMALL_BLACK);
    } else {
        lang_text_draw(74, 7, c->xOffset + 32, c->yOffset + 182, FONT_SMALL_BLACK);
    }
    if (b->data.entertainment.days2 > 0) {
        int width = lang_text_draw(74, 10, c->xOffset + 32, c->yOffset + 202, FONT_SMALL_BLACK);
        lang_text_draw_amount(8, 44, 2 * b->data.entertainment.days2,
            c->xOffset + width + 32, c->yOffset + 202, FONT_SMALL_BLACK);
    } else {
        lang_text_draw(74, 9, c->xOffset + 32, c->yOffset + 202, FONT_SMALL_BLACK);
    }
}

void window_building_draw_hippodrome(BuildingInfoContext *c)
{
    c->helpId = 74;
    window_building_play_sound(c, "wavs/hippodrome.wav");
    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    lang_text_draw_centered(73, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    building *b = building_get(c->buildingId);
    if (!c->hasRoadAccess) {
        window_building_draw_description(c, 69, 25);
    } else if (b->numWorkers <= 0) {
        window_building_draw_description(c, 73, 4);
    } else if (!b->data.entertainment.numShows) {
        window_building_draw_description(c, 73, 2);
    } else if (b->data.entertainment.days1) {
        window_building_draw_description(c, 73, 3);
    }

    inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 6);
    window_building_draw_employment(c, 138);
    if (b->data.entertainment.days1 > 0) {
        int width = lang_text_draw(73, 6, c->xOffset + 32, c->yOffset + 202, FONT_SMALL_BLACK);
        lang_text_draw_amount(8, 44, 2 * b->data.entertainment.days1,
            c->xOffset + width + 32, c->yOffset + 202, FONT_SMALL_BLACK);
    } else {
        lang_text_draw(73, 5, c->xOffset + 32, c->yOffset + 202, FONT_SMALL_BLACK);
    }
}

static void draw_entertainment_school(BuildingInfoContext *c, const char *sound_file, int group_id)
{
    c->helpId = 75;
    window_building_play_sound(c, sound_file);

    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    lang_text_draw_centered(group_id, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    if (!c->hasRoadAccess) {
        window_building_draw_description(c, 69, 25);
    } else if (building_get(c->buildingId)->numWorkers <= 0) {
        window_building_draw_description(c, group_id, 7);
    } else if (c->workerPercentage >= 100) {
        window_building_draw_description(c, group_id, 2);
    } else if (c->workerPercentage >= 75) {
        window_building_draw_description(c, group_id, 3);
    } else if (c->workerPercentage >= 50) {
        window_building_draw_description(c, group_id, 4);
    } else if (c->workerPercentage >= 25) {
        window_building_draw_description(c, group_id, 5);
    } else {
        window_building_draw_description(c, group_id, 6);
    }
    inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
    window_building_draw_employment(c, 142);
}

void window_building_draw_actor_colony(BuildingInfoContext *c)
{
    draw_entertainment_school(c, "wavs/art_pit.wav", 77);
}

void window_building_draw_gladiator_school(BuildingInfoContext *c)
{
    draw_entertainment_school(c, "wavs/glad_pit.wav", 75);
}

void window_building_draw_lion_house(BuildingInfoContext *c)
{
    draw_entertainment_school(c, "wavs/lion_pit.wav", 76);
}

void window_building_draw_chariot_maker(BuildingInfoContext *c)
{
    draw_entertainment_school(c, "wavs/char_pit.wav", 78);
}
