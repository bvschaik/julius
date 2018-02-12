#include "utility.h"

#include "building/building.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "map/water_supply.h"

void window_building_draw_engineers_post(BuildingInfoContext *c)
{
    c->helpId = 81;
    window_building_play_sound(c, "wavs/eng_post.wav");
    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    lang_text_draw_centered(104, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

    building *b = building_get(c->buildingId);

    if (!c->hasRoadAccess) {
        window_building_draw_description(c, 69, 25);
    } else if (!b->numWorkers) {
        window_building_draw_description(c, 104, 9);
    } else {
        if (b->figureId) {
            window_building_draw_description(c, 104, 2);
        } else {
            window_building_draw_description(c, 104, 3);
        }
        if (c->workerPercentage >= 100) {
            window_building_draw_description_at(c, 72, 104, 4);
        } else if (c->workerPercentage >= 75) {
            window_building_draw_description_at(c, 72, 104, 5);
        } else if (c->workerPercentage >= 50) {
            window_building_draw_description_at(c, 72, 104, 6);
        } else if (c->workerPercentage >= 25) {
            window_building_draw_description_at(c, 72, 104, 7);
        } else {
            window_building_draw_description_at(c, 72, 104, 8);
        }
    }

    inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
    window_building_draw_employment(c, 142);
}

void window_building_draw_prefect(BuildingInfoContext *c)
{
    c->helpId = 86;
    window_building_play_sound(c, "wavs/prefecture.wav");
    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    lang_text_draw_centered(88, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    lang_text_draw_centered(13, 1, c->xOffset, c->yOffset + 16 * c->heightBlocks - 24, 16 * c->widthBlocks, FONT_NORMAL_BLACK);

    building *b = building_get(c->buildingId);
    if (!c->hasRoadAccess) {
        window_building_draw_description(c, 69, 25);
    } else if (b->numWorkers <= 0) {
        window_building_draw_description(c, 88, 9);
    } else {
        if (b->figureId) {
            window_building_draw_description(c, 88, 2);
        } else {
            window_building_draw_description(c, 88, 3);
        }
        if (c->workerPercentage >= 100) {
            window_building_draw_description_at(c, 72, 88, 4);
        } else if (c->workerPercentage >= 75) {
            window_building_draw_description_at(c, 72, 88, 5);
        } else if (c->workerPercentage >= 50) {
            window_building_draw_description_at(c, 72, 88, 6);
        } else if (c->workerPercentage >= 25) {
            window_building_draw_description_at(c, 72, 88, 7);
        } else {
            window_building_draw_description_at(c, 72, 88, 8);
        }
    }

    inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
    window_building_draw_employment(c, 142);
}

void window_building_draw_burning_ruin(BuildingInfoContext *c)
{
    c->helpId = 0;
    window_building_play_sound(c, "wavs/ruin.wav");
    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    lang_text_draw_centered(111, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

    lang_text_draw(41, c->rubbleBuildingType, c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 173, FONT_NORMAL_BLACK);
    lang_text_draw_multiline(111, 1, c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 143, 16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}

void window_building_draw_rubble(BuildingInfoContext *c)
{
    c->helpId = 0;
    window_building_play_sound(c, "wavs/ruin.wav");
    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    lang_text_draw_centered(140, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

    lang_text_draw(41, c->rubbleBuildingType, c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 173, FONT_NORMAL_BLACK);
    lang_text_draw_multiline(140, 1, c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 143, 16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}

void window_building_draw_reservoir(BuildingInfoContext *c)
{
    c->helpId = 59;
    window_building_play_sound(c, "wavs/resevoir.wav");
    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    lang_text_draw_centered(107, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    lang_text_draw_centered(13, 1, c->xOffset, c->yOffset + 16 * c->heightBlocks - 24, 16 * c->widthBlocks, FONT_NORMAL_BLACK);
    int text_id = building_get(c->buildingId)->hasWaterAccess ? 1 : 3;
    window_building_draw_description_at(c, 16 * c->heightBlocks - 173, 107, text_id);
}

void window_building_draw_aqueduct(BuildingInfoContext *c)
{
    c->helpId = 60;
    window_building_play_sound(c, "wavs/aquaduct.wav");
    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    lang_text_draw_centered(141, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    window_building_draw_description_at(c, 16 * c->heightBlocks - 128, 141, c->aqueductHasWater ? 1 : 2);
}

void window_building_draw_fountain(BuildingInfoContext *c)
{
    c->helpId = 61;
    window_building_play_sound(c, "wavs/fountain.wav");
    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    lang_text_draw_centered(108, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    int text_id;
    building *b = building_get(c->buildingId);
    if (b->hasWaterAccess) {
        if (b->numWorkers > 0) {
            text_id = 1;
        } else {
            text_id = 2;
        }
    } else if (c->hasReservoirPipes) {
        text_id = 2;
    } else {
        text_id = 3;
    }
    window_building_draw_description_at(c, 16 * c->heightBlocks - 126, 108, text_id);
}

void window_building_draw_well(BuildingInfoContext *c)
{
    c->helpId = 62;
    window_building_play_sound(c, "wavs/well.wav");
    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    lang_text_draw_centered(109, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    int well_necessity = map_water_supply_is_well_unnecessary(c->buildingId, 2);
    int text_id = 0;
    if (well_necessity == WELL_NECESSARY) { // well is OK
        text_id = 1;
    } else if (well_necessity == WELL_UNNECESSARY_FOUNTAIN) { // all houses have fountain
        text_id = 2;
    } else if (well_necessity == WELL_UNNECESSARY_NO_HOUSES) { // no houses around
        text_id = 3;
    }
    if (text_id) {
        window_building_draw_description_at(c, 16 * c->heightBlocks - 126, 109, text_id);
    }
}

void window_building_draw_mission_post(BuildingInfoContext *c)
{
    c->helpId = 8;
    window_building_play_sound(c, "wavs/mission.wav");
    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    lang_text_draw_centered(134, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    window_building_draw_description_at(c, 106, 134, 1);
}

static void draw_native(BuildingInfoContext *c, int group_id)
{
    c->helpId = 0;
    window_building_play_sound(c, "wavs/empty_land.wav");
    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    lang_text_draw_centered(group_id, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    window_building_draw_description_at(c, 106, group_id, 1);
}

void window_building_draw_native_hut(BuildingInfoContext *c)
{
    draw_native(c, 131);
}

void window_building_draw_native_meeting(BuildingInfoContext *c)
{
    draw_native(c, 132);
}

void window_building_draw_native_crops(BuildingInfoContext *c)
{
    draw_native(c, 133);
}
