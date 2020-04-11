#ifndef GAME_SYSTEM_H
#define GAME_SYSTEM_H

/**
 * @file
 * Functions that should implemented by the underlying system
 */

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
 * Show the virtual keyboard
 */
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
