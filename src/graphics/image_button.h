#ifndef GRAPHICS_IMAGE_BUTTON_H
#define GRAPHICS_IMAGE_BUTTON_H

#include "graphics/button.h"
#include "input/mouse.h"

#include "core/time.h"

enum {
    IB_NORMAL = 4,
    IB_SCROLL = 6,
    IB_BUILD = 2
};

typedef struct {
    short x_offset;
    short y_offset;
    short width;
    short height;
    short button_type;
    short image_collection;
    short image_offset;
    void (*left_click_handler)(int param1, int param2);
    void (*right_click_handler)(int param1, int param2);
    int parameter1;
    int parameter2;
    char enabled;
    // state
    char pressed;
    char focused;
    time_millis pressed_since;
} image_button;

void image_buttons_draw(int x, int y, image_button *buttons, int num_buttons);

int image_buttons_handle_mouse(const mouse *m, int x, int y, image_button *buttons, int num_buttons, int *focus_button_id);

#endif // GRAPHICS_IMAGE_BUTTON_H
