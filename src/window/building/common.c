#include "common.h"

#include "building/building.h"
#include "building/model.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/text.h"
#include "sound/speech.h"

#include "Data/CityInfo.h"

void window_building_draw_employment(BuildingInfoContext *c, int y_offset)
{
    building *b = building_get(c->buildingId);
    int textId;
    if (b->numWorkers >= model_get_building(b->type)->laborers) {
        textId = 0;
    } else if (Data_CityInfo.population <= 0) {
        textId = 16; // no people in city
    } else if (b->housesCovered <= 0) {
        textId = 17; // no employees nearby
    } else if (b->housesCovered < 40) {
        textId = 20; // poor access to employees
    } else if (Data_CityInfo.laborCategory[b->laborCategory].workersAllocated <= 0) {
        textId = 18; // no people allocated
    } else {
        textId = 19; // too few people allocated
    }
    if (!textId && b->housesCovered < 40) {
        textId = 20; // poor access to employees
    }
    y_offset += c->yOffset;
    image_draw(image_group(GROUP_CONTEXT_ICONS) + 14,
        c->xOffset + 40, y_offset + 6);
    if (textId) {
        int width = lang_text_draw_amount(8, 12, b->numWorkers,
            c->xOffset + 60, y_offset + 10, FONT_SMALL_BLACK);
        width += text_draw_number(model_get_building(b->type)->laborers, '(', "",
            c->xOffset + 70 + width, y_offset + 10, FONT_SMALL_BLACK);
        lang_text_draw(69, 0, c->xOffset + 70 + width, y_offset + 10, FONT_SMALL_BLACK);
        lang_text_draw(69, textId, c->xOffset + 70, y_offset + 26, FONT_SMALL_BLACK);
    } else {
        int width = lang_text_draw_amount(8, 12, b->numWorkers,
            c->xOffset + 60, y_offset + 16, FONT_SMALL_BLACK);
        width += text_draw_number(model_get_building(b->type)->laborers, '(', "",
            c->xOffset + 70 + width, y_offset + 16, FONT_SMALL_BLACK);
        lang_text_draw(69, 0, c->xOffset + 70 + width, y_offset + 16, FONT_SMALL_BLACK);
    }
}

void window_building_draw_description(BuildingInfoContext *c, int text_group, int text_id)
{
    lang_text_draw_multiline(text_group, text_id, c->xOffset + 32, c->yOffset + 56,
        16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}

void window_building_draw_description_at(BuildingInfoContext *c, int y_offset, int text_group, int text_id)
{
    lang_text_draw_multiline(text_group, text_id, c->xOffset + 32, c->yOffset + y_offset,
        16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}

void window_building_play_sound(BuildingInfoContext *c, const char *sound_file)
{
    if (c->canPlaySound) {
        sound_speech_play_file(sound_file);
        c->canPlaySound = 0;
    }
}
