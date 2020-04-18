#include "keyboard_input.h"

#include "game/cheats.h"
#include "game/system.h"
#include "input/hotkey.h"
#include "input/keys.h"
#include "input/keyboard.h"

static int is_alt_down(SDL_KeyboardEvent *event)
{
    return (event->keysym.mod & KMOD_ALT) != 0;
}

static key_type get_key_from_scancode(SDL_Scancode scancode)
{
    switch (scancode) {
        case SDL_SCANCODE_A: return KEY_A;
        case SDL_SCANCODE_B: return KEY_B;
        case SDL_SCANCODE_C: return KEY_C;
        case SDL_SCANCODE_D: return KEY_D;
        case SDL_SCANCODE_E: return KEY_E;
        case SDL_SCANCODE_F: return KEY_F;
        case SDL_SCANCODE_G: return KEY_G;
        case SDL_SCANCODE_H: return KEY_H;
        case SDL_SCANCODE_I: return KEY_I;
        case SDL_SCANCODE_J: return KEY_J;
        case SDL_SCANCODE_K: return KEY_K;
        case SDL_SCANCODE_L: return KEY_L;
        case SDL_SCANCODE_M: return KEY_M;
        case SDL_SCANCODE_N: return KEY_N;
        case SDL_SCANCODE_O: return KEY_O;
        case SDL_SCANCODE_P: return KEY_P;
        case SDL_SCANCODE_Q: return KEY_Q;
        case SDL_SCANCODE_R: return KEY_R;
        case SDL_SCANCODE_S: return KEY_S;
        case SDL_SCANCODE_T: return KEY_T;
        case SDL_SCANCODE_U: return KEY_U;
        case SDL_SCANCODE_V: return KEY_V;
        case SDL_SCANCODE_W: return KEY_W;
        case SDL_SCANCODE_X: return KEY_X;
        case SDL_SCANCODE_Y: return KEY_Y;
        case SDL_SCANCODE_Z: return KEY_Z;
        case SDL_SCANCODE_1: return KEY_1;
        case SDL_SCANCODE_2: return KEY_2;
        case SDL_SCANCODE_3: return KEY_3;
        case SDL_SCANCODE_4: return KEY_4;
        case SDL_SCANCODE_5: return KEY_5;
        case SDL_SCANCODE_6: return KEY_6;
        case SDL_SCANCODE_7: return KEY_7;
        case SDL_SCANCODE_8: return KEY_8;
        case SDL_SCANCODE_9: return KEY_9;
        case SDL_SCANCODE_0: return KEY_0;
        case SDL_SCANCODE_RETURN: return KEY_ENTER;
        case SDL_SCANCODE_ESCAPE: return KEY_ESCAPE;
        case SDL_SCANCODE_BACKSPACE: return KEY_BACKSPACE;
        case SDL_SCANCODE_TAB: return KEY_TAB;
        case SDL_SCANCODE_SPACE: return KEY_SPACE;
        case SDL_SCANCODE_MINUS: return KEY_MINUS;
        case SDL_SCANCODE_EQUALS: return KEY_EQUALS;
        case SDL_SCANCODE_LEFTBRACKET: return KEY_LEFTBRACKET;
        case SDL_SCANCODE_RIGHTBRACKET: return KEY_RIGHTBRACKET;
        case SDL_SCANCODE_BACKSLASH: return KEY_BACKSLASH;
        case SDL_SCANCODE_SEMICOLON: return KEY_SEMICOLON;
        case SDL_SCANCODE_APOSTROPHE: return KEY_APOSTROPHE;
        case SDL_SCANCODE_GRAVE: return KEY_GRAVE;
        case SDL_SCANCODE_COMMA: return KEY_COMMA;
        case SDL_SCANCODE_PERIOD: return KEY_PERIOD;
        case SDL_SCANCODE_SLASH: return KEY_SLASH;
        case SDL_SCANCODE_F1: return KEY_F1;
        case SDL_SCANCODE_F2: return KEY_F2;
        case SDL_SCANCODE_F3: return KEY_F3;
        case SDL_SCANCODE_F4: return KEY_F4;
        case SDL_SCANCODE_F5: return KEY_F5;
        case SDL_SCANCODE_F6: return KEY_F6;
        case SDL_SCANCODE_F7: return KEY_F7;
        case SDL_SCANCODE_F8: return KEY_F8;
        case SDL_SCANCODE_F9: return KEY_F9;
        case SDL_SCANCODE_F10: return KEY_F10;
        case SDL_SCANCODE_F11: return KEY_F11;
        case SDL_SCANCODE_F12: return KEY_F12;
        case SDL_SCANCODE_INSERT: return KEY_INSERT;
        case SDL_SCANCODE_HOME: return KEY_HOME;
        case SDL_SCANCODE_PAGEUP: return KEY_PAGEUP;
        case SDL_SCANCODE_DELETE: return KEY_DELETE;
        case SDL_SCANCODE_END: return KEY_END;
        case SDL_SCANCODE_PAGEDOWN: return KEY_PAGEDOWN;
        case SDL_SCANCODE_RIGHT: return KEY_RIGHT;
        case SDL_SCANCODE_LEFT: return KEY_LEFT;
        case SDL_SCANCODE_DOWN: return KEY_DOWN;
        case SDL_SCANCODE_UP: return KEY_UP;
        case SDL_SCANCODE_KP_MINUS: return KEY_MINUS;
        case SDL_SCANCODE_KP_PLUS: return KEY_EQUALS;
        case SDL_SCANCODE_KP_ENTER: return KEY_ENTER;
        case SDL_SCANCODE_KP_1: return KEY_1;
        case SDL_SCANCODE_KP_2: return KEY_2;
        case SDL_SCANCODE_KP_3: return KEY_3;
        case SDL_SCANCODE_KP_4: return KEY_4;
        case SDL_SCANCODE_KP_5: return KEY_5;
        case SDL_SCANCODE_KP_6: return KEY_6;
        case SDL_SCANCODE_KP_7: return KEY_7;
        case SDL_SCANCODE_KP_8: return KEY_8;
        case SDL_SCANCODE_KP_9: return KEY_9;
        case SDL_SCANCODE_KP_0: return KEY_0;
        case SDL_SCANCODE_KP_PERIOD: return KEY_PERIOD;
        case SDL_SCANCODE_KP_EQUALS: return KEY_EQUALS;
        default: return KEY_NONE;
    }
}

static key_modifier_type get_modifier(int mod)
{
    key_modifier_type key_mod = KEY_MOD_NONE;
    if (mod & KMOD_SHIFT) {
        key_mod |= KEY_MOD_SHIFT;
    }
    if (mod & KMOD_CTRL) {
        key_mod |= KEY_MOD_CTRL;
    }
    if (mod & KMOD_ALT) {
        key_mod |= KEY_MOD_ALT;
    }
    if (mod & KMOD_GUI) {
        key_mod |= KEY_MOD_GUI;
    }
    return key_mod;
}

void platform_handle_key_down(SDL_KeyboardEvent *event)
{
    // handle keyboard input keys
    switch (event->keysym.sym) {
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            // only send enter if no modifier is also down
            if ((event->keysym.mod & (KMOD_CTRL | KMOD_ALT | KMOD_GUI)) == 0) {
                keyboard_return();
            }
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
            break;
        case SDLK_RIGHT:
            keyboard_right();
            break;
        case SDLK_UP:
            keyboard_left();
            break;
        case SDLK_DOWN:
            keyboard_right();
            break;
        case SDLK_HOME:
            keyboard_home();
            break;
        case SDLK_END:
            keyboard_end();
            break;
    }

    // handle hotkeys
    key_type key = get_key_from_scancode(event->keysym.scancode);
    key_modifier_type mod = get_modifier(event->keysym.mod);

    if (key != KEY_NONE) {
        hotkey_key_pressed(key, mod, event->repeat);
    }

    // handle cheats: special case since they ARE layout dependent
    if (!event->repeat && is_alt_down(event)) {
        switch (event->keysym.sym) {
            case SDLK_k:
                game_cheat_activate();
                break;
            case SDLK_c:
                game_cheat_money();
                break;
            case SDLK_v:
                game_cheat_victory();
                break;
        }
    }
}

void platform_handle_key_up(SDL_KeyboardEvent *event)
{
    key_type key = get_key_from_scancode(event->keysym.scancode);
    if (key != KEY_NONE) {
        hotkey_key_released(key);
    }
}

void platform_handle_text(SDL_TextInputEvent *event)
{
    keyboard_character(event->text);
}

key_type system_keyboard_key_for_symbol(const char *name)
{
    SDL_Keycode keycode = SDL_GetKeyFromName(name);
    if (keycode == SDLK_UNKNOWN) {
        return KEY_NONE;
    }
    SDL_Scancode scancode = SDL_GetScancodeFromKey(keycode);
    if (scancode == SDL_SCANCODE_UNKNOWN) {
        return KEY_NONE;
    }
    return get_key_from_scancode(scancode);
}
