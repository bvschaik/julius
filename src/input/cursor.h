#ifndef INPUT_CURSOR_H
#define INPUT_CURSOR_H

#include "UI/Window.h"

enum {
    CURSOR_ARROW = 0,
    CURSOR_SHOVEL = 1,
    CURSOR_SWORD = 2,
};

typedef struct {
    int hotspotX;
    int hotspotY;
    char data[32*32+1];
} cursor;

const cursor *input_cursor_data(int cursor_id);

void input_cursor_update(WindowId window);

#endif // INPUT_CURSOR_H
