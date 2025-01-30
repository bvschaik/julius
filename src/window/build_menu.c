#include "build_menu.h"

#include "assets/assets.h"
#include "building/construction.h"
#include "building/menu.h"
#include "building/monument.h"
#include "building/model.h"
#include "building/rotation.h"
#include "city/view.h"
#include "city/warning.h"
#include "core/lang.h"
#include "core/string.h"
#include "graphics/generic_button.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/property.h"
#include "translation/translation.h"
#include "widget/city.h"
#include "widget/sidebar/city.h"
#include "window/city.h"

#define MENU_X_OFFSET 298
#define MENU_Y_OFFSET 110
#define MENU_ITEM_HEIGHT 24
#define MENU_ITEM_WIDTH 208
#define MENU_CLICK_MARGIN 20
#define MENU_TEXT_X_OFFSET 8


#define MENU_ICON_WIDTH 14
#define MENU_ICON_X_OFFSET 3
#define MENU_ICON_Y_OFFSET 3
#define MENU_ITEM_MONEY_OFFSET 82

#define TOOLTIP_TEXT_LENGTH 1000

static uint8_t tooltip_text[TOOLTIP_TEXT_LENGTH];

static void button_menu_index(const generic_button *button);
static void button_menu_item(int item);

static generic_button build_menu_buttons[] = {
    {0, 0, 290, 20, button_menu_index, 0, 1},
    {0, 24, 290, 20, button_menu_index, 0, 2},
    {0, 48, 290, 20, button_menu_index, 0, 3},
    {0, 72, 290, 20, button_menu_index, 0, 4},
    {0, 96, 290, 20, button_menu_index, 0, 5},
    {0, 120, 290, 20, button_menu_index, 0, 6},
    {0, 144, 290, 20, button_menu_index, 0, 7},
    {0, 168, 290, 20, button_menu_index, 0, 8},
    {0, 192, 290, 20, button_menu_index, 0, 9},
    {0, 216, 290, 20, button_menu_index, 0, 10},
    {0, 240, 290, 20, button_menu_index, 0, 11},
    {0, 264, 290, 20, button_menu_index, 0, 12},
    {0, 288, 290, 20, button_menu_index, 0, 13},
    {0, 312, 290, 20, button_menu_index, 0, 14},
    {0, 336, 290, 20, button_menu_index, 0, 15},
    {0, 360, 290, 20, button_menu_index, 0, 16},
    {0, 384, 290, 20, button_menu_index, 0, 17},
    {0, 408, 290, 20, button_menu_index, 0, 18},
    {0, 432, 290, 20, button_menu_index, 0, 19},
    {0, 456, 290, 20, button_menu_index, 0, 20},
    {0, 480, 290, 20, button_menu_index, 0, 21},
    {0, 504, 290, 20, button_menu_index, 0, 22},
    {0, 528, 290, 20, button_menu_index, 0, 23},
    {0, 552, 290, 20, button_menu_index, 0, 24},
    {0, 576, 290, 20, button_menu_index, 0, 25},
    {0, 600, 290, 20, button_menu_index, 0, 26},
    {0, 624, 290, 20, button_menu_index, 0, 27},
    {0, 648, 290, 20, button_menu_index, 0, 28},
    {0, 672, 290, 20, button_menu_index, 0, 29},
    {0, 696, 290, 20, button_menu_index, 0, 30},
};

static const int Y_MENU_OFFSETS[] = {
    0, 322, 306, 274, 258, 226, 210, 178, 162, 130, 114,
    82, 66, 34, 18, -30, -46, -62, -78, -78, -94,
    -94, -110, -110,
    0, 0, 0, 0, 0, 0
};

static struct {
    build_menu_group selected_submenu;
    unsigned int num_items;
    int y_offset;
    unsigned int focus_button_id;
} data = { SUBMENU_NONE };

static int init(build_menu_group submenu)
{
    data.selected_submenu = submenu;
    data.num_items = building_menu_count_items(submenu);
    data.y_offset = Y_MENU_OFFSETS[data.num_items];
    if (submenu == BUILD_MENU_VACANT_HOUSE ||
        submenu == BUILD_MENU_CLEAR_LAND) {
        button_menu_item(0);
        return 0;
    } else {
        return 1;
    }
}

int window_build_menu_image(void)
{
    building_type type = building_construction_type();
    int image_base = image_group(GROUP_PANEL_WINDOWS);
    if (type == BUILDING_NONE) {
        return image_base + 12;
    }
    switch (building_menu_for_type(type)) {
        default:
        case BUILD_MENU_VACANT_HOUSE:
            return image_base;
        case BUILD_MENU_CLEAR_LAND:
            if (scenario_property_climate() == CLIMATE_DESERT) {
                return image_group(GROUP_PANEL_WINDOWS_DESERT);
            } else {
                return image_base + 11;
            }
        case BUILD_MENU_ROAD:
            if (scenario_property_climate() == CLIMATE_DESERT) {
                return image_group(GROUP_PANEL_WINDOWS_DESERT) + 1;
            } else {
                return image_base + 10;
            }
        case BUILD_MENU_WATER:
            if (scenario_property_climate() == CLIMATE_DESERT) {
                return image_group(GROUP_PANEL_WINDOWS_DESERT) + 2;
            } else {
                return image_base + 3;
            }
        case BUILD_MENU_HEALTH:
            return image_base + 5;
        case BUILD_MENU_TEMPLES:
        case BUILD_MENU_SMALL_TEMPLES:
        case BUILD_MENU_LARGE_TEMPLES:
            return image_base + 1;
        case BUILD_MENU_EDUCATION:
            return image_base + 6;
        case BUILD_MENU_ENTERTAINMENT:
            return image_base + 4;
        case BUILD_MENU_ADMINISTRATION:
            return image_base + 2;
        case BUILD_MENU_ENGINEERING:
            return image_base + 7;
        case BUILD_MENU_SECURITY:
        case BUILD_MENU_FORTS:
            if (scenario_property_climate() == CLIMATE_DESERT) {
                return image_group(GROUP_PANEL_WINDOWS_DESERT) + 3;
            } else {
                return image_base + 8;
            }
        case BUILD_MENU_INDUSTRY:
        case BUILD_MENU_FARMS:
        case BUILD_MENU_RAW_MATERIALS:
        case BUILD_MENU_WORKSHOPS:
            return image_base + 9;
    }
}

static void draw_background(void)
{
    window_city_draw_panels();
}

static int get_sidebar_x_offset(void)
{
    int view_x, view_y, view_width, view_height;
    city_view_get_viewport(&view_x, &view_y, &view_width, &view_height);
    return view_x + view_width;
}

static int is_auto_cycle_button(building_type type)
{
    return (type == BUILDING_MENU_SMALL_TEMPLES && data.selected_submenu == BUILD_MENU_SMALL_TEMPLES) ||
        (type == BUILDING_MENU_LARGE_TEMPLES && data.selected_submenu == BUILD_MENU_LARGE_TEMPLES) ||
        (type == BUILDING_MENU_SHRINES && data.selected_submenu == BUILD_MENU_SHRINES) ||
        (type == BUILDING_MENU_TREES && data.selected_submenu == BUILD_MENU_TREES) ||
        (type == BUILDING_MENU_PATHS && data.selected_submenu == BUILD_MENU_PATHS) ||
        (type == BUILDING_MENU_GARDENS && data.selected_submenu == BUILD_MENU_GARDENS);
}

static void draw_menu_buttons(void)
{
    int x_offset = get_sidebar_x_offset();
    int item_index = -1;
    int item_x_align = x_offset - MENU_X_OFFSET;
    for (unsigned int i = 0; i < data.num_items; i++) {
        item_index = building_menu_next_index(data.selected_submenu, item_index);
        label_draw(item_x_align, data.y_offset + MENU_Y_OFFSET + MENU_ITEM_HEIGHT * i, 18,
            data.focus_button_id == i + 1 ? 1 : 2);
        int type = building_menu_type(data.selected_submenu, item_index);

        if (is_auto_cycle_button(type)) {
            text_draw_centered(translation_for(TR_AUTO_CYCLE_TEMPLES),
                item_x_align + MENU_TEXT_X_OFFSET, data.y_offset + MENU_Y_OFFSET + 4 + MENU_ITEM_HEIGHT * i,
                MENU_ITEM_WIDTH, FONT_NORMAL_GREEN, 0);
            lang_text_draw_centered(18, 5 - building_construction_is_auto_cycling(), x_offset - MENU_ITEM_MONEY_OFFSET,
                data.y_offset + MENU_Y_OFFSET + 4 + MENU_ITEM_HEIGHT * i, MENU_ITEM_MONEY_OFFSET,
                FONT_NORMAL_GREEN);
            continue;
        }
        
        lang_text_draw_centered(28, type, item_x_align + MENU_TEXT_X_OFFSET, data.y_offset + MENU_Y_OFFSET + 4 + MENU_ITEM_HEIGHT * i,
                MENU_ITEM_WIDTH, FONT_NORMAL_GREEN);
        if (type == BUILDING_DRAGGABLE_RESERVOIR) {
            type = BUILDING_RESERVOIR;
        }
        int cost = model_get_building(type)->cost;
        if (type == BUILDING_FORT) {
            cost = 0;
        }
        if (type == BUILDING_MENU_GRAND_TEMPLES) {
            cost = 0;
        }
        if (cost) {
            text_draw_money(cost, x_offset - MENU_ITEM_MONEY_OFFSET,
                data.y_offset + MENU_Y_OFFSET + 4 + MENU_ITEM_HEIGHT * i,
                FONT_NORMAL_GREEN);
        }

        int icons_drawn = 0;
        if (building_rotation_type_has_rotations(type)) {
            int image_id = assets_get_image_id("UI", "Rotate Build Icon");
            image_draw(image_id, item_x_align + icons_drawn * MENU_ICON_WIDTH + MENU_ICON_X_OFFSET,
                data.y_offset + MENU_Y_OFFSET + MENU_ICON_Y_OFFSET + MENU_ITEM_HEIGHT * i, COLOR_MASK_NONE, SCALE_NONE);
            icons_drawn++;
        }

        if (building_monument_type_is_monument(type)) {
            int image_id = assets_get_image_id("UI", "Monument Build Icon");
            image_draw(image_id, item_x_align + icons_drawn * MENU_ICON_WIDTH + MENU_ICON_X_OFFSET,
                data.y_offset + MENU_Y_OFFSET + MENU_ICON_Y_OFFSET + MENU_ITEM_HEIGHT * i, COLOR_MASK_NONE, SCALE_NONE);
            icons_drawn++;
        }
    }
}

static void draw_foreground(void)
{
    window_city_draw();
    draw_menu_buttons();
}

static int click_outside_menu(const mouse *m, int x_offset)
{
    return m->left.went_up &&
        (m->x < x_offset - MENU_X_OFFSET - MENU_CLICK_MARGIN ||
        m->x > x_offset + MENU_CLICK_MARGIN ||
        m->y < data.y_offset + MENU_Y_OFFSET - MENU_CLICK_MARGIN ||
        m->y > data.y_offset + MENU_Y_OFFSET + MENU_CLICK_MARGIN + MENU_ITEM_HEIGHT * (int) data.num_items);
}

static int handle_build_submenu(const mouse *m)
{
    return generic_buttons_handle_mouse(
        m, get_sidebar_x_offset() - MENU_X_OFFSET, data.y_offset + MENU_Y_OFFSET,
        build_menu_buttons, data.num_items, &data.focus_button_id);
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (handle_build_submenu(m) ||
        widget_sidebar_city_handle_mouse_build_menu(m)) {
        return;
    }
    if (input_go_back_requested(m, h) || click_outside_menu(m, get_sidebar_x_offset())) {
        data.selected_submenu = SUBMENU_NONE;
        window_city_show();
        return;
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

static void button_menu_index(const generic_button *button)
{
    int index = button->parameter1 - 1;
    button_menu_item(button_index_to_submenu_item(index));
}

static int set_submenu_for_type(building_type type)
{
    build_menu_group current_menu = data.selected_submenu;
    build_menu_group new_menu = building_menu_get_submenu_for_type(type);
    if (!new_menu) {
        return 0;
    }
    data.selected_submenu = new_menu;
    return current_menu != data.selected_submenu;
}

static void button_menu_item(int item)
{
    widget_city_clear_current_tile();

    building_type type = building_menu_type(data.selected_submenu, item);

    if (is_auto_cycle_button(type)) {
        building_construction_toggle_auto_cycle();
        window_invalidate();
        return;
    }
    building_construction_set_type(type);

    if (set_submenu_for_type(type)) {
        data.num_items = building_menu_count_items(data.selected_submenu);
        data.y_offset = Y_MENU_OFFSETS[data.num_items];
        building_construction_clear_type();
        window_invalidate();
    } else {
        data.selected_submenu = SUBMENU_NONE;
        window_city_show();
    }
}

static inline int remanining_length(const uint8_t *index)
{
    return TOOLTIP_TEXT_LENGTH - (int) (index - tooltip_text);
}

static void generate_tooltip_text_for_monument(building_type monument)
{
    int phases = building_monument_phases(monument) - 1;
    uint8_t *index = tooltip_text;
    index += string_from_int(index, phases, 0);
    index = string_copy(lang_get_string(CUSTOM_TRANSLATION, TR_TOOLTIP_MONUMENT_PHASE + (phases != 1 ? 1 : 0)),
        index, remanining_length(index));
    index = string_copy(lang_get_string(CUSTOM_TRANSLATION, TR_TOOLTIP_MONUMENT_RESOURCE_REQUIREMENTS),
        index, remanining_length(index));

    int has_listed_resource = 0;

    for (resource_type r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        int amount = 0;
        for (int phase = 1; phase <= phases; phase++) {
            amount += building_monument_resources_needed_for_monument_type(monument, r, phase);
        }
        if (amount) {
            if (has_listed_resource) {
                index = string_copy(string_from_ascii(", "), index, remanining_length(index));
            }
            index += string_from_int(index, amount, 0);
            index = string_copy(string_from_ascii(" "), index, remanining_length(index));
            index = string_copy(resource_get_data(r)->text, index, remanining_length(index));
            has_listed_resource = 1;
        }
    }
}

static void get_tooltip(tooltip_context *c)
{
    if (!data.focus_button_id || data.selected_submenu == SUBMENU_NONE ||
        mouse_get()->x > get_sidebar_x_offset() - MENU_X_OFFSET + MENU_ICON_WIDTH * 3) {
        return;
    }

    int button = button_index_to_submenu_item(data.focus_button_id - 1);
    building_type type = building_menu_type(data.selected_submenu, button);

    if (building_monument_type_is_monument(type)) {
        generate_tooltip_text_for_monument(type);
        c->precomposed_text = tooltip_text;
        c->type = TOOLTIP_BUTTON;
    }
}

void window_build_menu_show(int submenu)
{
    if (submenu == SUBMENU_NONE || submenu == data.selected_submenu) {
        window_build_menu_hide();
        return;
    }
    if (init(submenu)) {
        window_type window = {
            WINDOW_BUILD_MENU,
            draw_background,
            draw_foreground,
            handle_input,
            get_tooltip
        };
        window_show(&window);
    }
}

void window_build_menu_hide(void)
{
    data.selected_submenu = SUBMENU_NONE;
    window_city_show();
}
