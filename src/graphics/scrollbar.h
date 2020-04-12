#ifndef GRAPHICS_SCROLLBAR_H
#define GRAPHICS_SCROLLBAR_H

#include "graphics/color.h"
#include "graphics/font.h"
#include "input/mouse.h"

typedef struct {
    int x;
    int y;
    int height;
    void (*on_scroll_callback)(void);
    int max_scroll_position;
    int scroll_position;
    int is_dragging_scroll;
    int scroll_position_drag;
} scrollbar_type;

/**
 * Initializes the scrollbar
 * @param scrollbar Scrollbar
 * @param max_scroll_position Max position
 */
void scrollbar_init(scrollbar_type *scrollbar, int max_scroll_position);

/**
 * Resets the text to the specified scroll position and forces recalculation of lines
 */
void scrollbar_reset(scrollbar_type *scrollbar, int scroll_position);

/**
 * Draws the scrollbar
 */
void scrollbar_draw(scrollbar_type *scrollbar);

/**
 * Handles mouse interaction with the scrollbar and scroll wheel
 * @param m Mouse state
 * @return True if any interaction was handled
 */
int scrollbar_handle_mouse(scrollbar_type *scrollbar, const mouse *m);

#endif // GRAPHICS_SCROLLBAR_H
