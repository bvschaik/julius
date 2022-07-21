#include "scrollbar.h"

#include "core/calc.h"
#include "core/image.h"
#include "core/image_group.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/screen.h"

enum {
    TOUCH_DRAG_NONE = 0,
    TOUCH_DRAG_PENDING = 1,
    TOUCH_DRAG_IN_PROGRESS = 2
};

#define SCROLL_BUTTON_HEIGHT 26
#define SCROLL_BUTTON_WIDTH 39
#define SCROLL_DOT_SIZE 25
#define TOTAL_BUTTON_HEIGHT (2 * SCROLL_BUTTON_HEIGHT + SCROLL_DOT_SIZE)

static void text_scroll(int is_down, int num_lines);

static image_button image_button_scroll_up = {
    0, 0, SCROLL_BUTTON_WIDTH, SCROLL_BUTTON_HEIGHT, IB_SCROLL,
    GROUP_OK_CANCEL_SCROLL_BUTTONS, 8, text_scroll, button_none, 0, 1, 1
};
static image_button image_button_scroll_down = {
    0, 0, SCROLL_BUTTON_WIDTH, SCROLL_BUTTON_HEIGHT, IB_SCROLL,
    GROUP_OK_CANCEL_SCROLL_BUTTONS, 12, text_scroll, button_none, 1, 1, 1
};

static scrollbar_type *current;

void scrollbar_init(scrollbar_type *scrollbar, int scroll_position, int total_elements)
{
    int max_scroll_position = total_elements - scrollbar->elements_in_view;
    if (max_scroll_position < 0) {
        max_scroll_position = 0;
    }
    scrollbar->scroll_position = calc_bound(scroll_position, 0, max_scroll_position);
    scrollbar->max_scroll_position = max_scroll_position;
    scrollbar->is_dragging_scrollbar_dot = 0;
    scrollbar->touch_drag_state = TOUCH_DRAG_NONE;
}

void scrollbar_reset(scrollbar_type *scrollbar, int scroll_position)
{
    scrollbar->scroll_position = scroll_position;
    scrollbar->is_dragging_scrollbar_dot = 0;
    scrollbar->touch_drag_state = TOUCH_DRAG_NONE;
}

void scrollbar_update_total_elements(scrollbar_type *scrollbar, int total_elements)
{
    int max_scroll_position = total_elements - scrollbar->elements_in_view;
    if (max_scroll_position < 0) {
        max_scroll_position = 0;
    }
    scrollbar->max_scroll_position = max_scroll_position;
    if (scrollbar->scroll_position > max_scroll_position) {
        scrollbar->scroll_position = max_scroll_position;
    }
}

void scrollbar_draw(scrollbar_type *scrollbar)
{
    if (scrollbar->max_scroll_position > 0 || scrollbar->always_visible) {
        image_buttons_draw(scrollbar->x, scrollbar->y, &image_button_scroll_up, 1);
        image_buttons_draw(scrollbar->x, scrollbar->y + scrollbar->height - SCROLL_BUTTON_HEIGHT,
            &image_button_scroll_down, 1);

        int pct;
        if (scrollbar->scroll_position <= 0) {
            pct = 0;
        } else if (scrollbar->scroll_position >= scrollbar->max_scroll_position) {
            pct = 100;
        } else {
            pct = calc_percentage(scrollbar->scroll_position, scrollbar->max_scroll_position);
        }
        int offset = calc_adjust_with_percentage(
            scrollbar->height - TOTAL_BUTTON_HEIGHT - 2 * scrollbar->dot_padding, pct);
        if (scrollbar->is_dragging_scrollbar_dot) {
            offset = scrollbar->scrollbar_dot_drag_offset;
        }
        image_draw(image_group(GROUP_PANEL_BUTTON) + 39,
            scrollbar->x + (SCROLL_BUTTON_WIDTH - SCROLL_DOT_SIZE) / 2,
            scrollbar->y + offset + SCROLL_BUTTON_HEIGHT + scrollbar->dot_padding,
            COLOR_MASK_NONE, SCALE_NONE);
    }
}

static int touch_inside_scrollable_area(const scrollbar_type *scrollbar, const touch *t, int in_dialog)
{
    int x = t->start_point.x;
    int y = t->start_point.y;
    if (in_dialog) {
        x -= screen_dialog_offset_x();
        y -= screen_dialog_offset_y();
    }
    return scrollbar->max_scroll_position > 0 &&
        x >= scrollbar->x - scrollbar->scrollable_width && x <= scrollbar->x - 2 &&
        y >= scrollbar->y && y < scrollbar->y + scrollbar->height;
}

static int handle_touch(scrollbar_type *scrollbar, const touch *t, int in_dialog)
{
    int old_position = scrollbar->scroll_position;
    int active = scrollbar->touch_drag_state == TOUCH_DRAG_IN_PROGRESS;

    if (t->has_started && touch_inside_scrollable_area(scrollbar, t, in_dialog)) {
        scrollbar->touch_drag_state = TOUCH_DRAG_PENDING;
        scrollbar->position_on_touch = scrollbar->scroll_position;
    }
    if (t->has_moved && scrollbar->touch_drag_state != TOUCH_DRAG_NONE) {
        scrollbar->touch_drag_state = TOUCH_DRAG_IN_PROGRESS;
        int element_height = (scrollbar->height - 8 * scrollbar->has_y_margin) / scrollbar->elements_in_view;
        int current_y = t->current_point.y - ((t->current_point.y - (scrollbar->y + 8 * scrollbar->has_y_margin)) % element_height);
        int start_y = t->start_point.y - ((t->start_point.y - (scrollbar->y + 8 * scrollbar->has_y_margin)) % element_height);
        int touch_scrolled = (current_y - start_y) / element_height;
        scrollbar->scroll_position = calc_bound(scrollbar->position_on_touch - touch_scrolled, 0, scrollbar->max_scroll_position);
        active = 1;
    }
    if (t->has_ended) {
        scrollbar->touch_drag_state = TOUCH_DRAG_NONE;
    }
    if (scrollbar->on_scroll_callback && old_position != scrollbar->scroll_position) {
        scrollbar->on_scroll_callback();
    }
    return active;
}

static int handle_scrollbar_dot(scrollbar_type *scrollbar, const mouse *m)
{
    if (scrollbar->max_scroll_position <= 0 || !m->left.is_down) {
        return 0;
    }
    int track_height = scrollbar->height - TOTAL_BUTTON_HEIGHT - 2 * scrollbar->dot_padding;
    if (m->x < scrollbar->x || m->x >= scrollbar->x + SCROLL_BUTTON_WIDTH) {
        return 0;
    }
    if (m->y < scrollbar->y + SCROLL_BUTTON_HEIGHT + scrollbar->dot_padding ||
        m->y > scrollbar->y + scrollbar->height - SCROLL_BUTTON_HEIGHT - scrollbar->dot_padding) {
        return 0;
    }
    int dot_offset = m->y - scrollbar->y - SCROLL_DOT_SIZE / 2 - SCROLL_BUTTON_HEIGHT;
    if (dot_offset < 0) {
        dot_offset = 0;
    }
    if (dot_offset > track_height) {
        dot_offset = track_height;
    }
    int pct_scrolled = calc_percentage(dot_offset, track_height);
    scrollbar->scroll_position = calc_adjust_with_percentage(
        scrollbar->max_scroll_position, pct_scrolled);
    scrollbar->is_dragging_scrollbar_dot = 1;
    scrollbar->scrollbar_dot_drag_offset = dot_offset;
    if (scrollbar->scrollbar_dot_drag_offset < 0) {
        scrollbar->scrollbar_dot_drag_offset = 0;
    }
    if (scrollbar->on_scroll_callback) {
        scrollbar->on_scroll_callback();
    }
    return 1;
}

int scrollbar_handle_mouse(scrollbar_type *scrollbar, const mouse *m, int in_dialog)
{
    if (scrollbar->max_scroll_position <= 0) {
        return 0;
    }
    current = scrollbar;
    if (!m->is_touch) {
        scrollbar->touch_drag_state = TOUCH_DRAG_NONE;
    }
    if (scrollbar->touch_drag_state != TOUCH_DRAG_IN_PROGRESS) {
        if (m->scrolled == SCROLL_DOWN) {
            text_scroll(1, 3);
        } else if (m->scrolled == SCROLL_UP) {
            text_scroll(0, 3);
        }

        if (image_buttons_handle_mouse(m,
            scrollbar->x, scrollbar->y, &image_button_scroll_up, 1, 0)) {
            return 1;
        }
        if (image_buttons_handle_mouse(m,
            scrollbar->x, scrollbar->y + scrollbar->height - SCROLL_BUTTON_HEIGHT,
            &image_button_scroll_down, 1, 0)) {
            return 1;
        }
    }
    if (m->is_touch && handle_touch(scrollbar, touch_get_earliest(), in_dialog)) {
        return 1;
    }
    return handle_scrollbar_dot(scrollbar, m);
}

static void text_scroll(int is_down, int num_lines)
{
    scrollbar_type *scrollbar = current;
    if (is_down) {
        scrollbar->scroll_position += num_lines;
        if (scrollbar->scroll_position > scrollbar->max_scroll_position) {
            scrollbar->scroll_position = scrollbar->max_scroll_position;
        }
    } else {
        scrollbar->scroll_position -= num_lines;
        if (scrollbar->scroll_position < 0) {
            scrollbar->scroll_position = 0;
        }
    }
    scrollbar->is_dragging_scrollbar_dot = 0;
    if (scrollbar->on_scroll_callback) {
        scrollbar->on_scroll_callback();
    }
}
