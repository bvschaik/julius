#include "overlay_menu.h"

#include "city/view.h"
#include "core/time.h"
#include "game/state.h"
#include "graphics/generic_button.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/window.h"
#include "window/city.h"

static void button_menu_item(int index, int param2);
static void button_submenu_item(int index, int param2);

static generic_button menu_buttons[] = {
    {0, 0, 160, 24, GB_IMMEDIATE, button_menu_item, button_none, 0, 0},
    {0, 24, 160, 48, GB_IMMEDIATE, button_menu_item, button_none, 1, 0},
    {0, 48, 160, 72, GB_IMMEDIATE, button_menu_item, button_none, 2, 0},
    {0, 72, 160, 96, GB_IMMEDIATE, button_menu_item, button_none, 3, 0},
    {0, 96, 160, 120, GB_IMMEDIATE, button_menu_item, button_none, 4, 0},
    {0, 120, 160, 144, GB_IMMEDIATE, button_menu_item, button_none, 5, 0},
    {0, 144, 160, 168, GB_IMMEDIATE, button_menu_item, button_none, 6, 0},
    {0, 168, 160, 192, GB_IMMEDIATE, button_menu_item, button_none, 7, 0},
    {0, 192, 160, 216, GB_IMMEDIATE, button_menu_item, button_none, 8, 0},
    {0, 216, 160, 240, GB_IMMEDIATE, button_menu_item, button_none, 9, 0},
};
static generic_button submenu_buttons[] = {
    {0, 0, 160, 24, GB_IMMEDIATE, button_submenu_item, button_none, 0, 0},
    {0, 24, 160, 48, GB_IMMEDIATE, button_submenu_item, button_none, 1, 0},
    {0, 48, 160, 72, GB_IMMEDIATE, button_submenu_item, button_none, 2, 0},
    {0, 72, 160, 96, GB_IMMEDIATE, button_submenu_item, button_none, 3, 0},
    {0, 96, 160, 120, GB_IMMEDIATE, button_submenu_item, button_none, 4, 0},
    {0, 120, 160, 144, GB_IMMEDIATE, button_submenu_item, button_none, 5, 0},
    {0, 144, 160, 168, GB_IMMEDIATE, button_submenu_item, button_none, 6, 0},
    {0, 168, 160, 192, GB_IMMEDIATE, button_submenu_item, button_none, 7, 0},
    {0, 192, 160, 216, GB_IMMEDIATE, button_submenu_item, button_none, 8, 0},
    {0, 216, 160, 240, GB_IMMEDIATE, button_submenu_item, button_none, 9, 0},
};

static const int MENU_ID_TO_OVERLAY[8] = {OVERLAY_NONE, OVERLAY_WATER, 1, 3, 5, 6, 7, OVERLAY_RELIGION};
static const int MENU_ID_TO_SUBMENU_ID[8] = {0, 0, 1, 2, 3, 4, 5, 0};

static const int SUBMENU_ID_TO_OVERLAY[6][8] = {
    {0},
    {OVERLAY_FIRE, OVERLAY_DAMAGE, OVERLAY_CRIME, OVERLAY_NATIVE, OVERLAY_PROBLEMS, 0},
    {OVERLAY_ENTERTAINMENT, OVERLAY_THEATER, OVERLAY_AMPHITHEATER, OVERLAY_COLOSSEUM, OVERLAY_HIPPODROME, 0},
    {OVERLAY_EDUCATION, OVERLAY_SCHOOL, OVERLAY_LIBRARY, OVERLAY_ACADEMY, 0},
    {OVERLAY_BARBER, OVERLAY_BATHHOUSE, OVERLAY_CLINIC, OVERLAY_HOSPITAL, 0},
    {OVERLAY_TAX_INCOME, OVERLAY_FOOD_STOCKS, OVERLAY_DESIRABILITY, 0},
};

static struct {
    int selected_menu;
    int selected_submenu;
    int num_submenu_items;
    time_millis submenu_focus_time;

    int menu_focus_button_id;
    int submenu_focus_button_id;
} data;

static void init()
{
    data.selected_submenu = 0;
    data.num_submenu_items = 0;
}

static void draw_background()
{
    window_city_draw_panels();
}

static int get_sidebar_x_offset()
{
    int view_x, view_y, view_width, view_height;
    city_view_get_viewport(&view_x, &view_y, &view_width, &view_height);
    return view_x + view_width;
}

static void draw_foreground()
{
    window_city_draw();
    int x_offset = get_sidebar_x_offset();
    for (int i = 0; i < 8; i++) {
        label_draw(x_offset - 170, 74 + 24 * i, 10, data.menu_focus_button_id == i + 1 ? 1 : 2);
        lang_text_draw_centered(14, MENU_ID_TO_OVERLAY[i], x_offset - 170, 77 + 24 * i, 160, FONT_NORMAL_GREEN);
    }
    if (data.selected_submenu > 0) {
        image_draw(image_group(GROUP_BULLET), x_offset - 185, 80 + 24 * data.selected_menu);
        for (int i = 0; i < data.num_submenu_items; i++) {
            label_draw(x_offset - 348, 74 + 24 * (i + data.selected_menu), 10, data.submenu_focus_button_id == i + 1 ? 1 : 2);
            lang_text_draw_centered(14, SUBMENU_ID_TO_OVERLAY[data.selected_submenu][i],
                                    x_offset - 348, 77 + 24 * (i + data.selected_menu), 160, FONT_NORMAL_GREEN);
        }
    }
}

static int count_submenu_items(int submenu_id)
{
    int total = 0;
    for (int i = 0; i < 8 && SUBMENU_ID_TO_OVERLAY[submenu_id][i] > 0; i++) {
        total++;
    }
    return total;
}

static void handle_submenu()
{
    if (data.menu_focus_button_id || data.submenu_focus_button_id) {
        data.submenu_focus_time = time_get_millis();
        if (data.menu_focus_button_id) {
            data.selected_menu = data.menu_focus_button_id - 1;
            data.selected_submenu = MENU_ID_TO_SUBMENU_ID[data.selected_menu];
            data.num_submenu_items = count_submenu_items(data.selected_submenu);
        }
    } else if (time_get_millis() - data.submenu_focus_time > 500) {
        data.selected_submenu = 0;
        data.num_submenu_items = 0;
    }
}

static void handle_mouse(const mouse *m)
{
    if (m->right.went_down) {
        window_city_show();
        return;
    }
    int x_offset = get_sidebar_x_offset();
    generic_buttons_handle_mouse(m, x_offset - 170, 72, menu_buttons, 8, &data.menu_focus_button_id);

    handle_submenu();
    if (data.selected_submenu) {
        generic_buttons_handle_mouse(
            m, x_offset - 348, 72 + 24 * data.selected_menu,
            submenu_buttons, data.num_submenu_items, &data.submenu_focus_button_id);
    }
}

static void button_menu_item(int index, int param2)
{
    if (MENU_ID_TO_SUBMENU_ID[index] == 0) {
        game_state_set_overlay(MENU_ID_TO_OVERLAY[index]);
    }
    window_city_show();
}

static void button_submenu_item(int index, int param2)
{
    int overlay = SUBMENU_ID_TO_OVERLAY[data.selected_submenu][index];
    if (overlay) {
        game_state_set_overlay(overlay);
    }
    window_city_show();
}

void window_overlay_menu_show()
{
    window_type window = {
        WINDOW_OVERLAY_MENU,
        draw_background,
        draw_foreground,
        handle_mouse,
        0
    };
    init();
    window_show(&window);
}
