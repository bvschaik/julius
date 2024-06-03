#include "invasions.h"

#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/list_box.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/data.h"
#include "scenario/editor.h"
#include "scenario/property.h"
#include "window/editor/attributes.h"
#include "window/editor/edit_invasion.h"
#include "window/editor/map.h"

static void select_invasion(unsigned int id, int param2);
static void draw_invasion_button(const list_box_item *item);

static list_box_type invasion_list = {
    .x = 20,
    .y = 40,
    .width_blocks = 38,
    .height_blocks = 19,
    .num_columns = 2,
    .item_height = 30,
    .extend_to_hidden_scrollbar = 1,
    .on_select = select_invasion,
    .draw_item = draw_invasion_button
};

static void draw_background(void)
{
    window_editor_map_draw_all();
    list_box_request_refresh(&invasion_list);
}

static void draw_invasion_button(const list_box_item *item)
{
    button_border_draw(item->x + 5, item->y + 3, item->width - 10, item->height - 5, item->is_focused);
    editor_invasion invasion;
    scenario_editor_invasion_get(item->index, &invasion);
    if (invasion.type) {
        text_draw_number(invasion.year, '+', " ", item->x + 15, item->y + 10, FONT_NORMAL_BLACK, 0);
        lang_text_draw_year(scenario_property_start_year() + invasion.year, item->x + 70, item->y + 10,
            FONT_NORMAL_BLACK);
        int width = text_draw_number(invasion.amount, '@', " ", item->x + 125, item->y + 10, FONT_NORMAL_BLACK, 0);
        lang_text_draw(34, invasion.type, item->x + 120 + width, item->y + 10, FONT_NORMAL_BLACK);
    } else {
        lang_text_draw_centered(44, 23, item->x + 5, item->y + 10, item->width - 10, FONT_NORMAL_BLACK);
    }
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(0, 0, 40, 30);
    lang_text_draw(44, 15, 20, 12, FONT_LARGE_BLACK);
    lang_text_draw_centered(13, 3, 0, 456, 640, FONT_NORMAL_BLACK);
    lang_text_draw_multiline(152, 2, 32, 376, 576, FONT_NORMAL_BLACK);
    list_box_draw(&invasion_list);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (list_box_handle_input(&invasion_list, mouse_in_dialog(m), 1)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_editor_attributes_show();
    }
}

static void select_invasion(unsigned int id, int param2)
{
    window_editor_edit_invasion_show(id);
}

void window_editor_invasions_show(void)
{
    window_type window = {
        WINDOW_EDITOR_INVASIONS,
        draw_background,
        draw_foreground,
        handle_input
    };
    list_box_init(&invasion_list, MAX_INVASIONS);
    window_show(&window);
}
