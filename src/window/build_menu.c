#include "build_menu.h"

#include "building/construction.h"
#include "building/menu.h"
#include "building/model.h"
#include "city/view.h"
#include "graphics/generic_button.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "scenario/property.h"
#include "widget/sidebar.h"
#include "window/city.h"

#include "Data/State.h"

static void button_menu_index(int param1, int param2);
static void button_menu_item(int item);

static generic_button build_menu_buttons[] = {
    {0, 0, 256, 20, GB_IMMEDIATE, button_menu_index, button_none, 1, 0},
    {0, 24, 256, 44, GB_IMMEDIATE, button_menu_index, button_none, 2, 0},
    {0, 48, 256, 68, GB_IMMEDIATE, button_menu_index, button_none, 3, 0},
    {0, 72, 256, 92, GB_IMMEDIATE, button_menu_index, button_none, 4, 0},
    {0, 96, 256, 116, GB_IMMEDIATE, button_menu_index, button_none, 5, 0},
    {0, 120, 256, 140, GB_IMMEDIATE, button_menu_index, button_none, 6, 0},
    {0, 144, 256, 164, GB_IMMEDIATE, button_menu_index, button_none, 7, 0},
    {0, 168, 256, 188, GB_IMMEDIATE, button_menu_index, button_none, 8, 0},
    {0, 192, 256, 212, GB_IMMEDIATE, button_menu_index, button_none, 9, 0},
    {0, 216, 256, 236, GB_IMMEDIATE, button_menu_index, button_none, 10, 0},
    {0, 240, 256, 260, GB_IMMEDIATE, button_menu_index, button_none, 11, 0},
    {0, 264, 256, 284, GB_IMMEDIATE, button_menu_index, button_none, 12, 0},
    {0, 288, 256, 308, GB_IMMEDIATE, button_menu_index, button_none, 13, 0},
    {0, 312, 256, 332, GB_IMMEDIATE, button_menu_index, button_none, 14, 0},
    {0, 336, 256, 356, GB_IMMEDIATE, button_menu_index, button_none, 15, 0},
    {0, 360, 256, 380, GB_IMMEDIATE, button_menu_index, button_none, 16, 0},
    {0, 384, 256, 404, GB_IMMEDIATE, button_menu_index, button_none, 17, 0},
    {0, 408, 256, 428, GB_IMMEDIATE, button_menu_index, button_none, 18, 0},
    {0, 432, 256, 452, GB_IMMEDIATE, button_menu_index, button_none, 19, 0},
    {0, 456, 256, 476, GB_IMMEDIATE, button_menu_index, button_none, 20, 0},
    {0, 480, 256, 500, GB_IMMEDIATE, button_menu_index, button_none, 21, 0},
    {0, 504, 256, 524, GB_IMMEDIATE, button_menu_index, button_none, 22, 0},
    {0, 528, 256, 548, GB_IMMEDIATE, button_menu_index, button_none, 23, 0},
    {0, 552, 256, 572, GB_IMMEDIATE, button_menu_index, button_none, 24, 0},
    {0, 576, 256, 596, GB_IMMEDIATE, button_menu_index, button_none, 25, 0},
    {0, 600, 256, 620, GB_IMMEDIATE, button_menu_index, button_none, 26, 0},
    {0, 624, 256, 644, GB_IMMEDIATE, button_menu_index, button_none, 27, 0},
    {0, 648, 256, 668, GB_IMMEDIATE, button_menu_index, button_none, 28, 0},
    {0, 672, 256, 692, GB_IMMEDIATE, button_menu_index, button_none, 29, 0},
    {0, 696, 256, 716, GB_IMMEDIATE, button_menu_index, button_none, 30, 0},
};

static const int Y_MENU_OFFSETS[24] = {
    0, 322, 306, 274, 258, 226, 210, 178, 162, 130, 114,
    82, 66, 34, 18, -30, -46, -62, -78, -78, -94,
    -94, -110, -110
};

static struct {
    int selected_submenu;
    int num_items;
    int y_offset;

    int focus_button_id;
} data;

static int init(int submenu)
{
    data.selected_submenu = submenu;
    data.num_items = building_menu_count_items(submenu);
    data.y_offset = Y_MENU_OFFSETS[data.num_items];
    if (submenu == 0 || submenu == 1 || submenu == 2) {
        button_menu_item(0);
        return 0;
    } else {
        return 1;
    }
}

int window_build_menu_image()
{
    if (building_construction_type() == BUILDING_NONE) {
        return image_group(GROUP_PANEL_WINDOWS) + 12;
    }
    int image_base = image_group(GROUP_PANEL_WINDOWS);
    switch (data.selected_submenu) {
        default:
        case 0:
            return image_base;
        case 1:
            if (scenario_property_climate() == CLIMATE_DESERT) {
                return image_group(GROUP_PANEL_WINDOWS_DESERT);
            } else {
                return image_base + 11;
            }
        case 2:
            if (scenario_property_climate() == CLIMATE_DESERT) {
                return image_group(GROUP_PANEL_WINDOWS_DESERT) + 1;
            } else {
                return image_base + 10;
            }
        case 3:
            if (scenario_property_climate() == CLIMATE_DESERT) {
                return image_group(GROUP_PANEL_WINDOWS_DESERT) + 2;
            } else {
                return image_base + 3;
            }
        case 4:
            return image_base + 5;
        case 5:
            return image_base + 1;
        case 6:
            return image_base + 6;
        case 7:
            return image_base + 4;
        case 8:
            return image_base + 2;
        case 9:
            return image_base + 7;
        case 10:
            if (scenario_property_climate() == CLIMATE_DESERT) {
                return image_group(GROUP_PANEL_WINDOWS_DESERT) + 3;
            } else {
                return image_base + 8;
            }
        case 11:
            return image_base + 9;
    }
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

static void draw_menu_buttons()
{
    int x_offset = get_sidebar_x_offset();
    int item_index = -1;
    for (int i = 0; i < data.num_items; i++) {
        item_index = building_menu_next_index(data.selected_submenu, item_index);
        label_draw(x_offset - 266, data.y_offset + 110 + 24 * i, 16, data.focus_button_id == i + 1 ? 1 : 2);
        int type = building_menu_type(data.selected_submenu, item_index);
        lang_text_draw_centered(28, type, x_offset - 266, data.y_offset + 113 + 24 * i, 176, FONT_NORMAL_GREEN);
        if (type == BUILDING_DRAGGABLE_RESERVOIR) {
            type = BUILDING_RESERVOIR;
        }
        int cost = model_get_building(type)->cost;
        if (type == BUILDING_FORT) {
            cost = 0;
        }
        if (cost) {
            text_draw_money(cost, x_offset - 82, data.y_offset + 114 + 24 * i, FONT_NORMAL_GREEN);
        }
    }
}

static void draw_foreground()
{
    window_city_draw();
    draw_menu_buttons();
}

static int handle_build_submenu(const mouse *m)
{
    return generic_buttons_handle_mouse(
        m, get_sidebar_x_offset() - 258, data.y_offset + 110,
               build_menu_buttons, data.num_items, &data.focus_button_id);
}

static void handle_mouse(const mouse *m)
{
    if (m->right.went_up) {
        window_city_show();
        return;
    }

    if (!handle_build_submenu(m)) {
        widget_sidebar_handle_mouse_build_menu(m);
    }
}

static int button_index_to_submenu_item(int index)
{
    int item = -1;
    for (int i = 0; i <= index; i++) {
        item = building_menu_next_index(data.selected_submenu, item);
    }
    return item;
}

static void button_menu_index(int param1, int param2)
{
    button_menu_item(button_index_to_submenu_item(param1 - 1));
}

static void button_menu_item(int item)
{
    Data_State.map.current.gridOffset = 0;

    building_type type = building_menu_type(data.selected_submenu, item);
    building_construction_set_type(type);

    if (type == BUILDING_MENU_FARMS || type == BUILDING_MENU_RAW_MATERIALS ||
        type == BUILDING_MENU_WORKSHOPS || type == BUILDING_FORT ||
        type == BUILDING_MENU_SMALL_TEMPLES || type == BUILDING_MENU_LARGE_TEMPLES) {
        switch (type) {
            case BUILDING_MENU_FARMS:
                data.selected_submenu = 19;
                break;
            case BUILDING_MENU_RAW_MATERIALS:
                data.selected_submenu = 20;
                break;
            case BUILDING_MENU_WORKSHOPS:
                data.selected_submenu = 21;
                break;
            case BUILDING_MENU_SMALL_TEMPLES:
                data.selected_submenu = 22;
                break;
            case BUILDING_MENU_LARGE_TEMPLES:
                data.selected_submenu = 23;
                break;
            case BUILDING_FORT:
                data.selected_submenu = 24;
                break;
            default:
                break;
        }
        data.num_items = building_menu_count_items(data.selected_submenu);
        data.y_offset = Y_MENU_OFFSETS[data.num_items];
        building_construction_clear_type();
    } else {
        window_city_show();
    }
}

void window_build_menu_show(int submenu)
{
    if (init(submenu)) {
        window_type window = {
            WINDOW_BUILD_MENU,
            draw_background,
            draw_foreground,
            handle_mouse,
            0
        };
        window_show(&window);
    }
}
