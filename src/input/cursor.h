#ifndef INPUT_CURSOR_H
#define INPUT_CURSOR_H

#include "graphics/window.h"

#define CURSOR_TYPE_MAX 3

typedef enum {
    CURSOR_DISABLED = -1,
    CURSOR_ARROW = 0,
    CURSOR_SHOVEL = 1,
    CURSOR_SWORD = 2,
    CURSOR_MAX = 3,
} cursor_shape;

typedef enum {
    CURSOR_SCALE_1 = 0,
    CURSOR_SCALE_1_5 = 1,
    CURSOR_SCALE_2 = 2,
    CURSOR_SCALE_MAX = 3
} cursor_scale;

typedef enum {
    CURSOR_TYPE_PIXMAP = 0,
    CURSOR_TYPE_PNG = 1,
} cursor_type;

typedef struct {
    cursor_type type;
    int width;
    int height;
    int offset_x;
    int offset_y;
    int hotspot_x;
    int hotspot_y;
    int rotated;
    const char *data;
} cursor;

const cursor *input_cursor_data(cursor_shape cursor_id, cursor_scale scale);

void input_cursor_update(window_id window);

#endif // INPUT_CURSOR_H
