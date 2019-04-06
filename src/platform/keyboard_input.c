#include "keyboard_input.h"

#include "input/hotkey.h"
#include "input/keyboard.h"

static int is_repeatable_key(SDL_Keycode code)
{
    return code == SDLK_UP || code == SDLK_DOWN ||
           code == SDLK_LEFT || code == SDLK_RIGHT ||
           code == SDLK_BACKSPACE || code == SDLK_DELETE;
}

void platform_handle_key_down(SDL_KeyboardEvent *event)
{
    if (event->repeat && !is_repeatable_key(event->keysym.sym)) {
        // ignore multiple presses in SDL >= 2.0.5 for keys other than arrows
        // and backspace/delete to prevent hotkeys firing more than once
        return;
    }
    switch (event->keysym.sym) {
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            keyboard_return();
            hotkey_enter();
            break;
        case SDLK_BACKSPACE:
            keyboard_backspace();
            break;
        case SDLK_DELETE:
            keyboard_delete();
            break;
        case SDLK_INSERT:
            keyboard_insert();
            break;
        case SDLK_LEFT:
            keyboard_left();
            hotkey_left();
            break;
        case SDLK_RIGHT:
            keyboard_right();
            hotkey_right();
            break;
        case SDLK_UP:
            keyboard_left();
            hotkey_up();
            break;
        case SDLK_DOWN:
            keyboard_right();
            hotkey_down();
            break;
        case SDLK_HOME:
            keyboard_home();
            hotkey_home();
            break;
        case SDLK_END:
            keyboard_end();
            hotkey_end();
            break;
        case SDLK_PAGEUP:
            hotkey_page_up();
            break;
        case SDLK_PAGEDOWN:
            hotkey_page_down();
            break;
        case SDLK_ESCAPE:
            hotkey_esc();
            break;
        case SDLK_F1: hotkey_func(1); break;
        case SDLK_F2: hotkey_func(2); break;
        case SDLK_F3: hotkey_func(3); break;
        case SDLK_F4: hotkey_func(4); break;
        case SDLK_F5: hotkey_func(5); break;
        case SDLK_F6: hotkey_func(6); break;
        case SDLK_F7: hotkey_func(7); break;
        case SDLK_F8: hotkey_func(8); break;
        case SDLK_F9: hotkey_func(9); break;
        case SDLK_F10: hotkey_func(10); break;
        case SDLK_F11: hotkey_func(11); break;
        case SDLK_F12: hotkey_func(12); break;
        case SDLK_LCTRL:
        case SDLK_RCTRL:
            hotkey_ctrl(1);
            break;
        case SDLK_LALT:
        case SDLK_RALT:
            hotkey_alt(1);
            break;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            hotkey_shift(1);
            break;
        case SDLK_LEFTBRACKET:
        case SDLK_RIGHTBRACKET:
            hotkey_character(event->keysym.sym);
            break;
        default:
            if ((event->keysym.sym & SDLK_SCANCODE_MASK) == 0) {
                // Send key codes only for letters (layout dependent codes)
                // Other keys like numbers seem to be non-layout dependent
                if (event->keysym.sym >= 97 && event->keysym.sym <= 122)
                    hotkey_character(event->keysym.sym);
                else
                    hotkey_character(event->keysym.scancode);
            }
            break;
    }
}

void platform_handle_key_up(SDL_KeyboardEvent *event)
{
    switch (event->keysym.sym) {
        case SDLK_LCTRL:
        case SDLK_RCTRL:
            hotkey_ctrl(0);
            break;
        case SDLK_LALT:
        case SDLK_RALT:
            hotkey_alt(0);
            break;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            hotkey_shift(0);
            break;
        default:
            break;
    }
}

void platform_handle_text(SDL_TextInputEvent *event)
{
    if (event->text[0] && !event->text[1]) {
        // only accept non-multibyte chars
        keyboard_character(event->text[0]);
    }
}
