#include "common.h"

#include "building/building.h"
#include "building/model.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/text.h"
#include "sound/speech.h"

#include "Data/CityInfo.h"

void window_building_draw_employment(building_info_context *c, int y_offset)
{
    building *b = building_get(c->building_id);
    int text_id;
    if (b->numWorkers >= model_get_building(b->type)->laborers) {
        text_id = 0;
    } else if (Data_CityInfo.population <= 0) {
        text_id = 16; // no people in city
    } else if (b->housesCovered <= 0) {
        text_id = 17; // no employees nearby
    } else if (b->housesCovered < 40) {
        text_id = 20; // poor access to employees
    } else if (Data_CityInfo.laborCategory[b->laborCategory].workersAllocated <= 0) {
        text_id = 18; // no people allocated
    } else {
        text_id = 19; // too few people allocated
    }
    if (!text_id && b->housesCovered < 40) {
        text_id = 20; // poor access to employees
    }
    y_offset += c->y_offset;
    image_draw(image_group(GROUP_CONTEXT_ICONS) + 14,
        c->x_offset + 40, y_offset + 6);
    if (text_id) {
        int width = lang_text_draw_amount(8, 12, b->numWorkers,
            c->x_offset + 60, y_offset + 10, FONT_SMALL_BLACK);
        width += text_draw_number(model_get_building(b->type)->laborers, '(', "",
            c->x_offset + 70 + width, y_offset + 10, FONT_SMALL_BLACK);
        lang_text_draw(69, 0, c->x_offset + 70 + width, y_offset + 10, FONT_SMALL_BLACK);
        lang_text_draw(69, text_id, c->x_offset + 70, y_offset + 26, FONT_SMALL_BLACK);
    } else {
        int width = lang_text_draw_amount(8, 12, b->numWorkers,
            c->x_offset + 60, y_offset + 16, FONT_SMALL_BLACK);
        width += text_draw_number(model_get_building(b->type)->laborers, '(', "",
            c->x_offset + 70 + width, y_offset + 16, FONT_SMALL_BLACK);
        lang_text_draw(69, 0, c->x_offset + 70 + width, y_offset + 16, FONT_SMALL_BLACK);
    }
}

void window_building_draw_description(building_info_context *c, int text_group, int text_id)
{
    lang_text_draw_multiline(text_group, text_id, c->x_offset + 32, c->y_offset + 56,
        16 * (c->width_blocks - 4), FONT_NORMAL_BLACK);
}

void window_building_draw_description_at(building_info_context *c, int y_offset, int text_group, int text_id)
{
    lang_text_draw_multiline(text_group, text_id, c->x_offset + 32, c->y_offset + y_offset,
        16 * (c->width_blocks - 4), FONT_NORMAL_BLACK);
}

void window_building_play_sound(building_info_context *c, const char *sound_file)
{
    if (c->can_play_sound) {
        sound_speech_play_file(sound_file);
        c->can_play_sound = 0;
    }
}
