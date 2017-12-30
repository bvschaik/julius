#ifndef INPUT_CURSOR_H
#define INPUT_CURSOR_H

#include "UI/Window.h"

typedef enum {
    CURSOR_ARROW = 0,
    CURSOR_SHOVEL = 1,
    CURSOR_SWORD = 2,
    CURSOR_MAX,
} cursor_shape;

typedef struct {
    int hotspotX;
    int hotspotY;
    char data[32*32+1];
} cursor;

const cursor *input_cursor_data(cursor_shape cursor_id);

void input_cursor_update(WindowId window);

#endif // INPUT_CURSOR_H
