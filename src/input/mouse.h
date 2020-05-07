#ifndef GRAPHICS_MOUSE_H
#define GRAPHICS_MOUSE_H

/**
 * @file
 * Mouse state
 */

#include "input/touch.h"

/**
 * Mouse button state
 */
typedef struct {
    int is_down; /**< Mouse button is down */
    int went_down; /**< Mouse button went down during this cycle */
    int went_up; /**< Mouse button went up during this cycle */
    int double_click; /**< Mouse double clicked during this cycle */
    int system_change;
} mouse_button;


typedef enum {
    SCROLL_NONE = 0,
    SCROLL_UP = -1,
    SCROLL_DOWN = 1
} scroll_state;

/**
 * Mouse state
 */
typedef struct {
    int x; /**< Global position X */
    int y; /**< Global position Y */
    scroll_state scrolled; /**< Scroll state (up/down/none) */
    mouse_button left; /**< Left mouse button */
    mouse_button middle; /**< Middle mouse button */
    mouse_button right; /**< Right mouse button */
    int is_inside_window; /**< Whether the mouse is in the window */
    int is_touch; /**< Whether the mouse is a translated touch event */
} mouse;

/**
 * Gets the mouse state
 * @return Mouse state
 */
const mouse *mouse_get(void);

/**
 * Sets the mouse position
 * @param x X
 * @param y Y
 */
void mouse_set_position(int x, int y);

void mouse_set_left_down(int down);

void mouse_set_middle_down(int down);

void mouse_set_right_down(int down);

void mouse_set_scroll(scroll_state state);

void mouse_set_inside_window(int inside);

/**
 * Changes the mouse information from touch information
 * @param first The first touch
 */
void mouse_set_from_touch(const touch *first, const touch *last);

void mouse_reset_up_state(void);

void mouse_reset_scroll(void);

void mouse_reset_button_state(void);

void mouse_determine_button_state(void);

const mouse *mouse_in_dialog(const mouse *m);

#endif // GRAPHICS_MOUSE_H
