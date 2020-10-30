#include "culture.h"

#include "building/building.h"
#include "building/monument.h"
#include "city/constants.h"
#include "city/finance.h"
#include "city/resource.h"
#include "core/mods.h"
#include "graphics/generic_button.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "sound/speech.h"
#include "translation/translation.h"
#include "window/option_popup.h"
#include "window/building/military.h"

#define GOD_PANTHEON 5
#define MODULE_COST 1000

static void add_module_prompt(int param1, int param2);

static int god_id;

static generic_button add_module_button[] = {
    {0, 0, 304, 20, add_module_prompt, button_none, 0, 0}
};

static struct {
    int focus_button_id;
    int building_id;
} data = { 0, 0 };

option_menu_item temple_module_options[12] = {
   {TR_BUILDING_GRAND_TEMPLE_CERES_DESC_MODULE_1, TR_BUILDING_GRAND_TEMPLE_CERES_MODULE_1_DESC, 0, "Areldir", "UI_Elements", "Ceres M Icon"},
   {TR_BUILDING_GRAND_TEMPLE_CERES_DESC_MODULE_2, TR_BUILDING_GRAND_TEMPLE_CERES_MODULE_2_DESC, 0, "Areldir", "UI_Elements", "Ceres M2 Icon"},
   {TR_BUILDING_GRAND_TEMPLE_NEPTUNE_DESC_MODULE_1, TR_BUILDING_GRAND_TEMPLE_NEPTUNE_MODULE_1_DESC, 0, "Areldir", "UI_Elements", "Nept M2 Icon"},
   {TR_BUILDING_GRAND_TEMPLE_NEPTUNE_DESC_MODULE_2, TR_BUILDING_GRAND_TEMPLE_NEPTUNE_MODULE_2_DESC, 0, "Areldir", "UI_Elements", "Nept M Icon"},
   {TR_BUILDING_GRAND_TEMPLE_MERCURY_DESC_MODULE_1, TR_BUILDING_GRAND_TEMPLE_MERCURY_MODULE_1_DESC, 0, "Areldir", "UI_Elements", "Merc M Icon"},
   {TR_BUILDING_GRAND_TEMPLE_MERCURY_DESC_MODULE_2, TR_BUILDING_GRAND_TEMPLE_MERCURY_MODULE_2_DESC, 0, "Areldir", "UI_Elements", "Merc M2 Icon"},
   {TR_BUILDING_GRAND_TEMPLE_MARS_DESC_MODULE_1, TR_BUILDING_GRAND_TEMPLE_MARS_MODULE_1_DESC, 0, "Areldir", "UI_Elements", "Mars M2 Icon"},
   {TR_BUILDING_GRAND_TEMPLE_MARS_DESC_MODULE_2, TR_BUILDING_GRAND_TEMPLE_MARS_MODULE_2_DESC, 0, "Areldir", "UI_Elements", "Mars M Icon"},
   {TR_BUILDING_GRAND_TEMPLE_VENUS_DESC_MODULE_1, TR_BUILDING_GRAND_TEMPLE_VENUS_MODULE_1_DESC, 0, "Areldir", "UI_Elements", "Venus M Icon"},
   {TR_BUILDING_GRAND_TEMPLE_VENUS_DESC_MODULE_2, TR_BUILDING_GRAND_TEMPLE_VENUS_MODULE_2_DESC, 0, "Areldir", "UI_Elements", "Venus M2 Icon"},
   {TR_BUILDING_PANTHEON_DESC_MODULE_1, TR_BUILDING_PANTHEON_MODULE_1_DESC, 0, "Areldir", "UI_Elements", "Panth M Icon"},
   {TR_BUILDING_PANTHEON_DESC_MODULE_2, TR_BUILDING_PANTHEON_MODULE_2_DESC, 0, "Areldir", "UI_Elements", "Panth M2 Icon"}
};

static void draw_culture_info(building_info_context *c, int help_id, const char *sound_file, int group_id)
{
    c->help_id = help_id;
    window_building_play_sound(c, sound_file);
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(group_id, 0, c->x_offset, c->y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);

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
    lang_text_draw_centered(83, 0, c->x_offset, c->y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);

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

static void draw_temple(building_info_context *c, const char *sound_file, int group_id)
{
    c->help_id = 67;
    window_building_play_sound(c, sound_file);
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(group_id, 0, c->x_offset, c->y_offset + 12, 16 * c->width_blocks, FONT_LARGE_BLACK);
    inner_panel_draw(c->x_offset + 16, c->y_offset + 146, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 152);
}

static void draw_temple_info(building_info_context* c, int image_offset) {
    if (!c->has_road_access) {
        window_building_draw_description_at(c, 16 * c->height_blocks - 128, 69, 25);
        return;
    }
    
    building* b = building_get(c->building_id);
    if (building_is_ceres_temple(b->type) && building_monument_gt_module_is_active(CERES_MODULE_2_DISTRIBUTE_FOOD)) {
        building* b = building_get(c->building_id);
        int food = city_resource_ceres_temple_food();
        image_draw(image_group(GROUP_RESOURCE_ICONS) + food, c->x_offset + 112, c->y_offset + 60);
        text_draw_number(b->data.market.inventory[food - 1], '@', " ",
            c->x_offset + 132, c->y_offset + 60, FONT_NORMAL_BLACK);       
        text_draw_multiline(translation_for(TR_BUILDING_CERES_TEMPLE_MODULE_DESC), c->x_offset + 112, c->y_offset + 90, 16 * c->width_blocks - 132, FONT_NORMAL_BLACK, 0);
        image_draw(image_offset + image_group(GROUP_PANEL_WINDOWS),
            c->x_offset + 16, c->y_offset + 16 * c->height_blocks - 208);
        return;
    }

    if (building_is_venus_temple(b->type) && building_monument_gt_module_is_active(VENUS_MODULE_1_DISTRIBUTE_WINE)) {
        building* b = building_get(c->building_id);
        image_draw(image_group(GROUP_RESOURCE_ICONS) + RESOURCE_WINE, c->x_offset + 112, c->y_offset + 60);
        text_draw_number(b->data.market.inventory[INVENTORY_WINE], '@', " ",
            c->x_offset + 132, c->y_offset + 60, FONT_NORMAL_BLACK);
        text_draw_multiline(translation_for(TR_BUILDING_VENUS_TEMPLE_MODULE_DESC), c->x_offset + 112, c->y_offset + 90, 16 * c->width_blocks - 132, FONT_NORMAL_BLACK, 0);
        image_draw(image_offset + image_group(GROUP_PANEL_WINDOWS),
            c->x_offset + 16, c->y_offset + 16 * c->height_blocks - 208);

        return;
    }

    if (building_is_mars_temple(b->type) && building_monument_gt_module_is_active(MARS_MODULE_1_MESS_HALL)) {
        building* b = building_get(c->building_id);

        image_draw(image_group(GROUP_RESOURCE_ICONS) + RESOURCE_WHEAT, c->x_offset + 112, c->y_offset + 60);
        text_draw_number(b->data.market.inventory[INVENTORY_WHEAT], '@', " ", c->x_offset + 144, c->y_offset + 66, FONT_NORMAL_BLACK);

        image_draw(image_group(GROUP_RESOURCE_ICONS) + RESOURCE_VEGETABLES, c->x_offset + 202, c->y_offset + 60);
        text_draw_number(b->data.market.inventory[INVENTORY_VEGETABLES], '@', " ", c->x_offset + 234, c->y_offset + 66, FONT_NORMAL_BLACK);

        image_draw(image_group(GROUP_RESOURCE_ICONS) + RESOURCE_FRUIT, c->x_offset + 292, c->y_offset + 60);
        text_draw_number(b->data.market.inventory[INVENTORY_FRUIT], '@', " ",c->x_offset + 324, c->y_offset + 66, FONT_NORMAL_BLACK);

        image_draw(image_group(GROUP_RESOURCE_ICONS) + RESOURCE_MEAT + resource_image_offset(RESOURCE_MEAT, RESOURCE_IMAGE_ICON),
            c->x_offset + 372, c->y_offset + 60);
        text_draw_number(b->data.market.inventory[INVENTORY_MEAT], '@', " ", c->x_offset + 404, c->y_offset + 66, FONT_NORMAL_BLACK);

        text_draw_multiline(translation_for(TR_BUILDING_MARS_TEMPLE_MODULE_DESC), c->x_offset + 112, c->y_offset + 90, 
            16 * c->width_blocks - 132, FONT_NORMAL_BLACK, 0);
        image_draw(image_offset + image_group(GROUP_PANEL_WINDOWS),
            c->x_offset + 16, c->y_offset + 16 * c->height_blocks - 208);

        return;
    }

    image_draw(image_offset + image_group(GROUP_PANEL_WINDOWS),
        c->x_offset + 180, c->y_offset + 16 * c->height_blocks - 208);

}

void window_building_draw_temple_ceres(building_info_context *c)
{
    draw_temple(c, "wavs/temple_farm.wav", 92);
    draw_temple_info(c, 21);
}

void window_building_draw_temple_neptune(building_info_context *c)
{
    draw_temple(c, "wavs/temple_ship.wav", 93);
    draw_temple_info(c, 22);
}

void window_building_draw_temple_mercury(building_info_context *c)
{
    draw_temple(c, "wavs/temple_comm.wav", 94);
    draw_temple_info(c, 23);
}

void window_building_draw_temple_mars(building_info_context *c)
{
    draw_temple(c, "wavs/temple_war.wav", 95);
    draw_temple_info(c, 24);
}

void window_building_draw_temple_venus(building_info_context *c)
{
    draw_temple(c, "wavs/temple_love.wav", 96);
    draw_temple_info(c, 25);
}

void window_building_draw_oracle(building_info_context *c)
{
    c->help_id = 67;
    window_building_play_sound(c, "wavs/oracle.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(110, 0, c->x_offset, c->y_offset + 12, 16 * c->width_blocks, FONT_LARGE_BLACK);
    window_building_draw_description_at(c, 16 * c->height_blocks - 178, 110, 1);
    inner_panel_draw(c->x_offset + 16, c->y_offset + 146, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 152);

}

void window_building_draw_theater(building_info_context *c)
{
    c->help_id = 71;
    window_building_play_sound(c, "wavs/theatre.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(72, 0, c->x_offset, c->y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);
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
        int width = lang_text_draw(72, 6, c->x_offset + 32, c->y_offset + 182, FONT_SMALL_BLACK);
        lang_text_draw_amount(8, 44, 2 * b->data.entertainment.days1,
            c->x_offset + width + 32, c->y_offset + 182, FONT_SMALL_BLACK);
        lang_text_draw(72, 7 + b->data.entertainment.play,
            c->x_offset + 32, c->y_offset + 202, FONT_SMALL_BLACK);
    } else {
        lang_text_draw(72, 5, c->x_offset + 32, c->y_offset + 182, FONT_SMALL_BLACK);
    }
}

void window_building_draw_amphitheater(building_info_context *c)
{
    c->help_id = 72;
    // The file name has a typo
    window_building_play_sound(c, "wavs/ampitheatre.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(71, 0, c->x_offset, c->y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);
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
        int width = lang_text_draw(71, 8, c->x_offset + 32, c->y_offset + 182, FONT_SMALL_BLACK);
        lang_text_draw_amount(8, 44, 2 * b->data.entertainment.days1,
            c->x_offset + width + 32, c->y_offset + 182, FONT_SMALL_BLACK);
    } else {
        lang_text_draw(71, 7, c->x_offset + 32, c->y_offset + 182, FONT_SMALL_BLACK);
    }
    if (b->data.entertainment.days2 > 0) {
        int width = lang_text_draw(71, 10, c->x_offset + 32, c->y_offset + 202, FONT_SMALL_BLACK);
        lang_text_draw_amount(8, 44, 2 * b->data.entertainment.days2,
            c->x_offset + width + 32, c->y_offset + 202, FONT_SMALL_BLACK);
        lang_text_draw(72, 7 + b->data.entertainment.play,
            c->x_offset + 32, c->y_offset + 222, FONT_SMALL_BLACK);
    } else {
        lang_text_draw(71, 9, c->x_offset + 32, c->y_offset + 202, FONT_SMALL_BLACK);
    }
}

void window_building_draw_colosseum(building_info_context *c)
{
    c->help_id = 73;
    window_building_play_sound(c, "wavs/colloseum.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(74, 0, c->x_offset, c->y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);
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
        int width = lang_text_draw(74, 8, c->x_offset + 32, c->y_offset + 182, FONT_SMALL_BLACK);
        lang_text_draw_amount(8, 44, 2 * b->data.entertainment.days1,
            c->x_offset + width + 32, c->y_offset + 182, FONT_SMALL_BLACK);
    } else {
        lang_text_draw(74, 7, c->x_offset + 32, c->y_offset + 182, FONT_SMALL_BLACK);
    }
    if (b->data.entertainment.days2 > 0) {
        int width = lang_text_draw(74, 10, c->x_offset + 32, c->y_offset + 202, FONT_SMALL_BLACK);
        lang_text_draw_amount(8, 44, 2 * b->data.entertainment.days2,
            c->x_offset + width + 32, c->y_offset + 202, FONT_SMALL_BLACK);
    } else {
        lang_text_draw(74, 9, c->x_offset + 32, c->y_offset + 202, FONT_SMALL_BLACK);
    }
}

void window_building_draw_hippodrome(building_info_context *c)
{
    c->help_id = 74;
    window_building_play_sound(c, "wavs/hippodrome.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(73, 0, c->x_offset, c->y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);
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
        int width = lang_text_draw(73, 6, c->x_offset + 32, c->y_offset + 202, FONT_SMALL_BLACK);
        lang_text_draw_amount(8, 44, 2 * b->data.entertainment.days1,
            c->x_offset + width + 32, c->y_offset + 202, FONT_SMALL_BLACK);
    } else {
        lang_text_draw(73, 5, c->x_offset + 32, c->y_offset + 202, FONT_SMALL_BLACK);
    }
}

static void draw_entertainment_school(building_info_context *c, const char *sound_file, int group_id)
{
    c->help_id = 75;
    window_building_play_sound(c, sound_file);

    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(group_id, 0, c->x_offset, c->y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);
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

static void window_building_draw_monument_resources_needed(building_info_context* c) {
    building* b = building_get(c->building_id);
    for (int r = RESOURCE_TIMBER; r <= RESOURCE_MARBLE; r++) {
        int total_resources_needed = building_monument_resources_needed_for_monument_type(b->type, r, b->subtype.monument_phase);
        int resources_delivered = total_resources_needed - b->data.monument.resources_needed[r];
        int image_id = image_group(GROUP_RESOURCE_ICONS);
        image_draw(image_id + r, c->x_offset + 22, c->y_offset - 105 + r * 20);
        int width = text_draw_number(resources_delivered, '@', "/", c->x_offset + 54, c->y_offset + 10 + r * 20 - 106, FONT_NORMAL_BLACK);
        text_draw_number(total_resources_needed, '@', " ", c->x_offset + 44 + width, c->y_offset + 10 + r * 20 - 106, FONT_NORMAL_BLACK);

    }
}

static void window_building_draw_monument_construction_process(building_info_context* c, int tr_phase_name, int tr_phase_name_text, int tr_construction_desc) {
    building* b = building_get(c->building_id);
    if (b->subtype.monument_phase != MONUMENT_FINISHED) {
        int width = text_draw(translation_for(TR_CONSTRUCTION_PHASE), c->x_offset + 22, c->y_offset + 50, FONT_NORMAL_BLACK, 0);
        width += text_draw_number(b->subtype.monument_phase, '@', "/", c->x_offset + 22 + width, c->y_offset + 50, FONT_NORMAL_BLACK);
        width += text_draw_number(building_monument_monument_phases(b->type) - 1, '@', "", c->x_offset + 10 + width, c->y_offset + 50, FONT_NORMAL_BLACK);
        text_draw(translation_for(tr_phase_name + b->subtype.monument_phase - 1), c->x_offset + 32 + width, c->y_offset + 50, FONT_NORMAL_BLACK, 0);
        text_draw(translation_for(TR_REQUIRED_RESOURCES), c->x_offset + 22, c->y_offset + 70, FONT_NORMAL_BLACK, 0);
        window_building_draw_monument_resources_needed(c);
        int height = text_draw_multiline(translation_for(tr_phase_name_text + b->subtype.monument_phase - 1), c->x_offset + 22, c->y_offset + 170, 16 * (c->width_blocks - 4), FONT_NORMAL_BLACK, 0);
        height += text_draw_multiline(translation_for(tr_construction_desc), c->x_offset + 22, c->y_offset + 180 + height, 16 * (c->width_blocks - 4), FONT_NORMAL_BLACK, 0);
        if (c->height_blocks > 26) {
            int phase_offset = b->subtype.monument_phase % 2;
            image_draw(mods_get_image_id(mods_get_group_id("Areldir", "UI_Elements"), "Const. Banner 01") + phase_offset, c->x_offset + 32, c->y_offset + 196 + height);
        }
    }
}

static void window_building_draw_monument_temple_construction_process(building_info_context* c) {
    window_building_draw_monument_construction_process(c, TR_BUILDING_GRAND_TEMPLE_PHASE_1, TR_BUILDING_GRAND_TEMPLE_PHASE_1_TEXT, TR_BUILDING_GRAND_TEMPLE_CONSTRUCTION_DESC);
}

static void window_building_draw_monument_lighthouse_construction_process(building_info_context* c) {
    window_building_draw_monument_construction_process(c, TR_BUILDING_LIGHTHOUSE_PHASE_1, TR_BUILDING_LIGHTHOUSE_PHASE_1_TEXT, TR_BUILDING_LIGHTHOUSE_CONSTRUCTION_DESC);
}

void draw_grand_temple_venus_wine(building_info_context* c) {
    int y = 50;
    data.building_id = c->building_id;
    image_draw(image_group(GROUP_RESOURCE_ICONS) + RESOURCE_WINE, c->x_offset + 24, c->y_offset + y - 5);
    building* b = building_get(c->building_id);
    if (b->loads_stored < 1) {
        lang_text_draw_amount(8, 10, 0, c->x_offset + 52, c->y_offset + y, FONT_NORMAL_BLACK);
    }
    else {
        lang_text_draw_amount(8, 10, b->loads_stored, c->x_offset + 52, c->y_offset + y, FONT_NORMAL_BLACK);
    }

}

void draw_grand_temple_mars_military(building_info_context* c)
{
    int y = 60;
    data.building_id = c->building_id;
    image_draw(image_group(GROUP_RESOURCE_ICONS) + RESOURCE_WEAPONS, c->x_offset + 24, c->y_offset + y - 5);
    building* b = building_get(c->building_id);
    if (b->loads_stored < 1) {
        lang_text_draw_amount(8, 10, 0, c->x_offset + 52, c->y_offset + y, FONT_NORMAL_BLACK);
    }
    else {
        lang_text_draw_amount(8, 10, b->loads_stored, c->x_offset + 52, c->y_offset + y, FONT_NORMAL_BLACK);
    }

    lang_text_draw(50, 21, c->x_offset + 236, c->y_offset + y, FONT_NORMAL_BLACK); // "Priority"
    lang_text_draw(91, 0, c->x_offset + 326, c->y_offset + y, FONT_NORMAL_BLACK); // "Tower"
    lang_text_draw(89, 0, c->x_offset + 326, c->y_offset + y + 20, FONT_NORMAL_BLACK); // "Fort"

    window_building_draw_priority_buttons(c->x_offset + 285, c->y_offset + 55);
}

static void draw_grand_temple(building_info_context* c, const char* sound_file, int name, int bonus_desc,int banner_id, int quote, int temple_god_id, int extra_y)
{
    building* b = building_get(c->building_id);
    window_building_play_sound(c, sound_file);
    god_id = temple_god_id;
    if (b->subtype.monument_phase == MONUMENT_FINISHED) {
        outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    } 
    else {
        outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
        window_building_draw_monument_temple_construction_process(c);
    }
    if (b->data.monument.upgrades) {
        int module_name = temple_module_options[god_id * 2 + (b->data.monument.upgrades - 1)].header;
        text_draw_centered(translation_for(module_name), c->x_offset, c->y_offset + 12, 16 * c->width_blocks, FONT_LARGE_BLACK, 0);
    }
    else {
        text_draw_centered(translation_for(name), c->x_offset, c->y_offset + 12, 16 * c->width_blocks, FONT_LARGE_BLACK, 0);
    }
    
    if (b->subtype.monument_phase == MONUMENT_FINISHED) {
        int height = text_draw_multiline(translation_for(bonus_desc), c->x_offset + 22, c->y_offset + 56 + extra_y, 15 * c->width_blocks, FONT_NORMAL_BLACK, 0);
        if (b->data.monument.upgrades) {
            int module_desc = temple_module_options[god_id * 2 + (b->data.monument.upgrades - 1)].desc;
            height += text_draw_multiline(translation_for(module_desc), c->x_offset + 22, c->y_offset + 66 + height + extra_y, 15 * c->width_blocks, FONT_NORMAL_BLACK, 0);
        }
        if (b->type == BUILDING_GRAND_TEMPLE_MARS) {
            draw_grand_temple_mars_military(c);
        } else if (b->type == BUILDING_GRAND_TEMPLE_VENUS && (building_monument_gt_module_is_active(VENUS_MODULE_1_DISTRIBUTE_WINE))) {
            draw_grand_temple_venus_wine(c);
        }
        inner_panel_draw(c->x_offset + 16, c->y_offset + 86 + height + extra_y, c->width_blocks - 2, 4);
        window_building_draw_employment(c, 96 + height + extra_y);
        if (c->height_blocks > 26) {
            image_draw(banner_id, c->x_offset + 32, c->y_offset + 166 + height + extra_y);
            text_draw_centered_with_linebreaks(translation_for(quote), c->x_offset, c->y_offset + 386 + height + extra_y, 16 * c->width_blocks, FONT_NORMAL_BLACK, 0);
        }
    }
}

void window_building_draw_grand_temple_foreground(building_info_context* c)
{
    building* b = building_get(c->building_id);
    if (b->subtype.monument_phase == MONUMENT_FINISHED && !b->data.monument.upgrades) {
        button_border_draw(c->x_offset + 80, c->y_offset + 16 * c->height_blocks - 34,
            16 * (c->width_blocks - 10), 20, data.focus_button_id == 1 ? 1 : 0);
        text_draw_centered(translation_for(TR_BUILDING_GRAND_TEMPLE_ADD_MODULE), c->x_offset + 80, c->y_offset + 16 * c->height_blocks - 30,
            16 * (c->width_blocks - 10), FONT_NORMAL_BLACK, 0);
    }
}

int window_building_handle_mouse_grand_temple(const mouse* m, building_info_context* c)
{
    building* b = building_get(c->building_id);
	data.building_id = c->building_id;
    if (b->subtype.monument_phase != MONUMENT_FINISHED || b->data.monument.upgrades) {
        return 0;
    }
	if (generic_buttons_handle_mouse(m, c->x_offset + 80, c->y_offset + 16 * c->height_blocks - 34,
		add_module_button, 1, &data.focus_button_id)) {
        return 1;
	}
	return 0;
}

void window_building_draw_grand_temple_ceres(building_info_context* c)
{
    draw_grand_temple(c, "wavs/temple_farm.wav", TR_BUILDING_GRAND_TEMPLE_CERES_DESC, TR_BUILDING_GRAND_TEMPLE_CERES_BONUS_DESC, mods_get_image_id(mods_get_group_id("Areldir", "UI_Elements"), "Ceres L Banner"), TR_BUILDING_CERES_TEMPLE_QUOTE, GOD_CERES, 0);
}

void window_building_draw_grand_temple_neptune(building_info_context* c)
{
    draw_grand_temple(c, "wavs/temple_ship.wav", TR_BUILDING_GRAND_TEMPLE_NEPTUNE_DESC, TR_BUILDING_GRAND_TEMPLE_NEPTUNE_BONUS_DESC, mods_get_image_id(mods_get_group_id("Areldir", "UI_Elements"), "Nept L Banner"), TR_BUILDING_NEPTUNE_TEMPLE_QUOTE, GOD_NEPTUNE, 0);
}

void window_building_draw_grand_temple_mercury(building_info_context* c)
{
    draw_grand_temple(c, "wavs/temple_comm.wav", TR_BUILDING_GRAND_TEMPLE_MERCURY_DESC, TR_BUILDING_GRAND_TEMPLE_MERCURY_BONUS_DESC, mods_get_image_id(mods_get_group_id("Areldir", "UI_Elements"), "Merc L Banner"), TR_BUILDING_MERCURY_TEMPLE_QUOTE, GOD_MERCURY, 0);
}

void window_building_draw_grand_temple_mars(building_info_context* c)
{
    draw_grand_temple(c, "wavs/temple_war.wav", TR_BUILDING_GRAND_TEMPLE_MARS_DESC, TR_BUILDING_GRAND_TEMPLE_MARS_BONUS_DESC, mods_get_image_id(mods_get_group_id("Areldir", "UI_Elements"), "Mars L Banner"), TR_BUILDING_MARS_TEMPLE_QUOTE, GOD_MARS, 50);
}

void window_building_draw_grand_temple_venus(building_info_context* c)
{
    draw_grand_temple(c, "wavs/temple_love.wav", TR_BUILDING_GRAND_TEMPLE_VENUS_DESC, TR_BUILDING_GRAND_TEMPLE_VENUS_BONUS_DESC, mods_get_image_id(mods_get_group_id("Areldir", "UI_Elements"), "Venus L Banner"), TR_BUILDING_VENUS_TEMPLE_QUOTE, GOD_VENUS, 20);
}

void window_building_draw_pantheon(building_info_context* c)
{
    draw_grand_temple(c, "wavs/oracle.wav", TR_BUILDING_PANTHEON_DESC, TR_BUILDING_PANTHEON_BONUS_DESC, mods_get_image_id(mods_get_group_id("Areldir", "UI_Elements"), "Panth L Banner"), TR_BUILDING_PANTHEON_QUOTE, GOD_PANTHEON, 0);
}

void window_building_draw_lighthouse(building_info_context* c) {
    building* b = building_get(c->building_id);
    //window_building_play_sound(c, sound_file);
    if (b->subtype.monument_phase == MONUMENT_FINISHED) {
        outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
        int height = text_draw_multiline(translation_for(TR_BUILDING_LIGHTHOUSE_BONUS_DESC), c->x_offset + 22, c->y_offset + 56, 15 * c->width_blocks, FONT_NORMAL_BLACK, 0);
        image_draw(mods_get_image_id(mods_get_group_id("Areldir", "UI_Elements"), "Lighthouse Banner"), c->x_offset + 32, c->y_offset + 166 + height);
        inner_panel_draw(c->x_offset + 16, c->y_offset + 86 + height, c->width_blocks - 2, 4);
        window_building_draw_employment(c, 96 + height);

    } else {
        outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
        window_building_draw_monument_lighthouse_construction_process(c);
    }
    text_draw_centered(translation_for(TR_BUILDING_LIGHTHOUSE), c->x_offset, c->y_offset + 12, 16 * c->width_blocks, FONT_LARGE_BLACK, 0);
}


void window_building_draw_work_camp(building_info_context* c)
{
	window_building_play_sound(c, "wavs/eng_post.wav");
	outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
	inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    text_draw_centered(translation_for(TR_BUILDING_WORK_CAMP), c->x_offset, c->y_offset + 12, 16 * c->width_blocks, FONT_LARGE_BLACK, 0);
    text_draw_multiline(translation_for(TR_BUILDING_WORK_CAMP_DESC), c->x_offset + 32, c->y_offset + 76, 16 * (c->width_blocks - 4), FONT_NORMAL_BLACK, 0);
    window_building_draw_employment(c, 138);
}

void window_building_draw_engineer_guild(building_info_context* c)
{
    window_building_play_sound(c, "wavs/eng_post.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    text_draw_centered(translation_for(TR_BUILDING_ENGINEER_GUILD), c->x_offset, c->y_offset + 12, 16 * c->width_blocks, FONT_LARGE_BLACK, 0);
    text_draw_multiline(translation_for(TR_BUILDING_ENGINEER_GUILD_DESC), c->x_offset + 32, c->y_offset + 76, 16 * (c->width_blocks - 4), FONT_NORMAL_BLACK, 0);
    window_building_draw_employment(c, 138);
}

static void add_module(int selection)
{
    if (!selection) {
        return;
    }
    sound_speech_play_file("wavs/oracle.wav");
    building* b = building_get(data.building_id);
    city_finance_process_construction(MODULE_COST);
    building_monument_add_module(b, selection);
}

static void add_module_prompt(int param1, int param2)
{
    option_menu_item options[2] = { temple_module_options[god_id * 2], temple_module_options[god_id * 2 + 1] };
    window_option_popup_show(TR_SELECT_EPITHET_PROMPT_HEADER, TR_SELECT_EPITHET_PROMPT_TEXT, options, add_module, 1);
}
