#ifndef INPUT_CURSOR_H
#define INPUT_CURSOR_H

#include "graphics/window.h"

typedef enum {
    CURSOR_ARROW = 0,
    CURSOR_SHOVEL = 1,
    CURSOR_SWORD = 2,
    CURSOR_MAX,
} cursor_shape;

typedef struct {
    int hotspot_x;
    int hotspot_y;
    char data[32*32+1];
} cursor;

const cursor *input_cursor_data(cursor_shape cursor_id);

void input_cursor_update(window_id window);

#endif // INPUT_CURSOR_H
