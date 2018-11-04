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
 * Set fullscreen on/off
 */
void system_set_fullscreen(int fullscreen);

/**
 * Initialize cursors
 */
void system_init_cursors();

/**
 * Set cursor to the specified cursor in @link input/cursor.h @endlink
 * @param cursor_id Cursor to set
 */
void system_set_cursor(int cursor_id);

/**
 * Exit the game
 */
void system_exit();

#endif // GAME_SYSTEM_H
