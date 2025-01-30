#include "allowed_buildings.h"

#include "building/menu.h"
#include "building/properties.h"
#include "building/type.h"
#include "core/lang.h"
#include "core/string.h"
#include "graphics/button.h"
#include "graphics/color.h"
#include "graphics/graphics.h"
#include "graphics/grid_box.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/allowed_building.h"
#include "scenario/editor.h"
#include "window/editor/attributes.h"
#include "window/editor/map.h"

static void draw_allowed_building(const grid_box_item *item);
static void toggle_building(const grid_box_item *item);

typedef enum {
    ITEM_TYPE_NONE = 0,
    ITEM_TYPE_MENU = 1,
    ITEM_TYPE_BUILDING = 2
} item_type;

typedef struct {
    item_type type;
    build_menu_group parent;
    build_menu_group menu;
    building_type building;
} menu_item;

static grid_box_type allowed_building_list = {
    .x = 25,
    .y = 82,
    .width = 36 * BLOCK_SIZE,
    .height = 20 * BLOCK_SIZE,
    .item_height = 28,
    .item_margin.horizontal = 8,
    .item_margin.vertical = 2,
    .extend_to_hidden_scrollbar = 1,
    .draw_item = draw_allowed_building,
    .on_click = toggle_building
};

static struct {
    menu_item items[BUILD_MENU_MAX + BUILDING_TYPE_MAX];
    unsigned int total_items;
    void (*select_callback)(int);
    building_type selected_building;
} data;

static unsigned int count_menu_items(build_menu_group menu)
{
    unsigned int count = 0;
    unsigned int menu_items = building_menu_count_all_items(menu);

    for (unsigned int i = 0; i < menu_items; i++) {
        building_type type = building_menu_type(menu, i);
        build_menu_group submenu = building_menu_get_submenu_for_type(type);
        if (submenu) {
            if (submenu == menu) {
                continue;
            }
            count += count_menu_items(submenu);
        } else {
            count++;
        }
    }
    return count;
}

static void populate_menu_items(build_menu_group menu)
{
    unsigned int count = count_menu_items(menu);
    if (!count) {
        return;
    }
    data.items[data.total_items].type = ITEM_TYPE_MENU;
    data.items[data.total_items].menu = menu;

    // Top menu with single element - show the element directly
    if (count == 1 && !data.items[data.total_items].parent) {
        data.items[data.total_items].building = building_menu_type(menu, 0);
        data.total_items++;
        return;
    }

    data.total_items++;

    unsigned int menu_items = building_menu_count_all_items(menu);
    for (unsigned int i = 0; i < menu_items; i++) {
        building_type type = building_menu_type(menu, i);
        build_menu_group submenu = building_menu_get_submenu_for_type(type);
        if (submenu) {
            if (submenu == menu) {
                continue;
            }
            data.items[data.total_items].parent = menu;
            data.items[data.total_items].building = type;
            populate_menu_items(submenu);
        } else {
            data.items[data.total_items].parent = menu;
            data.items[data.total_items].type = ITEM_TYPE_BUILDING;
            data.items[data.total_items].building = type;
            data.total_items++;
        }
    }
}

static void populate_items(void)
{
    if (data.total_items) {
        return;
    }
    for (build_menu_group group = 0; group < BUILD_MENU_MAX; group++) {
        // Top level function: main menus only
        if (building_menu_is_submenu(group)) {
            break;
        }
        populate_menu_items(group);
    }
}

static void init(void (*on_select_callback)(int), int selected)
{
    data.select_callback = on_select_callback;
    data.selected_building = selected;
    populate_items();
    grid_box_init(&allowed_building_list, data.total_items);
}

static void draw_background(void)
{
    window_editor_map_draw_all();

    graphics_in_dialog();

    outer_panel_draw(16, 32, 38, 26);
    lang_text_draw_centered(44, 47, 26, 42, 608, FONT_LARGE_BLACK);
    lang_text_draw_centered(13, 3, 16, 424, 608, FONT_NORMAL_BLACK);

    graphics_reset_dialog();

    grid_box_request_refresh(&allowed_building_list);
}

static void draw_button(const uint8_t *name, building_type type, int x, int y, int width, int height, int is_focused)
{
    button_border_draw(x, y, width, height, is_focused);
    int allowed = scenario_allowed_building(type);
    font_t font = allowed || data.select_callback ? FONT_NORMAL_BLACK : FONT_NORMAL_PLAIN;
    color_t color = allowed || data.select_callback ? 0 : COLOR_FONT_RED;
    text_draw(name, x + 8, y + 8, font, color);
    if (!data.select_callback) {
        const uint8_t *text = lang_get_string(CUSTOM_TRANSLATION, TR_EDITOR_ALLOWED_BUILDINGS_NOT_ALLOWED - allowed);
        text_draw_right_aligned(text, x, y + 8, width - 8, font, color);
    } else if (type == data.selected_building) {
        lang_text_draw_right_aligned(CUSTOM_TRANSLATION, TR_SELECTED, x, y + 8, width - 8, FONT_NORMAL_BLACK);
    }
}

static void draw_allowed_building(const grid_box_item *item)
{
    const menu_item *current_menu = &data.items[item->index];
    if (current_menu->type == ITEM_TYPE_BUILDING) {
        int x_offset = 0;
        int y_offset = 0;
        if (building_menu_is_submenu(current_menu->parent)) {
            x_offset = 20;
            y_offset = -4;
        }
        draw_button(lang_get_building_type_string(current_menu->building), current_menu->building,
            item->x + x_offset, item->y + y_offset, item->width - x_offset, item->height, item->is_focused);
    } else if (current_menu->type == ITEM_TYPE_MENU) {
        if (building_menu_is_submenu(current_menu->menu)) {
            int width = text_draw(string_from_ascii(" -"), item->x + 4, item->y + 8, FONT_NORMAL_BLACK, 0);
            width += lang_text_draw(28, current_menu->building, item->x + 4 + width, item->y + 8, FONT_NORMAL_BLACK);
            text_draw(string_from_ascii(":"), item->x + 4 + width - 6, item->y + 8, FONT_NORMAL_BLACK, 0);
        } else {
            if (current_menu->building == BUILDING_NONE) {
                int width = lang_text_draw(68, current_menu->menu + 20, item->x + 4, item->y + 12, FONT_NORMAL_BLACK);
                text_draw(string_from_ascii(":"), item->x + 4 + width - 6, item->y + 12, FONT_NORMAL_BLACK, 0);
            } else {
                draw_button(lang_get_string(68, current_menu->menu + 20), current_menu->building,
                    item->x, item->y, item->width, item->height, item->is_focused);
            }
        }
    }
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    grid_box_draw(&allowed_building_list);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (grid_box_handle_input(&allowed_building_list, mouse_in_dialog(m), 1)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_go_back();
    }
}

static int can_toggle_item(const menu_item *item)
{
    if (item->type == ITEM_TYPE_BUILDING) {
        return 1;
    }
    return item->type == ITEM_TYPE_MENU && !item->parent && item->building != BUILDING_NONE;
}

void toggle_building(const grid_box_item *item)
{
    const menu_item *current_menu = &data.items[item->index];
    if (!can_toggle_item(current_menu)) {
        return;
    }
    if (data.select_callback) {
        data.select_callback(current_menu->building);
        window_go_back();
        return;
    }
    int allowed = scenario_allowed_building(current_menu->building);
    scenario_allowed_building_set(current_menu->building, allowed ^ 1);
    scenario_editor_set_as_unsaved();
    window_request_refresh();
}

void window_editor_allowed_buildings_show(void)
{
    init(0, 0);
    window_type window = {
        WINDOW_EDITOR_ALLOWED_BUILDINGS,
        draw_background,
        draw_foreground,
        handle_input
    };
    window_show(&window);
}

void window_editor_allowed_buildings_select(void (*on_select_callback)(int), int selected)
{
    init(on_select_callback, selected);
    window_type window = {
        WINDOW_EDITOR_ALLOWED_BUILDINGS,
        draw_background,
        draw_foreground,
        handle_input
    };
    window_show(&window);
}
