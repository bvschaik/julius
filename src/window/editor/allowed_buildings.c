#include "allowed_buildings.h"

#include "graphics/button.h"
#include "graphics/color.h"
#include "graphics/graphics.h"
#include "graphics/grid_box.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/editor.h"
#include "window/editor/attributes.h"
#include "window/editor/map.h"

static void draw_allowed_building(const grid_box_item *item);
static void toggle_building(unsigned int id, unsigned int mouse_x, unsigned int mouse_y);

static grid_box_type allowed_building_list = {
    .x = 25,
    .y = 82,
    .width = 36 * BLOCK_SIZE,
    .height = 20 * BLOCK_SIZE,
    .item_height = 20,
    .num_columns = 3,
    .item_margin.horizontal = 10,
    .item_margin.vertical = 2,
    .extend_to_hidden_scrollbar = 1,
    .draw_item = draw_allowed_building,
    .on_click = toggle_building
};

static void draw_background(void)
{
    window_editor_map_draw_all();

    graphics_in_dialog();

    outer_panel_draw(16, 32, 38, 26);
    lang_text_draw(44, 47, 26, 42, FONT_LARGE_BLACK);
    lang_text_draw_centered(13, 3, 16, 424, 608, FONT_NORMAL_BLACK);

    graphics_reset_dialog();

    grid_box_request_refresh(&allowed_building_list);
}

static void draw_allowed_building(const grid_box_item *item)
{
    button_border_draw(item->x, item->y, item->width, item->height, item->is_focused);
    if (scenario_editor_is_building_allowed(item->index + 1)) {
        lang_text_draw_centered(67, item->index + 1, item->x, item->y + 4, item->width, FONT_NORMAL_BLACK);
    } else {
        lang_text_draw_centered_colored(67, item->index + 1, item->x, item->y + 4, item->width,
            FONT_NORMAL_PLAIN, COLOR_FONT_RED);
    }
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    grid_box_draw(&allowed_building_list);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (grid_box_handle_input(&allowed_building_list, mouse_in_dialog(m), 1)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_editor_attributes_show();
    }
}

void toggle_building(unsigned int id, unsigned int mouse_x, unsigned int mouse_y)
{
    scenario_editor_toggle_building_allowed(id + 1);
    window_request_refresh();
}

void window_editor_allowed_buildings_show(void)
{
    window_type window = {
        WINDOW_EDITOR_ALLOWED_BUILDINGS,
        draw_background,
        draw_foreground,
        handle_input
    };
    grid_box_init(&allowed_building_list, 48);
    window_show(&window);
}
