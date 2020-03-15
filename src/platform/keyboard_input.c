#include "keyboard_input.h"

#include "game/system.h"
#include "input/hotkey.h"
#include "input/keyboard.h"

static int is_ctrl_down(SDL_KeyboardEvent *event)
{
    return (event->keysym.mod & KMOD_CTRL) != 0;
}

static int is_alt_down(SDL_KeyboardEvent *event)
{
    return (event->keysym.mod & KMOD_ALT) != 0;
}

static int is_repeatable_key(SDL_Keycode code)
{
    return code == SDLK_UP || code == SDLK_DOWN ||
           code == SDLK_LEFT || code == SDLK_RIGHT ||
           code == SDLK_BACKSPACE || code == SDLK_DELETE;
}

static void send_fn(SDL_KeyboardEvent *event, int f_number)
{
    int with_any_modifier = (event->keysym.mod & (KMOD_CTRL | KMOD_SHIFT | KMOD_GUI)) != 0;
    hotkey_func(f_number, with_any_modifier, is_ctrl_down(event));
}

void platform_handle_key_down(SDL_KeyboardEvent *event)
{
    if (event->repeat && !is_repeatable_key(event->keysym.sym)) {
        // ignore multiple presses in SDL >= 2.0.5 for keys other than arrows
        // and backspace/delete to prevent hotkeys firing more than once
        return;
    }

    // Send scancodes for non-layout dependent keys
    switch (event->keysym.scancode) {
        case SDL_SCANCODE_1:
        case SDL_SCANCODE_2:
        case SDL_SCANCODE_3:
        case SDL_SCANCODE_4:
        case SDL_SCANCODE_5:
        case SDL_SCANCODE_6:
        case SDL_SCANCODE_7:
        case SDL_SCANCODE_8:
        case SDL_SCANCODE_9:
        case SDL_SCANCODE_0:
        case SDL_SCANCODE_MINUS:
        case SDL_SCANCODE_EQUALS:
            hotkey_character(*SDL_GetScancodeName(event->keysym.scancode), is_ctrl_down(event), is_alt_down(event));
            return;
        case SDL_SCANCODE_LEFTBRACKET:
            hotkey_character('[', is_ctrl_down(event), is_alt_down(event));
            return;
        case SDL_SCANCODE_RIGHTBRACKET:
            hotkey_character(']', is_ctrl_down(event), is_alt_down(event));
            return;
        default:
            break;
    }

    // Send scancodes for letters that are not on the current keyboard layout (e.g. Russian)
    if (event->keysym.scancode >= SDL_SCANCODE_A && event->keysym.scancode <= SDL_SCANCODE_Z) {
        SDL_Keycode keycode = SDLK_a + event->keysym.scancode - SDL_SCANCODE_A;
        if (SDL_GetScancodeFromKey(keycode) == 0) {
            // There is no key producing the Latin letter, send scancode value
            char letter = 'a' + event->keysym.scancode - SDL_SCANCODE_A;
            hotkey_character(letter, is_ctrl_down(event), is_alt_down(event));
            return;
        }
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
            hotkey_left_press();
            break;
        case SDLK_RIGHT:
            keyboard_right();
            hotkey_right_press();
            break;
        case SDLK_UP:
            keyboard_left();
            hotkey_up_press();
            break;
        case SDLK_DOWN:
            keyboard_right();
            hotkey_down_press();
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
        case SDLK_F1: send_fn(event, 1); break;
        case SDLK_F2: send_fn(event, 2); break;
        case SDLK_F3: send_fn(event, 3); break;
        case SDLK_F4: send_fn(event, 4); break;
        case SDLK_F5: send_fn(event, 5); break;
        case SDLK_F6: send_fn(event, 6); break;
        case SDLK_F7: send_fn(event, 7); break;
        case SDLK_F8: send_fn(event, 8); break;
        case SDLK_F9: send_fn(event, 9); break;
        case SDLK_F10: send_fn(event, 10); break;
        case SDLK_F11: send_fn(event, 11); break;
        case SDLK_F12: send_fn(event, 12); break;
        case SDLK_LEFTBRACKET:
        case SDLK_RIGHTBRACKET:
        case SDLK_SPACE:
            hotkey_character(event->keysym.sym, is_ctrl_down(event), is_alt_down(event));
            break;
        default:
            if ((event->keysym.sym & SDLK_SCANCODE_MASK) == 0) {
                // Send keycodes only for letters (layout dependent codes)
                if (event->keysym.sym >= SDLK_a && event->keysym.sym <= SDLK_z) {
                    hotkey_character(event->keysym.sym, is_ctrl_down(event), is_alt_down(event));
                }
            }
            break;
    }
}

void platform_handle_key_up(SDL_KeyboardEvent *event)
{
    switch (event->keysym.sym) {
        case SDLK_LEFT:
            hotkey_left_release();
            break;
        case SDLK_RIGHT:
            hotkey_right_release();
            break;
        case SDLK_UP:
            hotkey_up_release();
            break;
        case SDLK_DOWN:
            hotkey_down_release();
            break;
    }
}

void platform_handle_text(SDL_TextInputEvent *event)
{
    keyboard_character(event->text);
}

int system_use_virtual_keyboard(void)
{
#if defined (__vita__) || defined(__SWITCH__)
    return 1;
#else
    return 0;
#endif
}
