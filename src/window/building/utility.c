#include "utility.h"

#include "building/building.h"
#include "building/roadblock.h"
#include "core/image.h"
#include "graphics/generic_button.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/window.h"
#include "map/water_supply.h"
#include "window/building_info.h"


static void go_to_orders(int param1, int param2);
static void toggle_figure_state(int index, int param2);
static void roadblock_orders(int index, int param2);



static struct {
    int focus_button_id;
    int orders_focus_button_id;
    int figure_focus_button_id;
    int building_id;
} data = {0, 0, 0, 0};

static generic_button go_to_orders_button[] = {
    {0, 0, 304, 20, go_to_orders, button_none, 0, 0}
};


static generic_button orders_permission_buttons[] = {
    {0, 4, 210, 22, toggle_figure_state, button_none, 1, 0},
    {0, 36, 210, 22, toggle_figure_state, button_none, 2, 0},
    {0, 68, 210, 22, toggle_figure_state, button_none, 3, 0},
    {0, 100, 210, 22, toggle_figure_state, button_none, 4, 0},
};

static generic_button roadblock_order_buttons[] = {
    {314, 0, 20, 20, roadblock_orders, button_none, 0, 0},
};



void window_building_draw_engineers_post(building_info_context *c)
{
    c->help_id = 81;
    window_building_play_sound(c, "wavs/eng_post.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(104, 0, c->x_offset, c->y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);

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
    lang_text_draw_centered(88, 0, c->x_offset, c->y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);

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


void window_building_draw_roadblock(building_info_context *c)
{
    c->help_id = 0;
    window_building_play_sound(c, "wavs/prefecture.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(28, 115, c->x_offset, c->y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);
    building *b = building_get(c->building_id);
    window_building_draw_description(c, 28, 116);
    
}

void window_building_draw_roadblock_foreground(building_info_context* c)
{
    button_border_draw(c->x_offset + 80, c->y_offset + 16 * c->height_blocks - 34,
        16 * (c->width_blocks - 10), 20, data.focus_button_id == 1 ? 1 : 0);
    lang_text_draw_centered(98, 5, c->x_offset + 80, c->y_offset + 16 * c->height_blocks - 30,
        16 * (c->width_blocks - 10), FONT_NORMAL_BLACK);

}

void window_building_draw_roadblock_orders(building_info_context* c)
{
    c->help_id = 3;
    int y_offset = window_building_get_vertical_offset(c, 28);
    outer_panel_draw(c->x_offset, y_offset, 29, 28);
    lang_text_draw_centered(28, 115, c->x_offset, y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);
    inner_panel_draw(c->x_offset + 16, y_offset + 42, c->width_blocks - 2, 21);

}

void window_building_draw_roadblock_orders_foreground(building_info_context* c)
{
    int y_offset = window_building_get_vertical_offset(c, 28);
    int ids[8] = { GROUP_FIGURE_ENGINEER,GROUP_FIGURE_PREFECT,GROUP_FIGURE_PRIEST,GROUP_FIGURE_PRIEST,GROUP_FIGURE_MARKET_LADY,GROUP_FIGURE_MARKET_LADY,GROUP_FIGURE_ACTOR,GROUP_FIGURE_LION_TAMER};
    building* b = building_get(c->building_id);
    data.building_id = b->id;


    for (int i = 0; i < 4; i++) {
        image_draw(image_group(ids[i*2]) + 4, c->x_offset + 32, y_offset + 46 + 32 * i);
        image_draw(image_group(ids[i*2+1]) + 4, c->x_offset + 64, y_offset + 46 + 32 * i);
       // lang_text_draw(23, resource, c->x_offset + 72, y_offset + 50 + 22 * i, FONT_NORMAL_WHITE);
        button_border_draw(c->x_offset + 180, y_offset + 50 + 32 * i, 210, 22, data.figure_focus_button_id == i + 1);
        int state = building_roadblock_get_permission(i+1, b);
        if (state) {
            lang_text_draw(99, 7, c->x_offset + 230, y_offset + 55 + 32 * i, FONT_NORMAL_WHITE);
        }
        else {
            lang_text_draw(99, 8, c->x_offset + 230, y_offset + 55 + 32 * i, FONT_NORMAL_RED);
        }


        building* b = building_get(c->building_id);

    }
}


void window_building_draw_burning_ruin(building_info_context *c)
{
    c->help_id = 0;
    window_building_play_sound(c, "wavs/ruin.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(111, 0, c->x_offset, c->y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);

    lang_text_draw(41, c->rubble_building_type, c->x_offset + 32, c->y_offset + 16 * c->height_blocks - 173, FONT_NORMAL_BLACK);
    lang_text_draw_multiline(111, 1, c->x_offset + 32, c->y_offset + 16 * c->height_blocks - 143, 16 * (c->width_blocks - 4), FONT_NORMAL_BLACK);
}

void window_building_draw_rubble(building_info_context *c)
{
    c->help_id = 0;
    window_building_play_sound(c, "wavs/ruin.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(140, 0, c->x_offset, c->y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);

    lang_text_draw(41, c->rubble_building_type, c->x_offset + 32, c->y_offset + 16 * c->height_blocks - 173, FONT_NORMAL_BLACK);
    lang_text_draw_multiline(140, 1, c->x_offset + 32, c->y_offset + 16 * c->height_blocks - 143, 16 * (c->width_blocks - 4), FONT_NORMAL_BLACK);
}

void window_building_draw_reservoir(building_info_context *c)
{
    c->help_id = 59;
    window_building_play_sound(c, "wavs/resevoir.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(107, 0, c->x_offset, c->y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);
    int text_id = building_get(c->building_id)->has_water_access ? 1 : 3;
    window_building_draw_description_at(c, 16 * c->height_blocks - 173, 107, text_id);
}

void window_building_draw_aqueduct(building_info_context *c)
{
    c->help_id = 60;
    window_building_play_sound(c, "wavs/aquaduct.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(141, 0, c->x_offset, c->y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);
    window_building_draw_description_at(c, 16 * c->height_blocks - 144, 141, c->aqueduct_has_water ? 1 : 2);
}

void window_building_draw_fountain(building_info_context *c)
{
    c->help_id = 61;
    window_building_play_sound(c, "wavs/fountain.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(108, 0, c->x_offset, c->y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);
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
    lang_text_draw_centered(109, 0, c->x_offset, c->y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);
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
        window_building_draw_description_at(c, 16 * c->height_blocks - 160, 109, text_id);
    }
}

void window_building_draw_mission_post(building_info_context *c)
{
    c->help_id = 8;
    window_building_play_sound(c, "wavs/mission.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(134, 0, c->x_offset, c->y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);
    window_building_draw_description(c, 134, 1);
    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment_without_house_cover(c, 142);
}

static void draw_native(building_info_context *c, int group_id)
{
    c->help_id = 0;
    window_building_play_sound(c, "wavs/empty_land.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(group_id, 0, c->x_offset, c->y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);
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

void toggle_figure_state(int index, int param2) {
    building* b = building_get(data.building_id);
    if (b->type == BUILDING_ROADBLOCK) {
        building_roadblock_set_permission(index, b);
    }
    window_invalidate();

}

static void roadblock_orders(int param1, int param2) {

}

static void go_to_orders(int param1, int param2)
{
    window_building_info_show_storage_orders();
}


int window_building_handle_mouse_roadblock(const mouse* m, building_info_context* c)
{
   return generic_buttons_handle_mouse(
        m, c->x_offset + 80, c->y_offset + 16 * c->height_blocks - 34,
        go_to_orders_button, 1, &data.focus_button_id);
}

int window_building_handle_mouse_roadblock_orders(const mouse* m, building_info_context* c)
{
    int y_offset = window_building_get_vertical_offset(c, 28);

    data.building_id = c->building_id;
    if (generic_buttons_handle_mouse(m, c->x_offset + 180, y_offset + 46,
        orders_permission_buttons, 4,
        &data.figure_focus_button_id)) {
        return 1;
    }
    return generic_buttons_handle_mouse(m, c->x_offset + 80, y_offset + 404, roadblock_order_buttons, 1, &data.orders_focus_button_id);


}

