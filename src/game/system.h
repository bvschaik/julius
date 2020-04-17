#ifndef GAME_SYSTEM_H
#define GAME_SYSTEM_H

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
 * Set cursor to the specified cursor in @link input/cursor.h @endlink
 * @param cursor_id Cursor to set
 */
void system_set_cursor(int cursor_id);

/**
 * Get the key corresponding to the symbol in the current layout
 * @param name Name of the key
 * @return Corresponding key, or KEY_NONE if the key does not exist on the layout
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

void system_keyboard_show(void);

/**
 * Hide the virtual keyboard
 */
void system_keyboard_hide(void);

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
 * Exit the game
 */
void system_exit(void);

#endif // GAME_SYSTEM_H
