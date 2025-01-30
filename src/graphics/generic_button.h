#ifndef GRAPHICS_GENERIC_BUTTON_H
#define GRAPHICS_GENERIC_BUTTON_H

#include "input/mouse.h"

typedef struct generic_button {
    short x;
    short y;
    short width;
    short height;
    void (*left_click_handler)(const struct generic_button *button);
    void (*right_click_handler)(const struct generic_button *button);
    int parameter1;
    int parameter2;
} generic_button;

int generic_buttons_handle_mouse(const mouse *m, int x, int y, generic_button *buttons, unsigned int num_buttons,
    unsigned int *focus_button_id);

#endif // GRAPHICS_GENERIC_BUTTON_H
