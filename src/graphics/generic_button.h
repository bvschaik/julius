#ifndef GRAPHICS_CUSTOM_BUTTON_H
#define GRAPHICS_CUSTOM_BUTTON_H

#include "graphics/button.h"
#include "graphics/font.h"
#include "input/mouse.h"

typedef struct {
    short x;
    short y;
    short width;
    short height;
    void (*left_click_handler)(int param1, int param2);
    void (*right_click_handler)(int param1, int param2);
    int parameter1;
    int parameter2;
} generic_button;

int generic_buttons_handle_mouse(const mouse *m, int x, int y, generic_button *buttons, int num_buttons, int *focus_button_id);
int generic_buttons_min_handle_mouse(const mouse* m, int x, int y, generic_button* buttons, int num_buttons, int* focus_button_id, int minimum_button);

#endif // GRAPHICS_CUSTOM_BUTTON_H
