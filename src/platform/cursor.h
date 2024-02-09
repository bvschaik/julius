#ifndef PLATFORM_CURSOR_H
#define PLATFORM_CURSOR_H

#include "input/cursor.h"

/**
 * Gets the current cursor shape
 * @return The cursor shape
 */
cursor_shape platform_cursor_get_current_shape(void);

/**
 * Gets the current cursor scale
 * @return The cursor scale
 */
cursor_scale platform_cursor_get_current_scale(void);

/**
 * Gets the current cursor texture size as a power of two
 * @param c The cursor info struct
 * @return The cursor size as a power of two
 */
int platform_cursor_get_texture_size(const cursor *c);

/**
 * Force software cursor mode
 */
void platform_cursor_force_software_mode(void);

/**
 * Indicates whether we're using the software cursor
 * @return Whether we're using the software cursor
 */
int platform_cursor_is_software(void);

#endif // PLATFORM_CURSOR_H
