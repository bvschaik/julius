#include "requests.h"

#include "core/image_group_editor.h"
#include "game/resource.h"
#include "graphics/button.h"
#include "graphics/graphics.h"
#include "graphics/grid_box.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/data.h"
#include "scenario/editor.h"
#include "scenario/property.h"
#include "window/editor/attributes.h"
#include "window/editor/edit_request.h"
#include "window/editor/map.h"

static void button_request(unsigned int id, unsigned int mouse_x, unsigned int mouse_y);
static void draw_request_button(const grid_box_item *item);

static grid_box_type request_buttons = {
    .x = 20,
    .y = 40,
    .width = 38 * BLOCK_SIZE,
    .height = 19 * BLOCK_SIZE,
    .num_columns = 2,
    .item_height = 30,
    .item_margin.horizontal = 10,
    .item_margin.vertical = 5,
    .extend_to_hidden_scrollbar = 1,
    .on_click = button_request,
    .draw_item = draw_request_button
};

static void draw_background(void)
{
    window_editor_map_draw_all();

    graphics_in_dialog();

    outer_panel_draw(0, 0, 40, 30);
    lang_text_draw(44, 14, 20, 12, FONT_LARGE_BLACK);
    lang_text_draw_centered(13, 3, 0, 456, 640, FONT_NORMAL_BLACK);
    lang_text_draw_multiline(152, 1, 32, 376, 576, FONT_NORMAL_BLACK);

    graphics_reset_dialog();

    grid_box_request_refresh(&request_buttons);
}

static void draw_request_button(const grid_box_item *item)
{
    button_border_draw(item->x, item->y, item->width, item->height, item->is_focused);
    editor_request request;
    scenario_editor_request_get(item->index, &request);
    if (request.resource) {
        text_draw_number(request.year, '+', " ", item->x + 10, item->y + 7, FONT_NORMAL_BLACK, 0);
        lang_text_draw_year(scenario_property_start_year() + request.year, item->x + 65, item->y + 7,
            FONT_NORMAL_BLACK);
        int width = text_draw_number(request.amount, '@', " ", item->x + 165, item->y + 7, FONT_NORMAL_BLACK, 0);
        image_draw(resource_get_data(request.resource)->image.editor.icon, item->x + 165 + width, item->y + 2,
            COLOR_MASK_NONE, SCALE_NONE);
    } else {
        lang_text_draw_centered(44, 23, item->x, item->y + 7, item->width, FONT_NORMAL_BLACK);
    }
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    grid_box_draw(&request_buttons);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (grid_box_handle_input(&request_buttons, mouse_in_dialog(m), 1)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_editor_attributes_show();
    }
}

static void button_request(unsigned int id, unsigned int mouse_x, unsigned int mouse_y)
{
    window_editor_edit_request_show(id);
}

void window_editor_requests_show(void)
{
    window_type window = {
        WINDOW_EDITOR_REQUESTS,
        draw_background,
        draw_foreground,
        handle_input
    };
    grid_box_init(&request_buttons, MAX_REQUESTS);
    window_show(&window);
}
