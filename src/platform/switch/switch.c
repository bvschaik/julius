#include "core/calc.h"
#include "core/config.h"
#include "core/encoding.h"
#include "core/string.h"
#include "game/system.h"
#include "graphics/screen.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include "input/touch.h"
#include "platform/switch/switch.h"

#include <switch.h>

#define MAX_VKBD_TEXT_SIZE 600

static AppletOperationMode display_mode = -1;

#define HANDHELD_SCREEN_SCALE 200
#define DOCKED_SCREEN_SCALE 150
#define CURSOR_SCALE 200

static struct {
    uint8_t text[MAX_VKBD_TEXT_SIZE];
    char utf8_text[MAX_VKBD_TEXT_SIZE];
    int requested;
} vkbd;

static void center_mouse_cursor(void)
{
    int x = screen_width() / 2;
    int y = screen_height() / 2;
    system_set_mouse_position(&x, &y);
    mouse_set_position(x, y);
}

static void change_display_size(void)
{
    AppletOperationMode mode = appletGetOperationMode();
    if (mode != display_mode) {
        SDL_Log("Changing display mode to %s", mode == AppletOperationMode_Handheld ? "handheld" : "docked");
        display_mode = mode;
        if (display_mode == AppletOperationMode_Handheld) {
            system_scale_display(HANDHELD_SCREEN_SCALE);
        } else {
            system_scale_display(DOCKED_SCREEN_SCALE);
        }
        center_mouse_cursor();
    }
}

static const uint8_t *switch_keyboard_get(const uint8_t *text, int max_length)
{
    max_length = calc_bound(max_length, 0, MAX_VKBD_TEXT_SIZE);
    string_copy(text, vkbd.text, max_length);
    encoding_to_utf8(text, vkbd.utf8_text, max_length, 0);
    Result rc = 0;

    SwkbdConfig kbd;

    rc = swkbdCreate(&kbd, 0);

    if (R_SUCCEEDED(rc)) {
        swkbdConfigMakePresetDefault(&kbd);
        swkbdConfigSetInitialText(&kbd, vkbd.utf8_text);
        rc = swkbdShow(&kbd, vkbd.utf8_text, max_length);
        swkbdClose(&kbd);
        if (R_SUCCEEDED(rc)) {
            encoding_from_utf8(vkbd.utf8_text, vkbd.text, max_length);
        }
    }

    return vkbd.text;
}

static void switch_start_text_input(void)
{
    if (!keyboard_is_capturing()) {
        return;
    }
    const uint8_t *text = switch_keyboard_get(keyboard_get_text(), keyboard_get_max_text_length());
    keyboard_set_text(text);
}

void platform_init_callback(void)
{
    romfsInit();
    config_set(CONFIG_SCREEN_CURSOR_SCALE, CURSOR_SCALE);
    touch_set_mode(TOUCH_MODE_TOUCHPAD);
}

void platform_per_frame_callback(void)
{
    change_display_size();
    if (vkbd.requested) {
        switch_start_text_input();
        vkbd.requested = 0;
    }
}

void platform_show_virtual_keyboard(void)
{
    vkbd.requested = 1;
}

void platform_hide_virtual_keyboard(void)
{}
