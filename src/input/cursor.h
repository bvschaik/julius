#ifndef INPUT_CURSOR_H
#define INPUT_CURSOR_H

#include "graphics/window.h"

typedef enum {
    CURSOR_ARROW = 0,
    CURSOR_SHOVEL = 1,
    CURSOR_SWORD = 2,
    CURSOR_MAX,
} cursor_shape;

typedef enum {
    CURSOR_SCALE_1 = 0,
    CURSOR_SCALE_1_5 = 1,
    CURSOR_SCALE_2 = 2,
} cursor_scale;

typedef struct {
    int hotspot_x;
    int hotspot_y;
    int width;
    int height;
    const char *data;
} cursor;

const cursor *input_cursor_data(cursor_shape cursor_id, cursor_scale scale);

void input_cursor_update(window_id window);

#endif // INPUT_CURSOR_H
