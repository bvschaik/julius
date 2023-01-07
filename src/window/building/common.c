#include "common.h"

#include "assets/assets.h"
#include "building/building.h"
#include "building/model.h"
#include "building/monument.h"
#include "city/labor.h"
#include "city/population.h"
#include "city/resource.h"
#include "city/view.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "translation/translation.h"
#include "sound/speech.h"

#include <stdlib.h>
#include <math.h>

void window_building_set_possible_position(int *x_offset, int *y_offset, int width_blocks, int height_blocks)
{
    int dialog_width = BLOCK_SIZE * width_blocks;
    int dialog_height = BLOCK_SIZE * height_blocks;
    int stub;
    int width;
    city_view_get_viewport(&stub, &stub, &width, &stub);
    width -= MARGIN_POSITION;

    if (*y_offset + dialog_height > screen_height() - MARGIN_POSITION) {
        *y_offset -= dialog_height;
    }

    *y_offset = (*y_offset < MIN_Y_POSITION) ? MIN_Y_POSITION : *y_offset;

    if (*x_offset + dialog_width > width) {
        *x_offset = width - dialog_width;
    }
}

int window_building_get_vertical_offset(building_info_context *c, int new_window_height)
{
    new_window_height = new_window_height * BLOCK_SIZE;
    int old_window_height = c->height_blocks * BLOCK_SIZE;
    int y_offset = c->y_offset;

    int center = (old_window_height / 2) + y_offset;
    int new_window_y = center - (new_window_height / 2);

    if (new_window_y < MIN_Y_POSITION) {
        new_window_y = MIN_Y_POSITION;
    } else {
        int height = screen_height() - MARGIN_POSITION;

        if (new_window_y + new_window_height > height) {
            new_window_y = height - new_window_height;
        }
    }

    return new_window_y;
}

static int draw_employment_info(building_info_context *c, building *b, int y_offset, int consider_house_covering)
{
    int text_id;
    if (b->num_workers >= building_get_laborers(b->type)) {
        text_id = 0;
    } else if (city_population() <= 0) {
        text_id = 16; // no people in city
    } else if (!consider_house_covering) {
        text_id = 19;
    } else if (b->houses_covered <= 0) {
        text_id = 17; // no employees nearby
    } else if (b->houses_covered < 40) {
        text_id = 20; // poor access to employees
    } else if (city_labor_category(b->labor_category)->workers_allocated <= 0) {
        text_id = 18; // no people allocated
    } else {
        text_id = 19; // too few people allocated
    }
    if (!text_id && consider_house_covering && b->houses_covered < 40) {
        text_id = 20; // poor access to employees
    }
    return text_id;
}

void window_building_draw_levy(int amount, int x_offset, int y_offset)
{
    image_draw(resource_get_data(RESOURCE_DENARII)->image.icon, x_offset + 300, y_offset + 5, COLOR_MASK_NONE, SCALE_NONE);
    int width = text_draw_money(abs(amount), x_offset + 320, y_offset + 10, FONT_NORMAL_BROWN);
    if (amount > 0) {
        text_draw(translation_for(TR_BUILDING_INFO_MONTHLY_LEVY),
            x_offset + 320 + width, y_offset + 10, FONT_NORMAL_BROWN, 0);
    }
}

void window_building_draw_tourism(building_info_context *c, int x_offset, int y_offset)
{
    building *b = building_get(c->building_id);
    if (b->tourism_income_this_year > 0) {
        //image_draw(image_group(GROUP_RESOURCE_ICONS) + 16, x_offset + 0, y_offset + 5);
        int width = text_draw_money(b->tourism_income_this_year, x_offset + 0, y_offset + 10, FONT_NORMAL_BROWN);
        text_draw(translation_for(TR_WINDOW_BUILDING_TOURISM_ANNUAL),
            x_offset + 0 + width, y_offset + 10, FONT_NORMAL_BROWN, 0);
    } else if (b->tourism_disabled) {
        text_draw(translation_for(TR_WINDOW_BUILDING_TOURISM_DISABLED),
            x_offset + 0, y_offset + 10, FONT_NORMAL_BROWN, 0);
    }
}

static void draw_employment_details(building_info_context *c, building *b, int y_offset, int text_id)
{
    y_offset += c->y_offset;
    image_draw(image_group(GROUP_CONTEXT_ICONS) + 14,
        c->x_offset + 40, y_offset + 6, COLOR_MASK_NONE, SCALE_NONE);

    int levy = building_get_levy(b);
    if (levy) {
        window_building_draw_levy(levy, c->x_offset, y_offset);
    }

    int laborers_needed = building_get_laborers(b->type);
    if (laborers_needed) {
        if (text_id) {
            int width = lang_text_draw_amount(8, 12, b->num_workers,
                c->x_offset + 60, y_offset + 10, FONT_NORMAL_BROWN);
            width += text_draw_number(laborers_needed, '(', "",
                c->x_offset + 70 + width, y_offset + 10, FONT_NORMAL_BROWN, 0);
            lang_text_draw(69, 0, c->x_offset + 70 + width, y_offset + 10, FONT_NORMAL_BROWN);
            lang_text_draw(69, text_id, c->x_offset + 70, y_offset + 26, FONT_NORMAL_BROWN);
        } else {
            int width = lang_text_draw_amount(8, 12, b->num_workers,
                c->x_offset + 60, y_offset + 16, FONT_NORMAL_BROWN);
            width += text_draw_number(laborers_needed, '(', "",
                c->x_offset + 70 + width, y_offset + 16, FONT_NORMAL_BROWN, 0);
            lang_text_draw(69, 0, c->x_offset + 70 + width, y_offset + 16, FONT_NORMAL_BROWN);
        }
    }
}

void window_building_draw_employment(building_info_context *c, int y_offset)
{
    building *b = building_get(c->building_id);
    int text_id = draw_employment_info(c, b, y_offset, 1);
    draw_employment_details(c, b, y_offset, text_id);
}

void window_building_draw_employment_without_house_cover(building_info_context *c, int y_offset)
{
    building *b = building_get(c->building_id);
    int text_id = draw_employment_info(c, b, y_offset, 0);
    draw_employment_details(c, b, y_offset, text_id);
}

void window_building_draw_description(building_info_context *c, int text_group, int text_id)
{
    lang_text_draw_multiline(text_group, text_id, c->x_offset + 32, c->y_offset + 56,
        BLOCK_SIZE * (c->width_blocks - 4), FONT_NORMAL_BLACK);
}

void window_building_draw_description_at(building_info_context *c, int y_offset, int text_group, int text_id)
{
    lang_text_draw_multiline(text_group, text_id, c->x_offset + 32, c->y_offset + y_offset,
        BLOCK_SIZE * (c->width_blocks - 4), FONT_NORMAL_BLACK);
}

void window_building_play_sound(building_info_context *c, const char *sound_file)
{
    if (c->can_play_sound) {
        sound_speech_play_file(sound_file);
        c->can_play_sound = 0;
    }
}

static void window_building_draw_monument_resources_needed(building_info_context *c)
{
    building *b = building_get(c->building_id);
    int y_offset = 95;
    if (building_monument_needs_resources(b)) {
        for (resource_type r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
            int resource_needed_amount = building_monument_resources_needed_for_monument_type(b->type, r,
                b->data.monument.phase);
            if (!resource_needed_amount) {
                continue;
            }
            int resource_delivered_amount = resource_needed_amount - b->resources[r];
            image_draw(resource_get_data(r)->image.icon, c->x_offset + 22, c->y_offset + y_offset,
                COLOR_MASK_NONE, SCALE_NONE);
            int width = text_draw_number(resource_delivered_amount, '@', "/",
                c->x_offset + 54, c->y_offset + y_offset + 5, FONT_NORMAL_BLACK, 0);
            text_draw_number(resource_needed_amount, '@', " ",
                c->x_offset + 44 + width, c->y_offset + +y_offset + 5, FONT_NORMAL_BLACK, 0);
            y_offset += 20;
        }
    } else {
        text_draw_multiline(translation_for(TR_BUILDING_MONUMENT_CONSTRUCTION_ARCHITECT_NEEDED),
            c->x_offset + 22, c->y_offset + 95, BLOCK_SIZE * (c->width_blocks - 4), FONT_NORMAL_BLACK, 0);
    }
}

void window_building_draw_monument_construction_process(building_info_context *c,
    int tr_phase_name, int tr_phase_name_text, int tr_construction_desc)
{
    building *b = building_get(c->building_id);

    if (b->data.monument.phase != MONUMENT_FINISHED) {
        if (!c->has_road_access) {
            window_building_draw_description(c, CUSTOM_TRANSLATION,
                TR_WINDOW_BUILDING_INFO_WARNING_NO_MONUMENT_ROAD_ACCESS);
            text_draw_multiline(translation_for(tr_construction_desc),
                c->x_offset + 22, c->y_offset + 180, 16 * (c->width_blocks - 4), FONT_NORMAL_BLACK, 0);
            return;
        }
        int width = text_draw(translation_for(TR_CONSTRUCTION_PHASE),
            c->x_offset + 22, c->y_offset + 50, FONT_NORMAL_BLACK, 0);
        width += text_draw_number(b->data.monument.phase, '@', "/",
            c->x_offset + 22 + width, c->y_offset + 50, FONT_NORMAL_BLACK, 0);
        width += text_draw_number(building_monument_phases(b->type) - 1, '@', "",
            c->x_offset + 10 + width, c->y_offset + 50, FONT_NORMAL_BLACK, 0);
        text_draw(translation_for(tr_phase_name + b->data.monument.phase - 1),
            c->x_offset + 32 + width, c->y_offset + 50, FONT_NORMAL_BLACK, 0);
        text_draw(translation_for(TR_REQUIRED_RESOURCES), c->x_offset + 22, c->y_offset + 70, FONT_NORMAL_BLACK, 0);
        window_building_draw_monument_resources_needed(c);
        int height = text_draw_multiline(translation_for(tr_phase_name_text + b->data.monument.phase - 1),
            c->x_offset + 22, c->y_offset + 170, BLOCK_SIZE * (c->width_blocks - 4), FONT_NORMAL_BLACK, 0);

        if (building_monument_is_construction_halted(b)) {
            height += text_draw_multiline(translation_for(TR_BUILDING_MONUMENT_CONSTRUCTION_HALTED),
                c->x_offset + 22, c->y_offset + 180 + height, BLOCK_SIZE * (c->width_blocks - 4), FONT_NORMAL_BLACK, 0);
        } else {
            height += text_draw_multiline(translation_for(tr_construction_desc),
                c->x_offset + 22, c->y_offset + 180 + height, BLOCK_SIZE * (c->width_blocks - 4), FONT_NORMAL_BLACK, 0);
        }
        if (c->height_blocks > 26) {
            int phase_offset = b->data.monument.phase % 2;
            image_draw_border(assets_get_image_id("UI", "Large_Banner_Border"),
                c->x_offset + 32, c->y_offset + 196 + height, COLOR_MASK_NONE);
            image_draw(assets_get_image_id("UI", "Const. Banner 01") +
                phase_offset, c->x_offset + 37, c->y_offset + 201 + height, COLOR_MASK_NONE, SCALE_NONE);
        }
    }
}
