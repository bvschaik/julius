#include "allowed_buildings.h"

#include "graphics/button.h"
#include "graphics/color.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/editor.h"
#include "window/editor/attributes.h"
#include "window/editor/map.h"

static void toggle_building(int id, int param2);

static generic_button buttons[] = {
    {25, 82, 190, 18, toggle_building, button_none, 1, 0},
    {25, 102, 190, 18, toggle_building, button_none, 2, 0},
    {25, 122, 190, 18, toggle_building, button_none, 3, 0},
    {25, 142, 190, 18, toggle_building, button_none, 4, 0},
    {25, 162, 190, 18, toggle_building, button_none, 5, 0},
    {25, 182, 190, 18, toggle_building, button_none, 6, 0},
    {25, 202, 190, 18, toggle_building, button_none, 7, 0},
    {25, 222, 190, 18, toggle_building, button_none, 8, 0},
    {25, 242, 190, 18, toggle_building, button_none, 9, 0},
    {25, 262, 190, 18, toggle_building, button_none, 10, 0},
    {25, 282, 190, 18, toggle_building, button_none, 11, 0},
    {25, 302, 190, 18, toggle_building, button_none, 12, 0},
    {25, 322, 190, 18, toggle_building, button_none, 13, 0},
    {25, 342, 190, 18, toggle_building, button_none, 14, 0},
    {25, 362, 190, 18, toggle_building, button_none, 15, 0},
    {25, 382, 190, 18, toggle_building, button_none, 16, 0},
    {224, 82, 190, 18, toggle_building, button_none, 17, 0},
    {224, 102, 190, 18, toggle_building, button_none, 18, 0},
    {224, 122, 190, 18, toggle_building, button_none, 19, 0},
    {224, 142, 190, 18, toggle_building, button_none, 20, 0},
    {224, 162, 190, 18, toggle_building, button_none, 21, 0},
    {224, 182, 190, 18, toggle_building, button_none, 22, 0},
    {224, 202, 190, 18, toggle_building, button_none, 23, 0},
    {224, 222, 190, 18, toggle_building, button_none, 24, 0},
    {224, 242, 190, 18, toggle_building, button_none, 25, 0},
    {224, 262, 190, 18, toggle_building, button_none, 26, 0},
    {224, 282, 190, 18, toggle_building, button_none, 27, 0},
    {224, 302, 190, 18, toggle_building, button_none, 28, 0},
    {224, 322, 190, 18, toggle_building, button_none, 29, 0},
    {224, 342, 190, 18, toggle_building, button_none, 30, 0},
    {224, 362, 190, 18, toggle_building, button_none, 31, 0},
    {224, 382, 190, 18, toggle_building, button_none, 32, 0},
    {423, 82, 190, 18, toggle_building, button_none, 33, 0},
    {423, 102, 190, 18, toggle_building, button_none, 34, 0},
    {423, 122, 190, 18, toggle_building, button_none, 35, 0},
    {423, 142, 190, 18, toggle_building, button_none, 36, 0},
    {423, 162, 190, 18, toggle_building, button_none, 37, 0},
    {423, 182, 190, 18, toggle_building, button_none, 38, 0},
    {423, 202, 190, 18, toggle_building, button_none, 39, 0},
    {423, 222, 190, 18, toggle_building, button_none, 40, 0},
    {423, 242, 190, 18, toggle_building, button_none, 41, 0},
    {423, 262, 190, 18, toggle_building, button_none, 42, 0},
    {423, 282, 190, 18, toggle_building, button_none, 43, 0},
    {423, 302, 190, 18, toggle_building, button_none, 44, 0},
    {423, 322, 190, 18, toggle_building, button_none, 45, 0},
    {423, 342, 190, 18, toggle_building, button_none, 46, 0},
    {423, 362, 190, 18, toggle_building, button_none, 47, 0},
    {423, 382, 190, 18, toggle_building, button_none, 48, 0},
};

static int focus_button_id;

static void draw_background(void)
{
    window_editor_map_draw_all();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(16, 32, 38, 26);

    lang_text_draw(44, 47, 26, 42, FONT_LARGE_BLACK);
    lang_text_draw_centered(13, 3, 16, 424, 608, FONT_NORMAL_BLACK);
    for (int i = 1; i <= 47; i++) {
        int x, y;
        if (i <= 16) {
            x = 25;
            y = 82 + 20 * (i - 1);
        } else if (i <= 32) {
            x = 224;
            y = 82 + 20 * (i - 17);
        } else {
            x = 423;
            y = 82 + 20 * (i - 33);
        }
        button_border_draw(x, y, 190, 18, focus_button_id == i);
        if (scenario_editor_is_building_allowed(i)) {
            lang_text_draw_centered(67, i, x, y + 4, 190, FONT_NORMAL_BLACK);
        } else {
            lang_text_draw_centered_colored(67, i, x, y + 4, 190, FONT_NORMAL_PLAIN, COLOR_FONT_RED);
        }
    }

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, buttons, 47, &focus_button_id)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_editor_attributes_show();
    }
}

void toggle_building(int id, int param2)
{
    scenario_editor_toggle_building_allowed(id);
}

void window_editor_allowed_buildings_show(void)
{
    window_type window = {
        WINDOW_EDITOR_ALLOWED_BUILDINGS,
        draw_background,
        draw_foreground,
        handle_input
    };
    window_show(&window);
}
