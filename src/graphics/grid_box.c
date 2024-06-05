#include "grid_box.h"

#include "graphics/button.h"
#include "graphics/font.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/scroll.h"

#define NO_POSITION ((unsigned int) -1)

static unsigned int calculate_scrollable_items(const grid_box_type *grid_box)
{
    if (grid_box->num_columns == 1) {
        return grid_box->total_items;
    }
    return (grid_box->total_items + grid_box->num_columns - 1) / grid_box->num_columns;
}

void grid_box_init(grid_box_type *grid_box, unsigned int total_items)
{
    grid_box->total_items = total_items;
    grid_box->focus.position = NO_POSITION;
    if (grid_box->num_columns == 0) {
        grid_box->num_columns = 1;
    }
    scrollbar_init(&grid_box->scrollbar, 0, calculate_scrollable_items(grid_box));
}

void grid_box_update_total_items(grid_box_type *grid_box, unsigned int total_items)
{
    grid_box->total_items = total_items;
    scrollbar_update_total_elements(&grid_box->scrollbar, calculate_scrollable_items(grid_box));
    grid_box_request_refresh(grid_box);
}

unsigned int grid_box_get_total_items(const grid_box_type *grid_box)
{
    return grid_box->total_items;
}

void grid_box_show_index(grid_box_type *grid_box, unsigned int index)
{
    if (index >= grid_box->total_items) {
        return;
    }
    scrollbar_type *scrollbar = &grid_box->scrollbar;
    if (index >= grid_box->total_items) {
        scrollbar_reset(scrollbar, calculate_scrollable_items(grid_box) - scrollbar->elements_in_view + 1);
    } else {
        scrollbar_reset(scrollbar, index / grid_box->num_columns);
    }
}

unsigned int grid_box_get_scroll_position(const grid_box_type *grid_box)
{
    return grid_box->scrollbar.scroll_position;
}

void grid_box_request_refresh(grid_box_type *grid_box)
{
    grid_box->refresh_requested = 1;
}

static unsigned int get_actual_width(const grid_box_type *grid_box)
{
    unsigned int width = grid_box->width;
    if (width <= 2 * BLOCK_SIZE) {
        return width;
    }
    if (!grid_box->extend_to_hidden_scrollbar ||
        calculate_scrollable_items(grid_box) > grid_box->scrollbar.elements_in_view) {
        width -= 2 * BLOCK_SIZE;
    }
    return width;
}

static void draw_scrollbar(grid_box_type *grid_box)
{
    scrollbar_type *scrollbar = &grid_box->scrollbar;

    scrollbar->x = grid_box->x + grid_box->width + 4 - 2 * BLOCK_SIZE;
    scrollbar->y = grid_box->y;
    scrollbar->on_scroll_callback = window_request_refresh;
    scrollbar->has_y_margin = 1;
    scrollbar->dot_padding = grid_box->decorate_scrollbar ? 8 : 0;

    scrollbar->height = grid_box->height;
    int scrollable_height_pixels = scrollbar->height;
    if (grid_box->draw_inner_panel) {
        scrollable_height_pixels -= BLOCK_SIZE;
    }
    scrollbar->scrollable_width = grid_box->width - 2 * BLOCK_SIZE;
    scrollbar->elements_in_view = scrollable_height_pixels / grid_box->item_height;

    scrollbar_update_total_elements(scrollbar, calculate_scrollable_items(grid_box));

    if (grid_box->decorate_scrollbar && grid_box->total_items > scrollbar->elements_in_view) {
        inner_panel_draw(scrollbar->x + 4, scrollbar->y + 32, 2, scrollbar->height / BLOCK_SIZE - 4);
    }
    scrollbar_draw(&grid_box->scrollbar);
}

void grid_box_draw(grid_box_type *grid_box)
{
    draw_scrollbar(grid_box);

    if(!grid_box->refresh_requested) {
        return;
    }
    grid_box->refresh_requested = 0;

    unsigned int width = get_actual_width(grid_box);
    unsigned int inner_padding = 0;

    if (grid_box->draw_inner_panel) {
        inner_padding = BLOCK_SIZE;
        inner_panel_draw(grid_box->x, grid_box->y, width / BLOCK_SIZE, grid_box->height / BLOCK_SIZE);
    }
    width = (width - inner_padding) / grid_box->num_columns;

    if (grid_box->draw_item) {
        grid_box_item item = {
            .y = grid_box->y + (inner_padding + grid_box->item_margin.vertical) / 2,
            .width = width - grid_box->item_margin.horizontal,
            .height = grid_box->item_height - grid_box->item_margin.vertical,
            .mouse.x = grid_box->focus.x,
            .mouse.y = grid_box->focus.y
        };

        unsigned int elements_in_view = grid_box->scrollbar.elements_in_view * grid_box->num_columns;
        unsigned int index = grid_box->scrollbar.scroll_position * grid_box->num_columns;

        for (unsigned int i = 0; i < elements_in_view; i++) {
            if (index >= grid_box->total_items) {
                break;
            }
            item.x = grid_box->x + (inner_padding + grid_box->item_margin.horizontal) / 2 +
                width * (i % grid_box->num_columns);
            item.index = index;
            item.position = i;
            item.is_focused = grid_box->focus.position == i;
            grid_box->draw_item(&item);
            if (i % grid_box->num_columns == grid_box->num_columns - 1) {
                item.y += grid_box->item_height;
            }
            index++;
        }
    }
}

static int determine_focus(grid_box_type *grid_box, int x, int y)
{
    unsigned int inner_padding = grid_box->draw_inner_panel ? BLOCK_SIZE / 2 : 0;
    unsigned int width = get_actual_width(grid_box);
    unsigned int old_position = grid_box->focus.position;
    if (x < grid_box->x + inner_padding || x >= grid_box->x + width - inner_padding ||
        y < grid_box->y + inner_padding || y >= grid_box->y + grid_box->height - inner_padding) {
        grid_box->focus.position = NO_POSITION;
        return old_position != NO_POSITION;
    }
    unsigned int mouse_x = x - grid_box->x - inner_padding;
    unsigned int mouse_y = y - grid_box->y;

    unsigned int button_id = mouse_y / grid_box->item_height * grid_box->num_columns;
    unsigned int item_width = (width - inner_padding * 2) / grid_box->num_columns;
    button_id += mouse_x / item_width;

    if (button_id + grid_box->scrollbar.scroll_position * grid_box->num_columns >= grid_box->total_items) {
        grid_box->focus.position = NO_POSITION;
        return old_position != NO_POSITION;
    }

    mouse_x %= item_width;
    mouse_y %= grid_box->item_height;

    if (mouse_x < grid_box->item_margin.horizontal / 2 ||
        mouse_x >= item_width - grid_box->item_margin.horizontal + grid_box->item_margin.horizontal / 2 ||
        mouse_y < grid_box->item_margin.vertical / 2 ||
        mouse_y >= grid_box->item_height - grid_box->item_margin.vertical + grid_box->item_margin.vertical / 2) {
        grid_box->focus.position = NO_POSITION;
        return old_position != NO_POSITION;
    }

    unsigned int old_x = grid_box->focus.x;
    unsigned int old_y = grid_box->focus.y;

    grid_box->focus.position = button_id;
    grid_box->focus.x = mouse_x - grid_box->item_margin.horizontal / 2;
    grid_box->focus.y = mouse_y - grid_box->item_margin.vertical / 2;

    return old_position != grid_box->focus.position || old_x != grid_box->focus.x || old_y != grid_box->focus.y;
}

int grid_box_handle_input(grid_box_type *grid_box, const mouse *m, int in_dialog)
{
    scrollbar_type *scrollbar = &grid_box->scrollbar;

    if (scrollbar_handle_mouse(scrollbar, m, in_dialog)) {
        grid_box_request_refresh(grid_box);
        return 1;
    }

    if (determine_focus(grid_box, m->x, m->y)) {
        grid_box_request_refresh(grid_box);
    }

    if (!m->left.went_up || grid_box->focus.position == NO_POSITION) {
        return 0;
    }

    unsigned int clicked_index = grid_box->focus.position + scrollbar->scroll_position * grid_box->num_columns;

    if (grid_box->on_click) {
        grid_box->on_click(clicked_index, grid_box->focus.x, grid_box->focus.y);
    }

    return 1;
}

void grid_box_handle_tooltip(const grid_box_type *grid_box, tooltip_context *c)
{
    if (grid_box->focus.position == NO_POSITION || !grid_box->handle_tooltip) {
        return;
    }
    unsigned int inner_padding = grid_box->draw_inner_panel ? BLOCK_SIZE : 0;
    unsigned int width = (get_actual_width(grid_box) - inner_padding) / grid_box->num_columns;
    grid_box_item item = {
        .width = width - grid_box->item_margin.horizontal,
        .height = grid_box->item_height,
        .index = grid_box->focus.position + grid_box->scrollbar.scroll_position * grid_box->num_columns,
        .position = grid_box->focus.position,
        .x = grid_box->x + (inner_padding + grid_box->item_margin.horizontal) / 2 +
            width * (grid_box->focus.position % grid_box->num_columns),
        .y = grid_box->y + (inner_padding + grid_box->item_margin.vertical) / 2 +
            grid_box->item_height * (grid_box->focus.position / grid_box->num_columns),
        .is_focused = 1,
        .mouse.x = grid_box->focus.x,
        .mouse.y = grid_box->focus.y
    };
    if (item.index < grid_box->total_items) {
        grid_box->handle_tooltip(&item, c);
    }
}
