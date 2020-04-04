#include "edit_request.h"

#include "game/resource.h"
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
#include "window/editor/requests.h"
#include "window/numeric_input.h"
#include "window/select_list.h"

static void button_year(int param1, int param2);
static void button_amount(int param1, int param2);
static void button_resource(int param1, int param2);
static void button_deadline_years(int param1, int param2);
static void button_favor(int param1, int param2);
static void button_delete(int param1, int param2);
static void button_save(int param1, int param2);

static generic_button buttons[] = {
    {30, 152, 60, 25, button_year, button_none},
    {330, 152, 80, 25, button_amount, button_none},
    {430, 152, 100, 25, button_resource, button_none},
    {70, 190, 140, 25, button_deadline_years, button_none},
    {400, 190, 80, 25, button_favor, button_none},
    {10, 234, 250, 25, button_delete, button_none},
    {300, 234, 100, 25, button_save, button_none}
};

static struct {
    int id;
    editor_request request;
    int focus_button_id;
} data;

static void init(int id)
{
    data.id = id;
    scenario_editor_request_get(id, &data.request);
}

static void draw_background(void)
{
    window_editor_map_draw_all();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(0, 100, 38, 11);
    lang_text_draw(44, 21, 14, 114, FONT_LARGE_BLACK);

    button_border_draw(30, 152, 60, 25, data.focus_button_id == 1);
    text_draw_number_centered_prefix(data.request.year, '+', 30, 158, 60, FONT_NORMAL_BLACK);
    lang_text_draw_year(scenario_property_start_year() + data.request.year, 110, 158, FONT_NORMAL_BLACK);

    lang_text_draw(44, 72, 250, 158, FONT_NORMAL_BLACK);
    button_border_draw(330, 152, 80, 25, data.focus_button_id == 2);
    text_draw_number_centered(data.request.amount, 330, 158, 80, FONT_NORMAL_BLACK);

    button_border_draw(430, 152, 100, 25, data.focus_button_id == 3);
    lang_text_draw_centered(23, data.request.resource, 430, 158, 100, FONT_NORMAL_BLACK);

    lang_text_draw(44, 24, 40, 196, FONT_NORMAL_BLACK);
    button_border_draw(70, 190, 140, 25, data.focus_button_id == 4);
    lang_text_draw_amount(8, 8, data.request.deadline_years, 80, 196, FONT_NORMAL_BLACK);

    lang_text_draw(44, 73, 300, 196, FONT_NORMAL_BLACK);
    button_border_draw(400, 190, 80, 25, data.focus_button_id == 5);
    text_draw_number_centered_prefix(data.request.favor, '+', 400, 196, 80, FONT_NORMAL_BLACK);

    button_border_draw(300, 234, 100, 25, data.focus_button_id == 7);
    lang_text_draw_centered(18, 3, 300, 240, 100, FONT_NORMAL_BLACK);

    button_border_draw(10, 234, 250, 25, data.focus_button_id == 6);
    lang_text_draw_centered(44, 25, 10, 240, 250, FONT_NORMAL_BLACK);

    graphics_reset_dialog();
}

static void handle_mouse(const mouse *m)
{
    if (generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, buttons, 7, &data.focus_button_id)) {
        return;
    }
    if (input_go_back_requested()) {
        button_save(0, 0);
    }
}

static void set_year(int value)
{
    data.request.year = value;
}
static void button_year(int param1, int param2)
{
    window_numeric_input_show(screen_dialog_offset_x() + 100, screen_dialog_offset_y() + 50, 3, 999, set_year);
}

static void set_amount(int value)
{
    data.request.amount = value;
}
static void button_amount(int param1, int param2)
{
    int max_amount = 999;
    int max_digits = 3;
    if (data.request.resource == RESOURCE_DENARII) {
        max_amount = 30000;
        max_digits = 5;
    }
    window_numeric_input_show(
        screen_dialog_offset_x() + 190, screen_dialog_offset_y() + 50,
        max_digits, max_amount, set_amount
    );
}

static void set_resource(int value)
{
    data.request.resource = value;
    if (data.request.amount > 999) {
        data.request.amount = 999;
    }
}
static void button_resource(int param1, int param2)
{
    window_select_list_show(screen_dialog_offset_x() + 210, screen_dialog_offset_y() + 40, 23, 17, set_resource);
}

static void set_deadline_years(int value)
{
    data.request.deadline_years = value;
}
static void button_deadline_years(int param1, int param2)
{
    window_numeric_input_show(screen_dialog_offset_x() + 220, screen_dialog_offset_y() + 100, 3, 999, set_deadline_years);
}

static void set_favor(int value)
{
    data.request.favor = value;
}
static void button_favor(int param1, int param2)
{
    window_numeric_input_show(screen_dialog_offset_x() + 260, screen_dialog_offset_y() + 100, 3, 100, set_favor);
}

static void button_delete(int param1, int param2)
{
    scenario_editor_request_delete(data.id);
    window_editor_requests_show();
}

static void button_save(int param1, int param2)
{
    scenario_editor_request_save(data.id, &data.request);
    window_editor_requests_show();
}

void window_editor_edit_request_show(int id)
{
    window_type window = {
        WINDOW_EDITOR_EDIT_REQUEST,
        draw_background,
        draw_foreground,
        handle_mouse
    };
    init(id);
    window_show(&window);
}
