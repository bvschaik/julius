#include "edit_price_change.h"

#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/editor.h"
#include "scenario/property.h"
#include "window/editor/map.h"
#include "window/editor/price_changes.h"
#include "window/numeric_input.h"
#include "window/select_list.h"

static void button_year(int param1, int param2);
static void button_resource(int param1, int param2);
static void button_toggle_rise(int param1, int param2);
static void button_amount(int param1, int param2);
static void button_delete(int param1, int param2);
static void button_save(int param1, int param2);

static generic_button buttons[] = {
    {30, 152, 60, 25, button_year, button_none},
    {240, 152, 120, 25, button_resource, button_none},
    {100, 192, 200, 25, button_toggle_rise, button_none},
    {350, 192, 100, 25, button_amount, button_none},
    {30, 230, 250, 25, button_delete, button_none},
    {320, 230, 100, 25, button_save, button_none}
};

static struct {
    int id;
    editor_price_change price_change;
    int focus_button_id;
} data;

static void init(int id)
{
    data.id = id;
    scenario_editor_price_change_get(id, &data.price_change);
}

static void draw_background(void)
{
    window_editor_map_draw_all();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(0, 100, 38, 11);
    lang_text_draw(44, 95, 20, 114, FONT_LARGE_BLACK);

    button_border_draw(30, 152, 60, 25, data.focus_button_id == 1);
    text_draw_number_centered_prefix(data.price_change.year, '+', 30, 158, 60, FONT_NORMAL_BLACK);
    lang_text_draw_year(scenario_property_start_year() + data.price_change.year, 100, 158, FONT_NORMAL_BLACK);

    button_border_draw(240, 152, 120, 25, data.focus_button_id == 2);
    lang_text_draw_centered(23, data.price_change.resource, 240, 158, 120, FONT_NORMAL_BLACK);

    button_border_draw(100, 192, 200, 25, data.focus_button_id == 3);
    lang_text_draw_centered(44, data.price_change.is_rise ? 104 : 103, 100, 198, 200, FONT_NORMAL_BLACK);

    button_border_draw(350, 192, 100, 25, data.focus_button_id == 4);
    text_draw_number_centered(data.price_change.amount, 350, 198, 100, FONT_NORMAL_BLACK);

    button_border_draw(30, 230, 250, 25, data.focus_button_id == 5);
    lang_text_draw_centered(44, 105, 30, 236, 250, FONT_NORMAL_BLACK);

    button_border_draw(320, 230, 100, 25, data.focus_button_id == 6);
    lang_text_draw_centered(18, 3, 320, 236, 100, FONT_NORMAL_BLACK);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, buttons, 6, &data.focus_button_id)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        button_save(0, 0);
    }
}

static void set_year(int value)
{
    data.price_change.year = value;
}

static void button_year(int param1, int param2)
{
    window_numeric_input_show(screen_dialog_offset_x() + 100, screen_dialog_offset_y() + 50, 3, 999, set_year);
}

static void set_resource(int value)
{
    data.price_change.resource = value;
}

static void button_resource(int param1, int param2)
{
    window_select_list_show(screen_dialog_offset_x() + 25, screen_dialog_offset_y() + 40, 23, 16, set_resource);
}

static void button_toggle_rise(int param1, int param2)
{
    data.price_change.is_rise = !data.price_change.is_rise;
}

static void set_amount(int value)
{
    data.price_change.amount = value;
}

static void button_amount(int param1, int param2)
{
    window_numeric_input_show(screen_dialog_offset_x() + 460, screen_dialog_offset_y() + 50, 2, 99, set_amount);
}

static void button_delete(int param1, int param2)
{
    scenario_editor_price_change_delete(data.id);
    window_editor_price_changes_show();
}

static void button_save(int param1, int param2)
{
    scenario_editor_price_change_save(data.id, &data.price_change);
    window_editor_price_changes_show();
}

void window_editor_edit_price_change_show(int id)
{
    window_type window = {
        WINDOW_EDITOR_EDIT_PRICE_CHANGE,
        draw_background,
        draw_foreground,
        handle_input
    };
    init(id);
    window_show(&window);
}
