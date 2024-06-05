#include "list_box.h"

#include "core/direction.h"
#include "graphics/button.h"
#include "graphics/font.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/scroll.h"

void list_box_init(list_box_type *list_box, unsigned int total_items)
{
    list_box->selected_index = LIST_BOX_NO_SELECTION;
    list_box->total_items = total_items;
    list_box->focus_button_id = LIST_BOX_NO_SELECTION;
    scrollbar_init(&list_box->scrollbar, 0, list_box->total_items);
}

void list_box_update_total_items(list_box_type *list_box, unsigned int total_items)
{
    list_box->total_items = total_items;
    scrollbar_update_total_elements(&list_box->scrollbar, list_box->total_items);
    list_box_request_refresh(list_box);
}

unsigned int list_box_get_total_items(const list_box_type *list_box)
{
    return list_box->total_items;
}

void list_box_select_index(list_box_type *list_box, unsigned int index)
{
    if (index == list_box->selected_index) {
        return;
    }
    list_box_request_refresh(list_box);
    list_box->selected_index = index;
    if (list_box->on_select) {
        list_box->on_select(index, 0);
    }
}

unsigned int list_box_get_selected_index(const list_box_type *list_box)
{
    return list_box->selected_index;
}

void list_box_show_index(list_box_type *list_box, unsigned int index)
{
    if (index == LIST_BOX_NO_SELECTION) {
        return;
    }
    scrollbar_type *scrollbar = &list_box->scrollbar;
    if (index >= list_box->total_items) {
        scrollbar_reset(scrollbar, list_box->total_items - scrollbar->elements_in_view + 1);
    } else {
        scrollbar_reset(scrollbar, index);
    }
}

void list_box_show_selected_index(list_box_type *list_box)
{
    list_box_show_index(list_box, list_box->selected_index);
}

int list_box_get_scroll_position(const list_box_type *list_box)
{
    return list_box->scrollbar.scroll_position;
}

void list_box_request_refresh(list_box_type *list_box)
{
    list_box->refresh_requested = 1;
    if (!list_box->draw_inner_panel) {
        window_request_refresh();
    }
}

static int get_actual_width_blocks(const list_box_type *list_box)
{
    int width_blocks = list_box->width_blocks;
    if (!list_box->extend_to_hidden_scrollbar || list_box->total_items > list_box->scrollbar.elements_in_view) {
        width_blocks -= 2;
    }
    return width_blocks;
}

static void draw_scrollbar(list_box_type *list_box)
{
    scrollbar_type *scrollbar = &list_box->scrollbar;

    scrollbar->x = list_box->x + (list_box->width_blocks - 2) * BLOCK_SIZE + 4;
    scrollbar->y = list_box->y;
    scrollbar->on_scroll_callback = window_request_refresh;
    scrollbar->has_y_margin = 1;
    scrollbar->dot_padding = list_box->decorate_scrollbar ? 8 : 0;

    scrollbar->height = list_box->height_blocks * BLOCK_SIZE;
    int scrollable_height_pixels = scrollbar->height;
    scrollbar->scrollable_width = (list_box->width_blocks - 2) * BLOCK_SIZE;
    if (list_box->draw_inner_panel) {
        scrollable_height_pixels -= BLOCK_SIZE;
    }
    scrollbar->elements_in_view = scrollable_height_pixels / list_box->item_height;

    scrollbar_update_total_elements(scrollbar, list_box->total_items);

    if (list_box->decorate_scrollbar && list_box->total_items > scrollbar->elements_in_view) {
        inner_panel_draw(scrollbar->x + 4, scrollbar->y + 32, 2, scrollbar->height / BLOCK_SIZE - 4);
    }
    scrollbar_draw(&list_box->scrollbar);
}

void list_box_draw(list_box_type *list_box)
{
    draw_scrollbar(list_box);

    if(!list_box->refresh_requested) {
        return;
    }
    list_box->refresh_requested = 0;

    int width_blocks = get_actual_width_blocks(list_box);
    int padding = 0;

    if (list_box->draw_inner_panel) {
        padding = BLOCK_SIZE / 2;
        inner_panel_draw(list_box->x, list_box->y, width_blocks, list_box->height_blocks);
    }

    if (list_box->draw_item) {
        list_box_item item = {
            .y = list_box->y + padding,
            .width = width_blocks * BLOCK_SIZE - padding * 2,
            .height = list_box->item_height
        };

        unsigned int index = list_box->scrollbar.scroll_position;

        for (unsigned int i = 0; i < list_box->scrollbar.elements_in_view; i++, index++) {
            if (index >= list_box->total_items) {
                break;
            }
            item.x = list_box->x + padding;
            item.index = index;
            item.position = i;
            item.is_selected = index == list_box->selected_index;
            item.is_focused = list_box->focus_button_id == i;
            list_box->draw_item(&item);
            item.y += list_box->item_height;
        }
    }
}

static int handle_arrow_keys(list_box_type *list_box, int direction)
{
    int delta;
    switch (direction) {
        case DIR_0_TOP:
        case DIR_1_TOP_RIGHT:
        case DIR_7_TOP_LEFT:
            delta = -1;
            break;
        case DIR_4_BOTTOM:
        case DIR_3_BOTTOM_RIGHT:
        case DIR_5_BOTTOM_LEFT:
            delta = 1;
            break;
        default:
            return 0;
    }
    unsigned int max_index = list_box->total_items - 1;
    if (list_box->selected_index == LIST_BOX_NO_SELECTION) {
        if (delta == 1) {
            list_box->selected_index = 0;
        } else {
            list_box->selected_index = max_index;
        }
    } else {
        list_box->selected_index += delta;
        if (list_box->selected_index == LIST_BOX_NO_SELECTION) {
            list_box->selected_index = max_index;
        } else if (list_box->selected_index > max_index) {
            list_box->selected_index = 0;
        }
    }
    list_box_request_refresh(list_box);
    if (list_box->on_select) {
        list_box->on_select(list_box->selected_index, 0);
    }
    scrollbar_type *scrollbar = &list_box->scrollbar;
    if (list_box->selected_index > scrollbar->scroll_position + scrollbar->elements_in_view - 1) {
        scrollbar_reset(scrollbar, list_box->selected_index - scrollbar->elements_in_view + 1);
    } else if (list_box->selected_index < scrollbar->scroll_position) {
        scrollbar_reset(scrollbar, list_box->selected_index);
    }
    return 1;
}

static unsigned int get_button_id_from_position(const list_box_type *list_box, int x, int y)
{
    int padding = list_box->draw_inner_panel ? BLOCK_SIZE / 2 : 0;
    int width_blocks = get_actual_width_blocks(list_box);
    if (x < list_box->x + padding || x >= list_box->x + width_blocks * BLOCK_SIZE - padding || y < list_box->y) {
        return LIST_BOX_NO_SELECTION;
    }
    unsigned int button_id = (y - padding / 2 - list_box->y) / list_box->item_height;
    if (button_id >= list_box->scrollbar.elements_in_view ||
        button_id + list_box->scrollbar.scroll_position >= list_box->total_items) {
        return LIST_BOX_NO_SELECTION;
    }
    return button_id;
}

int list_box_handle_input(list_box_type *list_box, const mouse *m, int in_dialog)
{
    scrollbar_type *scrollbar = &list_box->scrollbar;

    if (scrollbar_handle_mouse(scrollbar, m, in_dialog) ||
        handle_arrow_keys(list_box, scroll_for_menu(m))) {
        list_box_request_refresh(list_box);
        return 1;
    }
    unsigned int old_focus_button_id = list_box->focus_button_id;
    list_box->focus_button_id = get_button_id_from_position(list_box, m->x, m->y);

    if (old_focus_button_id != list_box->focus_button_id) {
        list_box_request_refresh(list_box);
    }

    if (!m->left.went_up || list_box->focus_button_id == LIST_BOX_NO_SELECTION) {
        return 0;
    }

    unsigned int selected_index = list_box->focus_button_id + scrollbar->scroll_position;

    if (selected_index >= list_box->total_items) {
        return 0;
    }

    if (list_box->selected_index != selected_index) {
        list_box->selected_index = selected_index;
        list_box_request_refresh(list_box);
    }
    if (list_box->on_select) {
        list_box->on_select(list_box->selected_index, m->left.double_click);
    }

    return 1;
}

void list_box_handle_tooltip(const list_box_type *list_box, tooltip_context *c)
{
    if (list_box->focus_button_id == LIST_BOX_NO_SELECTION || !list_box->handle_tooltip) {
        return;
    }
    int padding = list_box->draw_inner_panel ? BLOCK_SIZE / 2 : 0;
    int item_width = get_actual_width_blocks(list_box) * BLOCK_SIZE - padding * 2;
    list_box_item item = {
        .width = item_width,
        .height = list_box->item_height,
        .index = list_box->focus_button_id + list_box->scrollbar.scroll_position,
        .position = list_box->focus_button_id,
        .x = list_box->x + padding,
        .y = list_box->y + padding + list_box->item_height,
        .is_selected = item.index == list_box->selected_index,
        .is_focused = 1
    };
    if (item.index < list_box->total_items) {
        list_box->handle_tooltip(&item, c);
    }
}
