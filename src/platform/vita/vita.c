#include "vita.h"

#include "core/file.h"
#include "game/system.h"
#include "graphics/screen.h"
#include "input/mouse.h"
#include "input/touch.h"
#include "input/keyboard.h"
#include "platform/vita/vita_keyboard.h"

#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include "SDL.h"

// max heap size is approx. 330 MB with -d ATTRIBUTE2=12, otherwise max is 192
int _newlib_heap_size_user = 330 * 1024 * 1024;

static void center_mouse_cursor(void)
{
    int x = screen_width() / 2;
    int y = screen_height() / 2;
    system_set_mouse_position(&x, &y);
    mouse_set_position(x, y);
}

static int vkbd_requested;

void platform_init_callback(void)
{
    touch_set_mode(TOUCH_MODE_TOUCHPAD);
    center_mouse_cursor();
}

static void vita_start_text_input(void)
{
    if (!keyboard_is_capturing()) {
        return;
    }
    const uint8_t *text = vita_keyboard_get(keyboard_get_text(), keyboard_get_max_text_length());
    keyboard_set_text(text);
}

void platform_per_frame_callback(void)
{
    if (vkbd_requested) {
        vita_start_text_input();
        vkbd_requested = 0;
    }
}

void platform_show_virtual_keyboard(void)
{
    vkbd_requested = 1;
}

void platform_hide_virtual_keyboard(void)
{}
