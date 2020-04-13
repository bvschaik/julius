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

void system_keyboard_show(void);

/**
 * Hide the virtual keyboard
 */
void system_keyboard_hide(void);

/**
 * Exit the game
 */
void system_exit(void);

#endif // GAME_SYSTEM_H
