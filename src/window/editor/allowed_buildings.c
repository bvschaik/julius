#include "allowed_buildings.h"

#include "graphics/button.h"
#include "graphics/color.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/list_box.h"
#include "graphics/panel.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/editor.h"
#include "window/editor/attributes.h"
#include "window/editor/map.h"

static void draw_allowed_building(const list_box_item *item);
static void toggle_building(int id, int param2);

static list_box_type allowed_building_list = {
    .x = 25,
    .y = 82,
    .width_blocks = 36,
    .height_blocks = 20,
    .item_height = 20,
    .num_columns = 3,
    .extend_to_hidden_scrollbar = 1,
    .draw_item = draw_allowed_building,
    .on_select = toggle_building
};

static void draw_background(void)
{
    window_editor_map_draw_all();

    graphics_in_dialog();

    outer_panel_draw(16, 32, 38, 26);
    lang_text_draw(44, 47, 26, 42, FONT_LARGE_BLACK);
    lang_text_draw_centered(13, 3, 16, 424, 608, FONT_NORMAL_BLACK);

    graphics_reset_dialog();

    list_box_request_refresh(&allowed_building_list);
}

static void draw_allowed_building(const list_box_item *item)
{
    button_border_draw(item->x + 5, item->y + 1, item->width - 10, item->height - 2, item->is_focused);
    if (scenario_editor_is_building_allowed(item->index + 1)) {
        lang_text_draw_centered(67, item->index + 1, item->x + 5, item->y + 5, item->width - 10, FONT_NORMAL_BLACK);
    } else {
        lang_text_draw_centered_colored(67, item->index + 1, item->x + 5, item->y + 5, item->width - 10,
            FONT_NORMAL_PLAIN, COLOR_FONT_RED);
    }
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    list_box_draw(&allowed_building_list);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (list_box_handle_input(&allowed_building_list, mouse_in_dialog(m), 1)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_editor_attributes_show();
    }
}

void toggle_building(int id, int param2)
{
    scenario_editor_toggle_building_allowed(id + 1);
    list_box_request_refresh(&allowed_building_list);
}

void window_editor_allowed_buildings_show(void)
{
    window_type window = {
        WINDOW_EDITOR_ALLOWED_BUILDINGS,
        draw_background,
        draw_foreground,
        handle_input
    };
    list_box_init(&allowed_building_list, 48);
    window_show(&window);
}
