#include "industry.h"

#include "building/building.h"
#include "core/calc.h"
#include "figure/figure.h"
#include "game/resource.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"

#include "Data/CityInfo.h"

static void draw_farm(BuildingInfoContext *c, int help_id, const char *sound_file, int group_id, int resource)
{
    c->helpId = help_id;
    window_building_play_sound(c, sound_file);

    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    image_draw(image_group(GROUP_RESOURCE_ICONS) + resource, c->xOffset + 10, c->yOffset + 10);
    lang_text_draw_centered(group_id, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

    building *b = building_get(c->buildingId);
    int pct_grown = calc_percentage(b->data.industry.progress, 200);
    int width = lang_text_draw(group_id, 2, c->xOffset + 32, c->yOffset + 44, FONT_NORMAL_BLACK);
    width += text_draw_percentage(pct_grown, c->xOffset + 32 + width, c->yOffset + 44, FONT_NORMAL_BLACK);
    lang_text_draw(group_id, 3, c->xOffset + 32 + width, c->yOffset + 44, FONT_NORMAL_BLACK);

    if (!c->hasRoadAccess) {
        window_building_draw_description_at(c, 70, 69, 25);
    } else if (Data_CityInfo.resourceIndustryMothballed[resource]) {
        window_building_draw_description_at(c, 70, group_id, 4);
    } else if (b->data.industry.curseDaysLeft > 4) {
        window_building_draw_description_at(c, 70, group_id, 11);
    } else if (b->numWorkers <= 0) {
        window_building_draw_description_at(c, 70, group_id, 5);
    } else if (c->workerPercentage >= 100) {
        window_building_draw_description_at(c, 70, group_id, 6);
    } else if (c->workerPercentage >= 75) {
        window_building_draw_description_at(c, 70, group_id, 7);
    } else if (c->workerPercentage >= 50) {
        window_building_draw_description_at(c, 70, group_id, 8);
    } else if (c->workerPercentage >= 25) {
        window_building_draw_description_at(c, 70, group_id, 9);
    } else {
        window_building_draw_description_at(c, 70, group_id, 10);
    }

    inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
    window_building_draw_employment(c, 142);
    window_building_draw_description_at(c, 16 * c->heightBlocks - 113, group_id, 1);
}

void window_building_draw_wheat_farm(BuildingInfoContext *c)
{
    draw_farm(c, 89, "wavs/wheat_farm.wav", 112, RESOURCE_WHEAT);
}

void window_building_draw_vegetable_farm(BuildingInfoContext *c)
{
    draw_farm(c, 90, "wavs/veg_farm.wav", 113, RESOURCE_VEGETABLES);
}

void window_building_draw_fruit_farm(BuildingInfoContext *c)
{
    draw_farm(c, 90, "wavs/figs_farm.wav", 114, RESOURCE_FRUIT);
}

void window_building_draw_olive_farm(BuildingInfoContext *c)
{
    draw_farm(c, 91, "wavs/olives_farm.wav", 115, RESOURCE_OLIVES);
}

void window_building_draw_vines_farm(BuildingInfoContext *c)
{
    draw_farm(c, 91, "wavs/vines_farm.wav", 116, RESOURCE_VINES);
}

void window_building_draw_pig_farm(BuildingInfoContext *c)
{
    draw_farm(c, 90, "wavs/meat_farm.wav", 117, RESOURCE_MEAT);
}

static void draw_raw_material(BuildingInfoContext *c, int help_id, const char *sound_file, int group_id, int resource)
{
    c->helpId = help_id;
    window_building_play_sound(c, sound_file);

    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    image_draw(image_group(GROUP_RESOURCE_ICONS) + resource, c->xOffset + 10, c->yOffset + 10);
    lang_text_draw_centered(group_id, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

    building *b = building_get(c->buildingId);
    int pct_done = calc_percentage(b->data.industry.progress, 200);
    int width = lang_text_draw(group_id, 2, c->xOffset + 32, c->yOffset + 44, FONT_NORMAL_BLACK);
    width += text_draw_percentage(pct_done, c->xOffset + 32 + width, c->yOffset + 44, FONT_NORMAL_BLACK);
    lang_text_draw(group_id, 3, c->xOffset + 32 + width, c->yOffset + 44, FONT_NORMAL_BLACK);

    if (!c->hasRoadAccess) {
        window_building_draw_description_at(c, 70, 69, 25);
    } else if (Data_CityInfo.resourceIndustryMothballed[resource]) {
        window_building_draw_description_at(c, 70, group_id, 4);
    } else if (b->numWorkers <= 0) {
        window_building_draw_description_at(c, 70, group_id, 5);
    } else if (c->workerPercentage >= 100) {
        window_building_draw_description_at(c, 70, group_id, 6);
    } else if (c->workerPercentage >= 75) {
        window_building_draw_description_at(c, 70, group_id, 7);
    } else if (c->workerPercentage >= 50) {
        window_building_draw_description_at(c, 70, group_id, 8);
    } else if (c->workerPercentage >= 25) {
        window_building_draw_description_at(c, 70, group_id, 9);
    } else {
        window_building_draw_description_at(c, 70, group_id, 10);
    }

    inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
    window_building_draw_employment(c, 142);
    window_building_draw_description_at(c, 16 * c->heightBlocks - 113, group_id, 1);
}

void window_building_draw_marble_quarry(BuildingInfoContext *c)
{
    draw_raw_material(c, 95, "wavs/quarry.wav", 118, RESOURCE_MARBLE);
}

void window_building_draw_iron_mine(BuildingInfoContext *c)
{
    draw_raw_material(c, 93, "wavs/mine.wav", 119, RESOURCE_IRON);
}

void window_building_draw_timber_yard(BuildingInfoContext *c)
{
    draw_raw_material(c, 94, "wavs/timber.wav", 120, RESOURCE_TIMBER);
}

void window_building_draw_clay_pit(BuildingInfoContext *c)
{
    draw_raw_material(c, 92, "wavs/clay.wav", 121, RESOURCE_CLAY);
}

static void draw_workshop(BuildingInfoContext *c, int help_id, const char *sound_file, int group_id, int resource, int input_resource)
{
    c->helpId = help_id;
    window_building_play_sound(c, sound_file);

    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    image_draw(image_group(GROUP_RESOURCE_ICONS) + resource, c->xOffset + 10, c->yOffset + 10);
    lang_text_draw_centered(group_id, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

    building *b = building_get(c->buildingId);
    int pct_done = calc_percentage(b->data.industry.progress, 400);
    int width = lang_text_draw(group_id, 2, c->xOffset + 32, c->yOffset + 40, FONT_NORMAL_BLACK);
    width += text_draw_percentage(pct_done, c->xOffset + 32 + width, c->yOffset + 40, FONT_NORMAL_BLACK);
    lang_text_draw(group_id, 3, c->xOffset + 32 + width, c->yOffset + 40, FONT_NORMAL_BLACK);

    image_draw(image_group(GROUP_RESOURCE_ICONS) + input_resource, c->xOffset + 32, c->yOffset + 56);
    width = lang_text_draw(group_id, 12, c->xOffset + 60, c->yOffset + 60, FONT_NORMAL_BLACK);
    if (b->loadsStored < 1) {
        lang_text_draw_amount(8, 10, 0, c->xOffset + 60 + width, c->yOffset + 60, FONT_NORMAL_BLACK);
    } else {
        lang_text_draw_amount(8, 10, b->loadsStored, c->xOffset + 60 + width, c->yOffset + 60, FONT_NORMAL_BLACK);
    }

    if (!c->hasRoadAccess) {
        window_building_draw_description_at(c, 86, 69, 25);
    } else if (Data_CityInfo.resourceIndustryMothballed[resource]) {
        window_building_draw_description_at(c, 86, group_id, 4);
    } else if (b->numWorkers <= 0) {
        window_building_draw_description_at(c, 86, group_id, 5);
    } else if (b->loadsStored <= 0) {
        window_building_draw_description_at(c, 86, group_id, 11);
    } else if (c->workerPercentage >= 100) {
        window_building_draw_description_at(c, 86, group_id, 6);
    } else if (c->workerPercentage >= 75) {
        window_building_draw_description_at(c, 86, group_id, 7);
    } else if (c->workerPercentage >= 50) {
        window_building_draw_description_at(c, 86, group_id, 8);
    } else if (c->workerPercentage >= 25) {
        window_building_draw_description_at(c, 86, group_id, 9);
    } else {
        window_building_draw_description_at(c, 86, group_id, 10);
    }

    inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
    window_building_draw_employment(c, 142);
}

void window_building_draw_wine_workshop(BuildingInfoContext *c)
{
    draw_workshop(c, 96, "wavs/wine_workshop.wav", 122, RESOURCE_WINE, RESOURCE_VINES);
}

void window_building_draw_oil_workshop(BuildingInfoContext *c)
{
    draw_workshop(c, 97, "wavs/oil_workshop.wav", 123, RESOURCE_OIL, RESOURCE_OLIVES);
}

void window_building_draw_weapons_workshop(BuildingInfoContext *c)
{
    draw_workshop(c, 98, "wavs/weapons_workshop.wav", 124, RESOURCE_WEAPONS, RESOURCE_IRON);
}

void window_building_draw_furniture_workshop(BuildingInfoContext *c)
{
    draw_workshop(c, 99, "wavs/furniture_workshop.wav", 125, RESOURCE_FURNITURE, RESOURCE_TIMBER);
}

void window_building_draw_pottery_workshop(BuildingInfoContext *c)
{
    draw_workshop(c, 1, "wavs/pottery_workshop.wav", 126, RESOURCE_POTTERY, RESOURCE_CLAY);
}

void window_building_draw_shipyard(BuildingInfoContext *c)
{
    c->helpId = 82;
    window_building_play_sound(c, "wavs/shipyard.wav");
    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    lang_text_draw_centered(100, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

    building *b = building_get(c->buildingId);

    if (!c->hasRoadAccess) {
        window_building_draw_description(c, 69, 25);
    } else {
        int pct_done = calc_percentage(b->data.industry.progress, 160);
        int width = lang_text_draw(100, 2, c->xOffset + 32, c->yOffset + 56, FONT_NORMAL_BLACK);
        width += text_draw_percentage(pct_done, c->xOffset + 32 + width, c->yOffset + 56, FONT_NORMAL_BLACK);
        lang_text_draw(100, 3, c->xOffset + 32 + width, c->yOffset + 56, FONT_NORMAL_BLACK);
        if (Data_CityInfo.shipyardBoatsRequested) {
            lang_text_draw_multiline(100, 5, c->xOffset + 32, c->yOffset + 80, 16 * (c->widthBlocks - 6), FONT_NORMAL_BLACK);
        } else {
            lang_text_draw_multiline(100, 4, c->xOffset + 32, c->yOffset + 80, 16 * (c->widthBlocks - 6), FONT_NORMAL_BLACK);
        }
    }

    inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
    window_building_draw_employment(c, 142);
}

void window_building_draw_wharf(BuildingInfoContext *c)
{
    c->helpId = 84;
    window_building_play_sound(c, "wavs/wharf.wav");
    outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    lang_text_draw_centered(102, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    image_draw(image_group(GROUP_RESOURCE_ICONS) + RESOURCE_MEAT +
        resource_image_offset(RESOURCE_MEAT, RESOURCE_IMAGE_ICON),
        c->xOffset + 10, c->yOffset + 10);

    building *b = building_get(c->buildingId);

    if (!c->hasRoadAccess) {
        window_building_draw_description(c, 69, 25);
    } else if (!b->data.other.boatFigureId) {
        window_building_draw_description(c, 102, 2);
    } else {
        int text_id;
        switch (figure_get(b->data.other.boatFigureId)->actionState) {
            case FIGURE_ACTION_191_FISHING_BOAT_GOING_TO_FISH: text_id = 3; break;
            case FIGURE_ACTION_192_FISHING_BOAT_FISHING: text_id = 4; break;
            case FIGURE_ACTION_193_FISHING_BOAT_GOING_TO_WHARF: text_id = 5; break;
            case FIGURE_ACTION_194_FISHING_BOAT_AT_WHARF: text_id = 6; break;
            case FIGURE_ACTION_195_FISHING_BOAT_RETURNING_WITH_FISH: text_id = 7; break;
            default: text_id = 8; break;
        }
        window_building_draw_description(c, 102, text_id);
    }

    inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
    window_building_draw_employment(c, 142);
}
