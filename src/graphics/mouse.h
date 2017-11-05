#ifndef GRAPHICS_MOUSE_H
#define GRAPHICS_MOUSE_H

/**
 * @file
 * Mouse state
 */

/**
 * Mouse button state
 */
typedef struct
{
    int is_down; /**< Mouse button is down */
    int went_down; /**< Mouse button went down during this cycle */
    int went_up; /**< Mouse button went up during this cycle */
    int new_is_down;
} mouse_button;


typedef enum
{
    SCROLL_NONE = 0,
    SCROLL_UP = -1,
    SCROLL_DOWN = 1
} scroll_state;
/**
 * Mouse state
 */
typedef struct
{
    int x; /**< Global position X */
    int y; /**< Global position Y */
    scroll_state scrolled; /**< Scroll state (up/down/none) */
    mouse_button left; /**< Left mouse button */
    mouse_button right; /**< Right mouse button */
    int is_inside_window; /**< Whether the mouse is in the window */
} mouse;

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Gets the mouse state
 * @return Mouse state
 */
const mouse *mouse_get();

/**
 * Sets the mouse position
 * @param x X
 * @param y Y
 */
void mouse_set_position(int x, int y);

void mouse_set_left_down(int down);

void mouse_set_right_down(int down);

void mouse_set_scroll(scroll_state state);

void mouse_set_inside_window(int inside);

void mouse_reset_up_state();

void mouse_determine_button_state();


#ifdef __cplusplus
}
#endif

#endif // GRAPHICS_MOUSE_H
