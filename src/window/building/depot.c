#include "depot.h"

#include "building/building.h"
#include "building/count.h"
#include "building/storage.h"
#include "building/warehouse.h"
#include "figure/figure.h"
#include "graphics/generic_button.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/scrollbar.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "translation/translation.h"
#include "window/building_info.h"

static void order_set_source(int index, int param2);
static void order_set_destination(int index, int param2);
static void order_set_resource(int index, int param2);
static void order_set_condition_type(int index, int param2);
static void order_set_condition_threshold(int index, int param2);
static void set_order_resource(int depot_building_id, int resource_id);

static void setup_for_selected_depot(building_info_context *c, int offset);

#define DEPOT_BUTTONS_X_OFFSET 32
#define DEPOT_BUTTONS_Y_OFFSET 204
#define ROW_HEIGHT 22
#define ROW_HEIGHT_RESOURCE 26
#define MAX_VISIBLE_ROWS 15

static struct {
    int focus_button_id;
    int orders_focus_button_id;
    int resource_focus_button_id;
    int storage_building_focus_button_id;
    int depot_resource_focus_button_id;
    int depot_building_id;
    resource_type target_resource_id;
} data;

static void on_scroll(void);
static scrollbar_type scrollbar = { 0, 0, ROW_HEIGHT * MAX_VISIBLE_ROWS, 432, MAX_VISIBLE_ROWS, on_scroll, 0, 4 };

// field values will be overwritten in window_building_draw_depot_select_source_destination
static generic_button depot_select_storage_buttons[] = {
    {0, 0, 0, ROW_HEIGHT, button_none, button_none, 0, 0},
    {0, 0, 0, ROW_HEIGHT, button_none, button_none, 0, 0},
    {0, 0, 0, ROW_HEIGHT, button_none, button_none, 0, 0},
    {0, 0, 0, ROW_HEIGHT, button_none, button_none, 0, 0},
    {0, 0, 0, ROW_HEIGHT, button_none, button_none, 0, 0},
    {0, 0, 0, ROW_HEIGHT, button_none, button_none, 0, 0},
    {0, 0, 0, ROW_HEIGHT, button_none, button_none, 0, 0},
    {0, 0, 0, ROW_HEIGHT, button_none, button_none, 0, 0},
    {0, 0, 0, ROW_HEIGHT, button_none, button_none, 0, 0},
    {0, 0, 0, ROW_HEIGHT, button_none, button_none, 0, 0},
    {0, 0, 0, ROW_HEIGHT, button_none, button_none, 0, 0},
    {0, 0, 0, ROW_HEIGHT, button_none, button_none, 0, 0},
    {0, 0, 0, ROW_HEIGHT, button_none, button_none, 0, 0},
    {0, 0, 0, ROW_HEIGHT, button_none, button_none, 0, 0},
    {0, 0, 0, ROW_HEIGHT, button_none, button_none, 0, 0},
};

static generic_button depot_select_resource_buttons[] = {
    {18, 0, 214, ROW_HEIGHT_RESOURCE, set_order_resource, button_none, 0, RESOURCE_MIN},
    {232, 0, 214, ROW_HEIGHT_RESOURCE, set_order_resource, button_none, 0, RESOURCE_MIN + 1},
    {18, ROW_HEIGHT_RESOURCE * 1, 214, ROW_HEIGHT_RESOURCE, set_order_resource, button_none, 0, RESOURCE_MIN + 2},
    {232, ROW_HEIGHT_RESOURCE * 1, 214, ROW_HEIGHT_RESOURCE, set_order_resource, button_none, 0, RESOURCE_MIN + 3},
    {18, ROW_HEIGHT_RESOURCE * 2, 214, ROW_HEIGHT_RESOURCE, set_order_resource, button_none, 0, RESOURCE_MIN + 4},
    {232, ROW_HEIGHT_RESOURCE * 2, 214, ROW_HEIGHT_RESOURCE, set_order_resource, button_none, 0, RESOURCE_MIN + 5},
    {18, ROW_HEIGHT_RESOURCE * 3, 214, ROW_HEIGHT_RESOURCE, set_order_resource, button_none, 0, RESOURCE_MIN + 6},
    {232, ROW_HEIGHT_RESOURCE * 3, 214, ROW_HEIGHT_RESOURCE, set_order_resource, button_none, 0, RESOURCE_MIN + 7},
    {18, ROW_HEIGHT_RESOURCE * 4, 214, ROW_HEIGHT_RESOURCE, set_order_resource, button_none, 0, RESOURCE_MIN + 8},
    {232, ROW_HEIGHT_RESOURCE * 4, 214, ROW_HEIGHT_RESOURCE, set_order_resource, button_none, 0, RESOURCE_MIN + 9},
    {18, ROW_HEIGHT_RESOURCE * 5, 214, ROW_HEIGHT_RESOURCE, set_order_resource, button_none, 0, RESOURCE_MIN + 10},
    {232, ROW_HEIGHT_RESOURCE * 5, 214, ROW_HEIGHT_RESOURCE, set_order_resource, button_none, 0, RESOURCE_MIN + 11},
    {18, ROW_HEIGHT_RESOURCE * 6, 214, ROW_HEIGHT_RESOURCE, set_order_resource, button_none, 0, RESOURCE_MIN + 12},
    {232, ROW_HEIGHT_RESOURCE * 6, 214, ROW_HEIGHT_RESOURCE, set_order_resource, button_none, 0, RESOURCE_MIN + 13},
    {18, ROW_HEIGHT_RESOURCE * 7, 214, ROW_HEIGHT_RESOURCE, set_order_resource, button_none, 0, RESOURCE_MIN + 14},
    {232, ROW_HEIGHT_RESOURCE * 7, 214, ROW_HEIGHT_RESOURCE, set_order_resource, button_none, 0, RESOURCE_MIN + 15},
    {18, ROW_HEIGHT_RESOURCE * 8, 214, ROW_HEIGHT_RESOURCE, set_order_resource, button_none, 0, RESOURCE_MIN + 16},
};

static generic_button depot_order_buttons[] = {
    {100, 0, 26, 26, order_set_resource, button_none, 1, 0},
    {100, 56, 284, 22, order_set_source, button_none, 2, 0},
    {100, 82, 284, 22, order_set_destination, button_none, 3, 0},
    {100, 30, 284, 22, order_set_condition_type, button_none, 4, 0},
    {384, 30, 32, 22, order_set_condition_threshold, button_none, 5, 0},
};

static void setup_for_selected_depot(building_info_context *c, int offset)
{
    data.depot_building_id = c->building_id;

    building *b = building_get(data.depot_building_id);
    if (!b->data.depot.current_order.resource_type) {
        b->data.depot.current_order.resource_type = RESOURCE_WHEAT;
    }
    data.target_resource_id = b->data.depot.current_order.resource_type;

    for (int i = 0; i < MAX_VISIBLE_ROWS; i++) {
        depot_select_storage_buttons[i].parameter1 = data.depot_building_id;
        depot_select_storage_buttons[i].parameter2 = 0;
    }

    int button_index = 0;
    int storage_array_size = building_storage_get_array_size();
    for (int i = offset; i < storage_array_size; i++) {
        if (button_index >= MAX_VISIBLE_ROWS) {
            break;
        }
        int building_id = building_storage_get_array_entry(i)->building_id;
        if (building_id) {
            building *store_building = building_get(building_id);
            if (store_building && building_is_active(store_building)
                && building_storage_resource_max_storable(store_building, data.target_resource_id) > 0) {
                depot_select_storage_buttons[button_index].parameter2 = building_id;
                button_index++;
            }
        }
    }
}

void window_building_depot_init(void)
{
    int total = building_storage_get_array_size();
    scrollbar_init(&scrollbar, 0, total);
}

static void on_scroll(void)
{
    window_request_refresh();
}

void window_building_draw_depot(building_info_context *c)
{
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 138);
    text_draw_centered(translation_for(TR_BUILDING_DEPOT),
        c->x_offset, c->y_offset + 12, 16 * c->width_blocks, FONT_LARGE_BLACK, 0);
}

void window_building_draw_depot_foreground(building_info_context *c)
{
    int offset = scrollbar.scroll_position;
    setup_for_selected_depot(c, offset);
    building *b = building_get(data.depot_building_id);
    building *src = building_get(b->data.depot.current_order.src_storage_id);
    building *dst = building_get(b->data.depot.current_order.dst_storage_id);

    int group_id = 120;
    int text_offset = 0;
    if (!b->has_road_access) {
        window_building_draw_description(c, 69, 25);
    } else if (b->num_workers <= 0) {
        window_building_draw_description(c, group_id, text_offset + 5);
    } else if (c->worker_percentage < 25) {
        window_building_draw_description(c, group_id, text_offset + 10);
    } else if (c->worker_percentage < 50) {
        window_building_draw_description(c, group_id, text_offset + 9);
    } else if (c->worker_percentage < 75) {
        window_building_draw_description(c, group_id, text_offset + 8);
    } else if (c->worker_percentage < 100) {
        window_building_draw_description(c, group_id, text_offset + 7);
    } else {
        window_building_draw_description(c, CUSTOM_TRANSLATION, TR_BUILDING_DEPOT_DESC);
    }

    resource_type resource = data.target_resource_id;
    int x_offset = c->x_offset + DEPOT_BUTTONS_X_OFFSET;
    int y_offset = c->y_offset + DEPOT_BUTTONS_Y_OFFSET;

    text_draw(translation_for(TR_FIGURE_INFO_DEPOT_DELIVER), x_offset, y_offset + 8, FONT_SMALL_PLAIN, 0);
    depot_order_buttons[0].x = 100;
    int image_id = resource_get_data(resource)->image.icon;
    button_border_draw(x_offset + depot_order_buttons[0].x, y_offset + depot_order_buttons[0].y,
        depot_order_buttons[0].width, depot_order_buttons[0].height, data.focus_button_id == 1);
    image_draw(image_id, x_offset + depot_order_buttons[0].x + 2, y_offset + depot_order_buttons[0].y + 2, COLOR_MASK_NONE, SCALE_NONE);

    order_condition_type condition_type = b->data.depot.current_order.condition.condition_type;
    text_draw(translation_for(TR_BUILDING_INFO_DEPOT_CONDITION), x_offset, y_offset + depot_order_buttons[3].y + 6, FONT_SMALL_PLAIN, 0);
    button_border_draw(x_offset + depot_order_buttons[3].x, y_offset + depot_order_buttons[3].y,
        depot_order_buttons[3].width, depot_order_buttons[3].height, data.focus_button_id == 4);
    text_draw_centered(translation_for(TR_ORDER_CONDITION_NEVER + condition_type),
        x_offset + depot_order_buttons[3].x, y_offset + depot_order_buttons[3].y + 6, depot_order_buttons[3].width, FONT_SMALL_PLAIN, 0);
    if (condition_type != ORDER_CONDITION_ALWAYS && condition_type != ORDER_CONDITION_NEVER) {
        button_border_draw(x_offset + depot_order_buttons[4].x, y_offset + depot_order_buttons[4].y,
            depot_order_buttons[4].width, depot_order_buttons[4].height, data.focus_button_id == 5);
        text_draw_number_centered(b->data.depot.current_order.condition.threshold,
            x_offset + depot_order_buttons[4].x, y_offset + depot_order_buttons[4].y + 6, depot_order_buttons[4].width, FONT_SMALL_PLAIN);
    }

    text_draw(translation_for(TR_BUILDING_INFO_DEPOT_SOURCE), x_offset, y_offset + depot_order_buttons[1].y + 6, FONT_SMALL_PLAIN, 0);
    button_border_draw(x_offset + depot_order_buttons[1].x, y_offset + depot_order_buttons[1].y,
        depot_order_buttons[1].width, depot_order_buttons[1].height, data.focus_button_id == 2);
    text_draw_label_and_number_centered(lang_get_string(28, src->type),
        src->storage_id, "", x_offset + depot_order_buttons[1].x, y_offset + depot_order_buttons[1].y + 6,
        depot_order_buttons[1].width, FONT_SMALL_PLAIN, 0);

    text_draw(translation_for(TR_BUILDING_INFO_DEPOT_DESTINATION), x_offset, y_offset + depot_order_buttons[2].y + 6, FONT_SMALL_PLAIN, 0);
    button_border_draw(x_offset + depot_order_buttons[2].x, y_offset + depot_order_buttons[2].y,
        depot_order_buttons[2].width, depot_order_buttons[2].height, data.focus_button_id == 3);
    text_draw_label_and_number_centered(lang_get_string(28, dst->type),
        dst->storage_id, "", x_offset + depot_order_buttons[2].x, y_offset + depot_order_buttons[2].y + 6,
        depot_order_buttons[2].width, FONT_SMALL_PLAIN, 0);
}

int window_building_handle_mouse_depot(const mouse *m, building_info_context *c)
{
    return generic_buttons_handle_mouse(m, c->x_offset + DEPOT_BUTTONS_X_OFFSET, c->y_offset + DEPOT_BUTTONS_Y_OFFSET,
        depot_order_buttons, 5, &data.focus_button_id);
}

static void order_set_source(int index, int param2)
{
    window_building_info_depot_select_source();
}

static void order_set_destination(int index, int param2)
{
    window_building_info_depot_select_destination();
}

static void order_set_condition_type(int index, int param2)
{
    window_building_info_depot_toggle_condition_type();
}

static void order_set_condition_threshold(int index, int param2)
{
    window_building_info_depot_toggle_condition_threshold();
}

void window_building_draw_depot_order_source_destination_background(building_info_context *c, int is_select_destination)
{
    uint8_t *title = translation_for(TR_BUILDING_INFO_DEPOT_SELECT_SOURCE_TITLE);
    if (is_select_destination) {
        title = translation_for(TR_BUILDING_INFO_DEPOT_SELECT_DESTINATION_TITLE);
    }
    int y_offset = window_building_get_vertical_offset(c, 28);
    c->help_id = 0;
    outer_panel_draw(c->x_offset, y_offset, 29, 28);
    text_draw_centered(title, c->x_offset, y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK, 0);
    inner_panel_draw(c->x_offset + 16, y_offset + 42, c->width_blocks - 2, 21);
}

void window_building_draw_depot_select_source_destination(building_info_context *c) {
    int y_offset = window_building_get_vertical_offset(c, 28);

    scrollbar.x = c->x_offset + 16 * (c->width_blocks - 2) - 26;
    scrollbar.y = y_offset + 46;
    scrollbar_draw(&scrollbar);

    int index = 0, offset = scrollbar.scroll_position;
    setup_for_selected_depot(c, offset);

    for (index = 0; index < MAX_VISIBLE_ROWS; index++) {
        if (depot_select_storage_buttons[index].parameter2) {
            building *bld = building_get(depot_select_storage_buttons[index].parameter2);
            button_border_draw(c->x_offset + 18, y_offset + 46 + ROW_HEIGHT * index,
                16 * (c->width_blocks - 2) - 4 - (scrollbar.max_scroll_position > 0 ? 39 : 0),
                22, data.storage_building_focus_button_id == index + 1);
            text_draw_label_and_number_centered(lang_get_string(28, bld->type),
                bld->storage_id, "", c->x_offset + 32, y_offset + 52 + ROW_HEIGHT * index,
                16 * (c->width_blocks - 2) - 4 - (scrollbar.max_scroll_position > 0 ? 39 : 0),
                FONT_SMALL_PLAIN, COLOR_WHITE);
        }
    }
}

static void set_order_source(int depot_building_id, int building_id)
{
    building *b = building_get(depot_building_id);
    b->data.depot.current_order.src_storage_id = building_id;
    window_building_info_depot_return_to_main_window();
}

static void set_order_destination(int depot_building_id, int building_id)
{
    building *b = building_get(depot_building_id);
    b->data.depot.current_order.dst_storage_id = building_id;
    window_building_info_depot_return_to_main_window();
}

static int handle_mouse_depot_select_source_destination(const mouse *m, building_info_context *c, int is_source)
{
    if (scrollbar_handle_mouse(&scrollbar, m, 1)) {
        return 1;
    }

    int y_offset = window_building_get_vertical_offset(c, 28);
    int offset = scrollbar.scroll_position;
    setup_for_selected_depot(c, offset);
    for (int i = 0; i < MAX_VISIBLE_ROWS; i++) {
        depot_select_storage_buttons[i].x = 0;
        depot_select_storage_buttons[i].y = 22 * i;
        depot_select_storage_buttons[i].width = 16 * (c->width_blocks - 2) - 4 -
            (scrollbar.max_scroll_position > 0 ? 39 : 0);
        depot_select_storage_buttons[i].height = ROW_HEIGHT;
        depot_select_storage_buttons[i].left_click_handler = is_source ? set_order_source : set_order_destination;
    }

    return generic_buttons_handle_mouse(m, c->x_offset + 18, y_offset + 46, depot_select_storage_buttons,
        MAX_VISIBLE_ROWS, &data.storage_building_focus_button_id);
}

int window_building_handle_mouse_depot_select_source(const mouse *m, building_info_context *c)
{
    return handle_mouse_depot_select_source_destination(m, c, 1);
}

int window_building_handle_mouse_depot_select_destination(const mouse *m, building_info_context *c)
{
    return handle_mouse_depot_select_source_destination(m, c, 0);
}

static void order_set_resource(int index, int param2)
{
    window_building_info_depot_select_resource();
}

static void set_order_resource(int depot_building_id, int resource_id)
{
    building *b = building_get(depot_building_id);
    b->data.depot.current_order.resource_type = resource_id;
    window_building_info_depot_return_to_main_window();
}

void window_building_draw_depot_select_resource(building_info_context *c)
{
    int y_offset = window_building_get_vertical_offset(c, 28);
    c->help_id = 0;
    outer_panel_draw(c->x_offset, y_offset, 29, 28);
    text_draw_centered(translation_for(TR_BUILDING_INFO_DEPOT_SELECT_RESOURCE_TITLE),
        c->x_offset, y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK, 0);
    inner_panel_draw(c->x_offset + 16, y_offset + 42, c->width_blocks - 2, 21);
}

void window_building_draw_depot_select_resource_foreground(building_info_context *c)
{
    int y_offset = window_building_get_vertical_offset(c, 28);
    for (int i = RESOURCE_MIN; i < RESOURCE_MAX; i++) {
        int index = i - RESOURCE_MIN;

        int image_id = resource_get_data(i)->image.icon;
        const uint8_t *str = resource_get_data(i)->text;
        button_border_draw(c->x_offset + depot_select_resource_buttons[index].x,
            y_offset + 46 + depot_select_resource_buttons[index].y,
            214, 26, data.depot_resource_focus_button_id == index + 1);
        image_draw(image_id, c->x_offset + depot_select_resource_buttons[index].x + 3,
            y_offset + 46 + depot_select_resource_buttons[index].y + 3, COLOR_MASK_NONE, SCALE_NONE);
        text_draw(str, c->x_offset + depot_select_resource_buttons[index].x + 33,
            y_offset + 46 + depot_select_resource_buttons[index].y + 10, FONT_SMALL_PLAIN, COLOR_FONT_PLAIN);
    }
}

int window_building_handle_mouse_depot_select_resource(const mouse *m, building_info_context *c)
{
    setup_for_selected_depot(c, 0);
    for (size_t i = 0; i < sizeof(depot_select_resource_buttons) / sizeof(generic_button); i++) {
        depot_select_resource_buttons[i].parameter1 = data.depot_building_id;
    }
    int y_offset = window_building_get_vertical_offset(c, 28) + 46;
    return generic_buttons_handle_mouse(m, c->x_offset, y_offset, depot_select_resource_buttons,
        17, &data.depot_resource_focus_button_id);
}
