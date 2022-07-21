#ifndef GRAPHICS_SCROLLBAR_H
#define GRAPHICS_SCROLLBAR_H

#include "graphics/color.h"
#include "graphics/font.h"
#include "input/mouse.h"

typedef struct {
    int x;
    int y;
    int height;
    int scrollable_width;
    int elements_in_view;
    void (*on_scroll_callback)(void);
    int has_y_margin;
    int dot_padding;
    int always_visible;
    int max_scroll_position;
    int scroll_position;
    int is_dragging_scrollbar_dot;
    int scrollbar_dot_drag_offset;
    int touch_drag_state;
    int position_on_touch;
} scrollbar_type;

/**
 * Initializes the scrollbar
 * @param scrollbar Scrollbar
 * @param scroll_position Scroll position to set
 * @param total_elements The number of elements to scroll
 */
void scrollbar_init(scrollbar_type *scrollbar, int scroll_position, int total_elements);

/**
 * Resets the text to the specified scroll position and forces recalculation of lines
 * @param scrollbar Scrollbar
 * @param scroll_position Scroll position to set
 */
void scrollbar_reset(scrollbar_type *scrollbar, int scroll_position);

/**
 * Update the number of total elements, adjusting the scroll position if necessary
 * @param scrollbar Scrollbar
 * @param total_items New number of total elements
 */
void scrollbar_update_total_elements(scrollbar_type *scrollbar, int total_elements);

/**
 * Draws the scrollbar
 * @param scrollbar Scrollbar
 */
void scrollbar_draw(scrollbar_type *scrollbar);

/**
 * Handles mouse interaction with the scrollbar and scroll wheel
 * @param scrollbar Scrollbar
 * @param m Mouse state
 * @param in_dialog Whether we are inside a centered dialog box
 * @return True if any interaction was handled
 */
int scrollbar_handle_mouse(scrollbar_type *scrollbar, const mouse *m, int in_dialog);

#endif // GRAPHICS_SCROLLBAR_H
