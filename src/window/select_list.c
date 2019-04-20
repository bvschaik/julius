#include "select_list.h"

#include "graphics/button.h"
#include "graphics/color.h"
#include "graphics/generic_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/window.h"

#define MAX_ITEMS_PER_LIST 20

static void select_item(int id, int list_id);

static generic_button buttons_list1[MAX_ITEMS_PER_LIST] = {
    {5, 4, 195, 22, GB_ON_MOUSE_UP, select_item, button_none, 0, 0},
    {5, 24, 195, 42, GB_ON_MOUSE_UP, select_item, button_none, 1, 0},
    {5, 44, 195, 62, GB_ON_MOUSE_UP, select_item, button_none, 2, 0},
    {5, 64, 195, 82, GB_ON_MOUSE_UP, select_item, button_none, 3, 0},
    {5, 84, 195, 102, GB_ON_MOUSE_UP, select_item, button_none, 4, 0},
    {5, 104, 195, 122, GB_ON_MOUSE_UP, select_item, button_none, 5, 0},
    {5, 124, 195, 142, GB_ON_MOUSE_UP, select_item, button_none, 6, 0},
    {5, 144, 195, 162, GB_ON_MOUSE_UP, select_item, button_none, 7, 0},
    {5, 164, 195, 182, GB_ON_MOUSE_UP, select_item, button_none, 8, 0},
    {5, 184, 195, 202, GB_ON_MOUSE_UP, select_item, button_none, 9, 0},
    {5, 204, 195, 222, GB_ON_MOUSE_UP, select_item, button_none, 10, 0},
    {5, 224, 195, 242, GB_ON_MOUSE_UP, select_item, button_none, 11, 0},
    {5, 244, 195, 262, GB_ON_MOUSE_UP, select_item, button_none, 12, 0},
    {5, 264, 195, 282, GB_ON_MOUSE_UP, select_item, button_none, 13, 0},
    {5, 284, 195, 302, GB_ON_MOUSE_UP, select_item, button_none, 14, 0},
    {5, 304, 195, 322, GB_ON_MOUSE_UP, select_item, button_none, 15, 0},
    {5, 324, 195, 342, GB_ON_MOUSE_UP, select_item, button_none, 16, 0},
    {5, 344, 195, 362, GB_ON_MOUSE_UP, select_item, button_none, 17, 0},
    {5, 364, 195, 382, GB_ON_MOUSE_UP, select_item, button_none, 18, 0},
    {5, 384, 195, 402, GB_ON_MOUSE_UP, select_item, button_none, 19, 0},
};

static generic_button buttons_list2[MAX_ITEMS_PER_LIST] = {
    {205, 4, 395, 22, GB_ON_MOUSE_UP, select_item, button_none, 0, 1},
    {205, 24, 395, 42, GB_ON_MOUSE_UP, select_item, button_none, 1, 1},
    {205, 44, 395, 62, GB_ON_MOUSE_UP, select_item, button_none, 2, 1},
    {205, 64, 395, 82, GB_ON_MOUSE_UP, select_item, button_none, 3, 1},
    {205, 84, 395, 102, GB_ON_MOUSE_UP, select_item, button_none, 4, 1},
    {205, 104, 395, 122, GB_ON_MOUSE_UP, select_item, button_none, 5, 1},
    {205, 124, 395, 142, GB_ON_MOUSE_UP, select_item, button_none, 6, 1},
    {205, 144, 395, 162, GB_ON_MOUSE_UP, select_item, button_none, 7, 1},
    {205, 164, 395, 182, GB_ON_MOUSE_UP, select_item, button_none, 8, 1},
    {205, 184, 395, 202, GB_ON_MOUSE_UP, select_item, button_none, 9, 1},
    {205, 204, 395, 222, GB_ON_MOUSE_UP, select_item, button_none, 10, 1},
    {205, 224, 395, 242, GB_ON_MOUSE_UP, select_item, button_none, 11, 1},
    {205, 244, 395, 262, GB_ON_MOUSE_UP, select_item, button_none, 12, 1},
    {205, 264, 395, 282, GB_ON_MOUSE_UP, select_item, button_none, 13, 1},
    {205, 284, 395, 302, GB_ON_MOUSE_UP, select_item, button_none, 14, 1},
    {205, 304, 395, 322, GB_ON_MOUSE_UP, select_item, button_none, 15, 1},
    {205, 324, 395, 342, GB_ON_MOUSE_UP, select_item, button_none, 16, 1},
    {205, 344, 395, 362, GB_ON_MOUSE_UP, select_item, button_none, 17, 1},
    {205, 364, 395, 382, GB_ON_MOUSE_UP, select_item, button_none, 18, 1},
    {205, 384, 395, 402, GB_ON_MOUSE_UP, select_item, button_none, 19, 1},
};

static struct {
    int x;
    int y;
    int num_items;
    int group;
    void (*callback)(int);
    int focus_button_id;
} data;

static void init(int x, int y, int num_items, int group, void (*callback)(int))
{
    data.x = x;
    data.y = y;
    data.num_items = num_items;
    data.group = group;
    data.callback = callback;
}

static int items_in_first_list(void)
{
    return data.num_items / 2 + data.num_items % 2;
}

static void draw_foreground(void)
{
    if (data.num_items > MAX_ITEMS_PER_LIST) {
        int max_first = items_in_first_list();
        outer_panel_draw(data.x, data.y, 26, (20 * max_first + 24) / 16);
        for (int i = 0; i < max_first; i++) {
            color_t color = COLOR_BLACK;
            if (i + 1 == data.focus_button_id) {
                color = COLOR_BLUE;
            }
            lang_text_draw_centered_colored(data.group, i, data.x + 5, data.y + 11 + 20 * i, 190, FONT_NORMAL_PLAIN, color);
        }
        for (int i = 0; i < data.num_items - max_first; i++) {
            color_t color = COLOR_BLACK;
            if (max_first + i + 1 == data.focus_button_id) {
                color = COLOR_BLUE;
            }
            lang_text_draw_centered_colored(data.group, i + max_first, data.x + 205, data.y + 11 + 20 * i, 190, FONT_NORMAL_PLAIN, color);
        }
    } else {
        outer_panel_draw(data.x, data.y, 13, (20 * data.num_items + 24) / 16);
        for (int i = 0; i < data.num_items; i++) {
            color_t color = COLOR_BLACK;
            if (i + 1 == data.focus_button_id) {
                color = COLOR_BLUE;
            }
            lang_text_draw_centered_colored(data.group, i, data.x + 5, data.y + 11 + 20 * i, 190, FONT_NORMAL_PLAIN, color);
        }
    }
}

static void handle_mouse(const mouse *m)
{
    if (m->right.went_down) {
        window_go_back();
    }
    if (data.num_items > MAX_ITEMS_PER_LIST) {
        int items_first = items_in_first_list();
        if (!generic_buttons_handle_mouse(m, data.x, data.y, buttons_list1, items_first, &data.focus_button_id)) {
            int second_id = 0;
            generic_buttons_handle_mouse(m, data.x, data.y, buttons_list2, data.num_items - items_first, &second_id);
            if (second_id > 0) {
                data.focus_button_id = second_id + MAX_ITEMS_PER_LIST;
            }
        }
    } else {
        generic_buttons_handle_mouse(m, data.x, data.y, buttons_list1, data.num_items, &data.focus_button_id);
    }
}

void select_item(int id, int list_id)
{
    window_go_back();
    if (list_id == 0) {
        data.callback(id);
    } else {
        data.callback(id + items_in_first_list());
    }
}

void window_select_list_show(int x, int y, int num_items, int group, void (*callback)(int))
{
    window_type window = {
        WINDOW_SELECT_LIST,
        0,
        draw_foreground,
        handle_mouse,
        0
    };
    init(x, y, num_items, group, callback);
    window_show(&window);
}
