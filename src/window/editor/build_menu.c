#include "build_menu.h"

#include "city/view.h"
#include "editor/tool.h"
#include "graphics/generic_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/window.h"
#include "input/input.h"
#include "widget/map_editor.h"
#include "widget/sidebar_editor.h"
#include "window/editor/map.h"

static void button_menu_item(int index, int param2);

static generic_button build_menu_buttons[] = {
    {0, 0, 160, 20, button_menu_item, button_none, 0, 0},
    {0, 24, 160, 20, button_menu_item, button_none, 1, 0},
    {0, 48, 160, 20, button_menu_item, button_none, 2, 0},
    {0, 72, 160, 20, button_menu_item, button_none, 3, 0},
    {0, 96, 160, 20, button_menu_item, button_none, 4, 0},
    {0, 120, 160, 20, button_menu_item, button_none, 5, 0},
    {0, 144, 160, 20, button_menu_item, button_none, 6, 0},
    {0, 168, 160, 20, button_menu_item, button_none, 7, 0},
    {0, 192, 160, 20, button_menu_item, button_none, 8, 0},
    {0, 216, 160, 20, button_menu_item, button_none, 9, 0},
    {0, 240, 160, 20, button_menu_item, button_none, 10, 0},
    {0, 264, 160, 20, button_menu_item, button_none, 11, 0},
    {0, 288, 160, 20, button_menu_item, button_none, 12, 0},
    {0, 312, 160, 20, button_menu_item, button_none, 13, 0},
    {0, 336, 160, 20, button_menu_item, button_none, 14, 0}
};

static const int Y_MENU_OFFSETS[16] = {
    0, 322, 306, 274, 258, 226, 210, 178,
    162, 130, 114, 82, 66, 34, 18, -30,
};

#define MAX_ITEMS_PER_MENU 15
static const int MENU_TYPES[MENU_NUM_ITEMS][MAX_ITEMS_PER_MENU] = {
    {0, 1, 2, 3, 4, -1},
    {5, 6, -1},
    {7, 8, 9, -1},
    {10, 11, 12, 13, 14, 15, 16, 17, -1},
    {18, 19, -1},
    {20, 21, 22, -1},
    {23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, -1},
};

static struct {
    int selected_submenu;
    int num_items;
    int y_offset;

    int focus_button_id;
} data;

static int count_items(int submenu)
{
    int count = 0;
    for (int i = 0; i < MAX_ITEMS_PER_MENU && MENU_TYPES[submenu][i] >= 0; i++) {
        count++;
    }
    return count;
}

static void init(int submenu)
{
    data.selected_submenu = submenu;
    data.num_items = count_items(submenu);
    data.y_offset = Y_MENU_OFFSETS[data.num_items];
}

static void draw_background(void)
{
    window_editor_map_draw_panels();
}

static int get_sidebar_x_offset(void)
{
    int view_x, view_y, view_width, view_height;
    city_view_get_unscaled_viewport(&view_x, &view_y, &view_width, &view_height);
    return view_x + view_width;
}

static void draw_menu_buttons(void)
{
    int x_offset = get_sidebar_x_offset();
    for (int i = 0; i < data.num_items; i++) {
        label_draw(x_offset - 170, data.y_offset + 110 + 24 * i, 10, data.focus_button_id == i + 1 ? 1 : 2);
        lang_text_draw_centered(48, MENU_TYPES[data.selected_submenu][i], x_offset - 170, data.y_offset + 113 + 24 * i, 160, FONT_NORMAL_GREEN);
    }
}

static void draw_foreground(void)
{
    window_editor_map_draw();
    draw_menu_buttons();
}

static int handle_build_submenu(const mouse *m)
{
    return generic_buttons_handle_mouse(
        m, get_sidebar_x_offset() - 170, data.y_offset + 110,
               build_menu_buttons, data.num_items, &data.focus_button_id);
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (handle_build_submenu(m) ||
        widget_sidebar_editor_handle_mouse_build_menu(m)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_editor_map_show();
    }
}

static void button_menu_item(int index, int param2)
{
    widget_map_editor_clear_current_tile();

    switch (data.selected_submenu) {
        case MENU_BRUSH_SIZE:
            editor_tool_set_brush_size(index + 1);
            break;
        case MENU_ELEVATION:
            switch (index) {
                case 0: editor_tool_set_type(TOOL_RAISE_LAND); break;
                case 1: editor_tool_set_type(TOOL_LOWER_LAND); break;
                case 2: editor_tool_set_type(TOOL_ACCESS_RAMP); break;
            }
            break;
        case MENU_PEOPLE_POINTS:
            switch (index) {
                case 0: editor_tool_set_type(TOOL_ENTRY_POINT); break;
                case 1: editor_tool_set_type(TOOL_EXIT_POINT); break;
            }
            break;
        case MENU_RIVER_POINTS:
            switch (index) {
                case 0: editor_tool_set_type(TOOL_RIVER_ENTRY_POINT); break;
                case 1: editor_tool_set_type(TOOL_RIVER_EXIT_POINT); break;
            }
            break;
        case MENU_NATIVE_BUILDINGS:
            switch (index) {
                case 0: editor_tool_set_type(TOOL_NATIVE_HUT); break;
                case 1: editor_tool_set_type(TOOL_NATIVE_CENTER); break;
                case 2: editor_tool_set_type(TOOL_NATIVE_FIELD); break;
            }
            break;
        case MENU_INVASION_POINTS:
            editor_tool_set_with_id(TOOL_INVASION_POINT, index);
            break;
        case MENU_ANIMAL_POINTS:
            if (index < 8) {
                editor_tool_set_with_id(TOOL_FISHING_POINT, index);
            } else {
                editor_tool_set_with_id(TOOL_HERD_POINT, index - 8);
            }
            break;
    }

    window_editor_map_show();
}

void window_editor_build_menu_show(int submenu)
{
    init(submenu);
    window_type window = {
        WINDOW_EDITOR_BUILD_MENU,
        draw_background,
        draw_foreground,
        handle_input
    };
    window_show(&window);
}
