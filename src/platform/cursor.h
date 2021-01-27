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
 * @return The cursor size as a power of two
 */
int platform_cursor_get_texture_size(int width, int height);

#endif // PLATFORM_CURSOR_H
