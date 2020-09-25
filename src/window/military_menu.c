#include "military_menu.h"

#include "city/view.h"
#include "figure/formation.h"
#include "graphics/generic_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/window.h"
#include "input/input.h"
#include "map/grid.h"
#include "widget/sidebar/military.h"
#include "window/city.h"

static struct {
    int active_buttons;
    int focus_button_id;
} data;

static void button_menu_item(int index, int param2);

static generic_button menu_buttons[] = {
    {0, 0, 160, 24, button_menu_item, button_none, 1, 0},
    {0, 24, 160, 24, button_menu_item, button_none, 2, 0},
    {0, 48, 160, 24, button_menu_item, button_none, 3, 0},
    {0, 72, 160, 24, button_menu_item, button_none, 4, 0},
    {0, 96, 160, 24, button_menu_item, button_none, 5, 0},
    {0, 120, 160, 24, button_menu_item, button_none, 6, 0},
};

static int get_sidebar_x_offset(void)
{
    int view_x, view_y, view_width, view_height;
    city_view_get_viewport(&view_x, &view_y, &view_width, &view_height);
    return view_x + view_width;
}

static void draw_background(void)
{
    window_city_draw_panels();
}

static void draw_foreground(void)
{
    window_city_draw();
    int num_legions = formation_get_num_legions();
    int x_offset = get_sidebar_x_offset();

    for (int i = 0; i < num_legions; i++) {
        const formation *m = formation_get(formation_for_legion(i + 1));
        label_draw(x_offset - 170, 74 + 24 * i, 10, data.focus_button_id == i + 1 ? 1 : 2);
        lang_text_draw_centered(138, m->legion_id, x_offset - 170, 77 + 24 * i, 160, FONT_NORMAL_GREEN);
    }
    data.active_buttons = num_legions;
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (generic_buttons_handle_mouse(m, get_sidebar_x_offset() - 170, 72,
        menu_buttons, data.active_buttons, &data.focus_button_id)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_go_back();
    }
}

static void button_menu_item(int index, int param2)
{
    int formation_id = formation_for_legion(index);
    const formation *m = formation_get(formation_id);
    city_view_go_to_grid_offset(map_grid_offset(m->x_home, m->y_home));
    window_city_military_show(formation_id);
}

void window_military_menu_show(void)
{
    window_type window = {
        WINDOW_MILITARY_MENU,
        draw_background,
        draw_foreground,
        handle_input,
        0
    };
    window_show(&window);
}
