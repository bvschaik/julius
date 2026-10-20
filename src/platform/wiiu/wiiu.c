#include "core/calc.h"
#include "core/config.h"
#include "core/encoding.h"
#include "core/string.h"
#include "game/system.h"
#include "graphics/screen.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include "input/touch.h"
#include "platform/wiiu/wiiu.h"

#include "SDL.h"
#include "SDL_syswm.h"

#define SCREEN_SCALE 150
#define CURSOR_SCALE 150

#define MAX_VKBD_TEXT_SIZE 600

enum {
    VKBD_CLOSED,
    VKBD_OPEN,
    VKBD_RECEIVING,
    VKBD_DONE
};

static struct {
    int state;
    int accepted;
    uint8_t text[MAX_VKBD_TEXT_SIZE];
    char utf8_text[MAX_VKBD_TEXT_SIZE];
} vkbd;

static void center_mouse_cursor(void)
{
    int x = screen_width() / 2;
    int y = screen_height() / 2;
    system_set_mouse_position(&x, &y);
    mouse_set_position(x, y);
}

// SDL resends the swkbd text every frame until the keyboard is closed
static int SDLCALL keyboard_event_filter(void *userdata, SDL_Event *event)
{
    if (event->type == SDL_SYSWMEVENT) {
        if (event->syswm.msg && event->syswm.msg->subsystem == SDL_SYSWM_WIIU &&
            (vkbd.state == VKBD_OPEN || vkbd.state == VKBD_RECEIVING)) {
            switch (event->syswm.msg->msg.wiiu.event) {
                case SDL_WIIU_SYSWM_SWKBD_OK_START_EVENT:
                    vkbd.utf8_text[0] = 0;
                    vkbd.state = VKBD_RECEIVING;
                    break;
                case SDL_WIIU_SYSWM_SWKBD_OK_FINISH_EVENT:
                    vkbd.accepted = 1;
                    vkbd.state = VKBD_DONE;
                    break;
                case SDL_WIIU_SYSWM_SWKBD_CANCEL_EVENT:
                    vkbd.state = VKBD_DONE;
                    break;
            }
        }
        return 0;
    }
    if (event->type == SDL_TEXTINPUT && vkbd.state != VKBD_CLOSED) {
        if (vkbd.state == VKBD_RECEIVING) {
            SDL_strlcat(vkbd.utf8_text, event->text.text, MAX_VKBD_TEXT_SIZE);
        }
        return 0;
    }
    return 1;
}

void platform_init_callback(void)
{
    config_set(CONFIG_SCREEN_CURSOR_SCALE, CURSOR_SCALE);
    system_init_cursors(CURSOR_SCALE);
    system_scale_display(SCREEN_SCALE);
    touch_set_mode(TOUCH_MODE_ORIGINAL);
    center_mouse_cursor();
}

void platform_per_frame_callback(void)
{
    if (vkbd.state != VKBD_DONE) {
        return;
    }
    SDL_StopTextInput();
    if (vkbd.accepted) {
        encoding_from_utf8(vkbd.utf8_text, vkbd.text, MAX_VKBD_TEXT_SIZE);
        keyboard_set_text(vkbd.text);
    }
    vkbd.accepted = 0;
    vkbd.state = VKBD_CLOSED;
}

void platform_show_virtual_keyboard(void)
{
    if (!keyboard_is_capturing() || vkbd.state != VKBD_CLOSED) {
        return;
    }
    static int filter_installed;
    if (!filter_installed) {
        // setting a filter flushes the event queue
        SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
        SDL_SetEventFilter(keyboard_event_filter, 0);
        filter_installed = 1;
    }
    int max_length = calc_bound(keyboard_get_max_text_length(), 0, MAX_VKBD_TEXT_SIZE);
    string_copy(keyboard_get_text(), vkbd.text, max_length);
    encoding_to_utf8(vkbd.text, vkbd.utf8_text, MAX_VKBD_TEXT_SIZE, 0);
    SDL_WiiUSetSWKBDInitialText(vkbd.utf8_text);
    SDL_WiiUSetSWKBDHighlightInitialText(SDL_TRUE);
    vkbd.state = VKBD_OPEN;
    SDL_StartTextInput();
}

void platform_hide_virtual_keyboard(void)
{
    SDL_StopTextInput();
    vkbd.accepted = 0;
    vkbd.state = VKBD_CLOSED;
}
