#include "edit_invasion.h"

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
#include "scenario/types.h"
#include "window/editor/invasions.h"
#include "window/editor/map.h"
#include "window/numeric_input.h"
#include "window/select_list.h"

static void button_year(int param1, int param2);
static void button_amount(int param1, int param2);
static void button_type(int param1, int param2);
static void button_from(int param1, int param2);
static void button_attack(int param1, int param2);
static void button_delete(int param1, int param2);
static void button_save(int param1, int param2);

static generic_button buttons[] = {
    {30, 152, 60, 25, button_year, button_none},
    {200, 152, 80, 25, button_amount, button_none},
    {320, 152, 200, 25, button_type, button_none},
    {130, 190, 190, 25, button_from, button_none},
    {340, 190, 220, 25, button_attack, button_none},
    {20, 230, 250, 25, button_delete, button_none},
    {310, 230, 100, 25, button_save, button_none},
};

static struct {
    int id;
    editor_invasion invasion;
    int focus_button_id;
} data;

static void init(int id)
{
    data.id = id;
    scenario_editor_invasion_get(id, &data.invasion);
}

static void draw_background(void)
{
    window_editor_map_draw_all();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(0, 100, 38, 11);
    lang_text_draw(44, 22, 14, 114, FONT_LARGE_BLACK);

    button_border_draw(30, 152, 60, 25, data.focus_button_id == 1);
    text_draw_number_centered_prefix(data.invasion.year, '+', 30, 158, 60, FONT_NORMAL_BLACK);
    lang_text_draw_year(scenario_property_start_year() + data.invasion.year, 100, 158, FONT_NORMAL_BLACK);

    button_border_draw(200, 152, 80, 25, data.focus_button_id == 2);
    text_draw_number_centered(data.invasion.amount, 200, 158, 80, FONT_NORMAL_BLACK);

    button_border_draw(320, 152, 200, 25, data.focus_button_id == 3);
    lang_text_draw_centered(34, data.invasion.type, 320, 158, 200, FONT_NORMAL_BLACK);

    if (data.invasion.type != INVASION_TYPE_DISTANT_BATTLE) {
        lang_text_draw(44, 27, 40, 196, FONT_NORMAL_BLACK);
        button_border_draw(130, 190, 190, 25, data.focus_button_id == 4);
        lang_text_draw_centered(35, data.invasion.from, 130, 196, 190, FONT_NORMAL_BLACK);

        button_border_draw(340, 190, 220, 25, data.focus_button_id == 5);
        lang_text_draw_centered(36, data.invasion.attack_type, 340, 196, 220, FONT_NORMAL_BLACK);
    }

    button_border_draw(310, 230, 100, 25, data.focus_button_id == 7);
    lang_text_draw_centered(18, 3, 310, 236, 100, FONT_NORMAL_BLACK);

    button_border_draw(20, 230, 250, 25, data.focus_button_id == 6);
    lang_text_draw_centered(44, 26, 20, 236, 250, FONT_NORMAL_BLACK);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, buttons, 7, &data.focus_button_id)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        button_save(0, 0);
    }
}

static void set_year(int value)
{
    data.invasion.year = value;
}

static void button_year(int param1, int param2)
{
    window_numeric_input_show(screen_dialog_offset_x() + 100, screen_dialog_offset_y() + 50, 3, 999, set_year);
}

static void set_amount(int value)
{
    data.invasion.amount = value;
}

static void button_amount(int param1, int param2)
{
    window_numeric_input_show(screen_dialog_offset_x() + 60, screen_dialog_offset_y() + 50, 3, 200, set_amount);
}

static void set_type(int value)
{
    data.invasion.type = value == 3 ? 4 : value;
}

static void button_type(int param1, int param2)
{
    window_select_list_show(screen_dialog_offset_x() + 100, screen_dialog_offset_y() + 120, 34, 4, set_type);
}

static void set_from(int value)
{
    data.invasion.from = value;
}

static void button_from(int param1, int param2)
{
    if (data.invasion.type != INVASION_TYPE_DISTANT_BATTLE) {
        window_select_list_show(screen_dialog_offset_x() + 330, screen_dialog_offset_y() + 50, 35, 9, set_from);
    }
}

static void set_attack(int value)
{
    data.invasion.attack_type = value;
}

static void button_attack(int param1, int param2)
{
    if (data.invasion.type != INVASION_TYPE_DISTANT_BATTLE) {
        window_select_list_show(screen_dialog_offset_x() + 120, screen_dialog_offset_y() + 120, 36, 5, set_attack);
    }
}

static void button_delete(int param1, int param2)
{
    scenario_editor_invasion_delete(data.id);
    window_editor_invasions_show();
}

static void button_save(int param1, int param2)
{
    scenario_editor_invasion_save(data.id, &data.invasion);
    window_editor_invasions_show();
}

void window_editor_edit_invasion_show(int id)
{
    window_type window = {
        WINDOW_EDITOR_EDIT_INVASION,
        draw_background,
        draw_foreground,
        handle_input
    };
    init(id);
    window_show(&window);
}
