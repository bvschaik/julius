#include "core/calc.h"
#include "core/encoding.h"
#include "input/keyboard.h"
#include "platform/vita/vita_keyboard.h"

static struct {
    uint8_t *text;
    int requested;
    int max_length;
} vkbd;

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
    if (vkbd.requested) {
        vita_start_text_input();
        vkbd.requested = 0;
    }
}

void platform_show_virtual_keyboard(void)
{
    vkbd.requested = 1;
}

void platform_hide_virtual_keyboard(void)
{}
