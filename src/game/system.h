#ifndef GAME_SYSTEM_H
#define GAME_SYSTEM_H

#include "graphics/color.h"
#include "input/keys.h"

/**
 * @file
 * Functions that should implemented by the underlying system
 */

/**
 * Gets the version of Julius
 * @return Version number
 */
const char *system_version(void);

/**
 * Resize window
 * @param width New width
 * @param height New height
 */
void system_resize(int width, int height);

/**
 * Center window
 */
void system_center(void);

/**
 * Returns whether the window must always be fullscreen
 * @return true when only fullscreen can be used, false otherwise
 */
int system_is_fullscreen_only(void);

/**
 * Set fullscreen on/off
 */
void system_set_fullscreen(int fullscreen);

/**
 * Set display scale to the given percentage
 * @param scale_percentage Scale percentage to set, 100% is normal
 * @return The actual scale percentage that was set, which may be different to respect minimum resolution
 */
int system_scale_display(int scale_percentage);

/**
 * Checks whether the display can be scaled. If true, min_scale and max_scale indicate by how much.
 * @param min_scale Minimum scale that can be applied
 * @param max_scale Maximum scale that can be applied
 * @return True if the display can be scaled
 */
int system_can_scale_display(int *min_scale, int *max_scale);

/**
 * (Re-)Initialize cursors with the specified scale percentage
 * @param scale_percentage Scaling percentage to use
 */
void system_init_cursors(int scale_percentage);

/**
 * Set cursor to the specified cursor in @link input/cursor.h @endlink
 * @param cursor_id Cursor to set
 */
void system_set_cursor(int cursor_id);

/**
 * Get the key corresponding to the symbol in the current layout
 * @param name Name of the key
 * @return Corresponding key, or KEY_TYPE_NONE if the key does not exist on the layout
 */
key_type system_keyboard_key_for_symbol(const char *name);

/**
 * Gets the key name for the current keyboard layout
 * @param key Key to get the name for
 * @return Key name, may be empty
 */
const char *system_keyboard_key_name(key_type key);

/**
 * Gets the key modifier name
 * @param modifier Modifier
 * @return Modifier name, may depend on OS
 */
const char *system_keyboard_key_modifier_name(key_modifier_type modifier);

/**
 * Sets the position/size of the keyboard input box
 * @param x X offset
 * @param y Y offset
 * @param width Width of the box
 * @param height Height of the box
 */
void system_keyboard_set_input_rect(int x, int y, int width, int height);

/**
 * Show the virtual keyboard
 */
void system_keyboard_show(void);

/**
 * Hide the virtual keyboard
 */
void system_keyboard_hide(void);

/**
 * Enable text input events
 */
void system_start_text_input(void);

/**
 * Disable text input events
 */
void system_stop_text_input(void);

/**
 * Sets mouse to relative mode, where moving the mouse
 * does not move the cursor on the screen
 * @param enabled Boolean: 1 for enable, 0 for disable
 */
void system_mouse_set_relative_mode(int enabled);

/**
 * Gets relative mouse movement when in relative mode
 * @param x X position of the mouse
 * @param y Y position of the mouse
 */
void system_mouse_get_relative_state(int *x, int *y);

/**
 * Moves the mouse cursor
 * @param delta_x The amount to move on the X axis
 * @param delta_y The amount to move on the Y axis
 */
void system_move_mouse_cursor(int delta_x, int delta_y);

/**
 * Sets the mouse position
 * @param x Pointer to X position of the mouse
 * @param y Pointer to Y position of the mouse
 * Note: the function limits x and y to the bounds of the game window
 * if the x and y are less than 0 or greater than the width/height, they will be
 * changed to fit in the window
 */
void system_set_mouse_position(int *x, int *y);

/**
 * Creates a new framebuffer
 * @return The framebuffer
 */
color_t *system_create_framebuffer(int width, int height);

/**
 * Exit the game
 */
void system_exit(void);

#endif // GAME_SYSTEM_H
