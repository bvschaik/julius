#ifndef GRAPHICS_CUSTOM_BUTTON_H
#define GRAPHICS_CUSTOM_BUTTON_H

#include "graphics/button.h"
#include "input/mouse.h"

enum {
    GB_IMMEDIATE = 1,
    GB_ON_MOUSE_UP = 3,
};

typedef struct {
    short x_start;
    short y_start;
    short x_end;
    short y_end;
    int button_type;
    void (*left_click_handler)(int param1, int param2);
    void (*right_click_handler)(int param1, int param2);
    int parameter1;
    int parameter2;
} generic_button;

int generic_buttons_handle_mouse(const mouse *m, int x, int y, generic_button *buttons, int num_buttons, int *focus_button_id);

#endif // GRAPHICS_CUSTOM_BUTTON_H
