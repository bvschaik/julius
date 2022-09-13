#include "keyboard_input.h"

#include "game/cheats.h"
#include "game/system.h"
#include "input/hotkey.h"
#include "input/keys.h"
#include "input/keyboard.h"
#include "input/mouse.h"

static int is_alt_down(SDL_KeyboardEvent *event)
{
    return (event->keysym.mod & KMOD_ALT) != 0;
}

static key_type get_key_from_scancode(SDL_Scancode scancode)
{
    switch (scancode) {
        case SDL_SCANCODE_A: return KEY_TYPE_A;
        case SDL_SCANCODE_B: return KEY_TYPE_B;
        case SDL_SCANCODE_C: return KEY_TYPE_C;
        case SDL_SCANCODE_D: return KEY_TYPE_D;
        case SDL_SCANCODE_E: return KEY_TYPE_E;
        case SDL_SCANCODE_F: return KEY_TYPE_F;
        case SDL_SCANCODE_G: return KEY_TYPE_G;
        case SDL_SCANCODE_H: return KEY_TYPE_H;
        case SDL_SCANCODE_I: return KEY_TYPE_I;
        case SDL_SCANCODE_J: return KEY_TYPE_J;
        case SDL_SCANCODE_K: return KEY_TYPE_K;
        case SDL_SCANCODE_L: return KEY_TYPE_L;
        case SDL_SCANCODE_M: return KEY_TYPE_M;
        case SDL_SCANCODE_N: return KEY_TYPE_N;
        case SDL_SCANCODE_O: return KEY_TYPE_O;
        case SDL_SCANCODE_P: return KEY_TYPE_P;
        case SDL_SCANCODE_Q: return KEY_TYPE_Q;
        case SDL_SCANCODE_R: return KEY_TYPE_R;
        case SDL_SCANCODE_S: return KEY_TYPE_S;
        case SDL_SCANCODE_T: return KEY_TYPE_T;
        case SDL_SCANCODE_U: return KEY_TYPE_U;
        case SDL_SCANCODE_V: return KEY_TYPE_V;
        case SDL_SCANCODE_W: return KEY_TYPE_W;
        case SDL_SCANCODE_X: return KEY_TYPE_X;
        case SDL_SCANCODE_Y: return KEY_TYPE_Y;
        case SDL_SCANCODE_Z: return KEY_TYPE_Z;
        case SDL_SCANCODE_1: return KEY_TYPE_1;
        case SDL_SCANCODE_2: return KEY_TYPE_2;
        case SDL_SCANCODE_3: return KEY_TYPE_3;
        case SDL_SCANCODE_4: return KEY_TYPE_4;
        case SDL_SCANCODE_5: return KEY_TYPE_5;
        case SDL_SCANCODE_6: return KEY_TYPE_6;
        case SDL_SCANCODE_7: return KEY_TYPE_7;
        case SDL_SCANCODE_8: return KEY_TYPE_8;
        case SDL_SCANCODE_9: return KEY_TYPE_9;
        case SDL_SCANCODE_0: return KEY_TYPE_0;
        case SDL_SCANCODE_RETURN: return KEY_TYPE_ENTER;
        case SDL_SCANCODE_ESCAPE: return KEY_TYPE_ESCAPE;
        case SDL_SCANCODE_BACKSPACE: return KEY_TYPE_BACKSPACE;
        case SDL_SCANCODE_TAB: return KEY_TYPE_TAB;
        case SDL_SCANCODE_SPACE: return KEY_TYPE_SPACE;
        case SDL_SCANCODE_MINUS: return KEY_TYPE_MINUS;
        case SDL_SCANCODE_EQUALS: return KEY_TYPE_EQUALS;
        case SDL_SCANCODE_LEFTBRACKET: return KEY_TYPE_LEFTBRACKET;
        case SDL_SCANCODE_RIGHTBRACKET: return KEY_TYPE_RIGHTBRACKET;
        case SDL_SCANCODE_BACKSLASH: return KEY_TYPE_BACKSLASH;
        case SDL_SCANCODE_SEMICOLON: return KEY_TYPE_SEMICOLON;
        case SDL_SCANCODE_APOSTROPHE: return KEY_TYPE_APOSTROPHE;
        case SDL_SCANCODE_GRAVE: return KEY_TYPE_GRAVE;
        case SDL_SCANCODE_COMMA: return KEY_TYPE_COMMA;
        case SDL_SCANCODE_PERIOD: return KEY_TYPE_PERIOD;
        case SDL_SCANCODE_SLASH: return KEY_TYPE_SLASH;
        case SDL_SCANCODE_F1: return KEY_TYPE_F1;
        case SDL_SCANCODE_F2: return KEY_TYPE_F2;
        case SDL_SCANCODE_F3: return KEY_TYPE_F3;
        case SDL_SCANCODE_F4: return KEY_TYPE_F4;
        case SDL_SCANCODE_F5: return KEY_TYPE_F5;
        case SDL_SCANCODE_F6: return KEY_TYPE_F6;
        case SDL_SCANCODE_F7: return KEY_TYPE_F7;
        case SDL_SCANCODE_F8: return KEY_TYPE_F8;
        case SDL_SCANCODE_F9: return KEY_TYPE_F9;
        case SDL_SCANCODE_F10: return KEY_TYPE_F10;
        case SDL_SCANCODE_F11: return KEY_TYPE_F11;
        case SDL_SCANCODE_F12: return KEY_TYPE_F12;
        case SDL_SCANCODE_INSERT: return KEY_TYPE_INSERT;
        case SDL_SCANCODE_HOME: return KEY_TYPE_HOME;
        case SDL_SCANCODE_PAGEUP: return KEY_TYPE_PAGEUP;
        case SDL_SCANCODE_DELETE: return KEY_TYPE_DELETE;
        case SDL_SCANCODE_END: return KEY_TYPE_END;
        case SDL_SCANCODE_PAGEDOWN: return KEY_TYPE_PAGEDOWN;
        case SDL_SCANCODE_RIGHT: return KEY_TYPE_RIGHT;
        case SDL_SCANCODE_LEFT: return KEY_TYPE_LEFT;
        case SDL_SCANCODE_DOWN: return KEY_TYPE_DOWN;
        case SDL_SCANCODE_UP: return KEY_TYPE_UP;
        case SDL_SCANCODE_KP_ENTER: return KEY_TYPE_ENTER;
        case SDL_SCANCODE_KP_1: return KEY_TYPE_KP_1;
        case SDL_SCANCODE_KP_2: return KEY_TYPE_KP_2;
        case SDL_SCANCODE_KP_3: return KEY_TYPE_KP_3;
        case SDL_SCANCODE_KP_4: return KEY_TYPE_KP_4;
        case SDL_SCANCODE_KP_5: return KEY_TYPE_KP_5;
        case SDL_SCANCODE_KP_6: return KEY_TYPE_KP_6;
        case SDL_SCANCODE_KP_7: return KEY_TYPE_KP_7;
        case SDL_SCANCODE_KP_8: return KEY_TYPE_KP_8;
        case SDL_SCANCODE_KP_9: return KEY_TYPE_KP_9;
        case SDL_SCANCODE_KP_0: return KEY_TYPE_KP_0;
        case SDL_SCANCODE_KP_PERIOD: return KEY_TYPE_KP_PERIOD;
        case SDL_SCANCODE_KP_PLUS: return KEY_TYPE_KP_PLUS;
        case SDL_SCANCODE_KP_MINUS: return KEY_TYPE_KP_MINUS;
        case SDL_SCANCODE_KP_MULTIPLY: return KEY_TYPE_KP_MULTIPLY;
        case SDL_SCANCODE_KP_DIVIDE: return KEY_TYPE_KP_DIVIDE;
        case SDL_SCANCODE_NONUSBACKSLASH: return KEY_TYPE_NON_US;
        default: return KEY_TYPE_NONE;
    }
}

static SDL_Scancode get_scancode_from_key(key_type key)
{
    switch (key) {
        case KEY_TYPE_A: return SDL_SCANCODE_A;
        case KEY_TYPE_B: return SDL_SCANCODE_B;
        case KEY_TYPE_C: return SDL_SCANCODE_C;
        case KEY_TYPE_D: return SDL_SCANCODE_D;
        case KEY_TYPE_E: return SDL_SCANCODE_E;
        case KEY_TYPE_F: return SDL_SCANCODE_F;
        case KEY_TYPE_G: return SDL_SCANCODE_G;
        case KEY_TYPE_H: return SDL_SCANCODE_H;
        case KEY_TYPE_I: return SDL_SCANCODE_I;
        case KEY_TYPE_J: return SDL_SCANCODE_J;
        case KEY_TYPE_K: return SDL_SCANCODE_K;
        case KEY_TYPE_L: return SDL_SCANCODE_L;
        case KEY_TYPE_M: return SDL_SCANCODE_M;
        case KEY_TYPE_N: return SDL_SCANCODE_N;
        case KEY_TYPE_O: return SDL_SCANCODE_O;
        case KEY_TYPE_P: return SDL_SCANCODE_P;
        case KEY_TYPE_Q: return SDL_SCANCODE_Q;
        case KEY_TYPE_R: return SDL_SCANCODE_R;
        case KEY_TYPE_S: return SDL_SCANCODE_S;
        case KEY_TYPE_T: return SDL_SCANCODE_T;
        case KEY_TYPE_U: return SDL_SCANCODE_U;
        case KEY_TYPE_V: return SDL_SCANCODE_V;
        case KEY_TYPE_W: return SDL_SCANCODE_W;
        case KEY_TYPE_X: return SDL_SCANCODE_X;
        case KEY_TYPE_Y: return SDL_SCANCODE_Y;
        case KEY_TYPE_Z: return SDL_SCANCODE_Z;
        case KEY_TYPE_1: return SDL_SCANCODE_1;
        case KEY_TYPE_2: return SDL_SCANCODE_2;
        case KEY_TYPE_3: return SDL_SCANCODE_3;
        case KEY_TYPE_4: return SDL_SCANCODE_4;
        case KEY_TYPE_5: return SDL_SCANCODE_5;
        case KEY_TYPE_6: return SDL_SCANCODE_6;
        case KEY_TYPE_7: return SDL_SCANCODE_7;
        case KEY_TYPE_8: return SDL_SCANCODE_8;
        case KEY_TYPE_9: return SDL_SCANCODE_9;
        case KEY_TYPE_0: return SDL_SCANCODE_0;
        case KEY_TYPE_ENTER: return SDL_SCANCODE_RETURN;
        case KEY_TYPE_ESCAPE: return SDL_SCANCODE_ESCAPE;
        case KEY_TYPE_BACKSPACE: return SDL_SCANCODE_BACKSPACE;
        case KEY_TYPE_TAB: return SDL_SCANCODE_TAB;
        case KEY_TYPE_SPACE: return SDL_SCANCODE_SPACE;
        case KEY_TYPE_MINUS: return SDL_SCANCODE_MINUS;
        case KEY_TYPE_EQUALS: return SDL_SCANCODE_EQUALS;
        case KEY_TYPE_LEFTBRACKET: return SDL_SCANCODE_LEFTBRACKET;
        case KEY_TYPE_RIGHTBRACKET: return SDL_SCANCODE_RIGHTBRACKET;
        case KEY_TYPE_BACKSLASH: return SDL_SCANCODE_BACKSLASH;
        case KEY_TYPE_SEMICOLON: return SDL_SCANCODE_SEMICOLON;
        case KEY_TYPE_APOSTROPHE: return SDL_SCANCODE_APOSTROPHE;
        case KEY_TYPE_GRAVE: return SDL_SCANCODE_GRAVE;
        case KEY_TYPE_COMMA: return SDL_SCANCODE_COMMA;
        case KEY_TYPE_PERIOD: return SDL_SCANCODE_PERIOD;
        case KEY_TYPE_SLASH: return SDL_SCANCODE_SLASH;
        case KEY_TYPE_F1: return SDL_SCANCODE_F1;
        case KEY_TYPE_F2: return SDL_SCANCODE_F2;
        case KEY_TYPE_F3: return SDL_SCANCODE_F3;
        case KEY_TYPE_F4: return SDL_SCANCODE_F4;
        case KEY_TYPE_F5: return SDL_SCANCODE_F5;
        case KEY_TYPE_F6: return SDL_SCANCODE_F6;
        case KEY_TYPE_F7: return SDL_SCANCODE_F7;
        case KEY_TYPE_F8: return SDL_SCANCODE_F8;
        case KEY_TYPE_F9: return SDL_SCANCODE_F9;
        case KEY_TYPE_F10: return SDL_SCANCODE_F10;
        case KEY_TYPE_F11: return SDL_SCANCODE_F11;
        case KEY_TYPE_F12: return SDL_SCANCODE_F12;
        case KEY_TYPE_INSERT: return SDL_SCANCODE_INSERT;
        case KEY_TYPE_HOME: return SDL_SCANCODE_HOME;
        case KEY_TYPE_PAGEUP: return SDL_SCANCODE_PAGEUP;
        case KEY_TYPE_DELETE: return SDL_SCANCODE_DELETE;
        case KEY_TYPE_END: return SDL_SCANCODE_END;
        case KEY_TYPE_PAGEDOWN: return SDL_SCANCODE_PAGEDOWN;
        case KEY_TYPE_RIGHT: return SDL_SCANCODE_RIGHT;
        case KEY_TYPE_LEFT: return SDL_SCANCODE_LEFT;
        case KEY_TYPE_DOWN: return SDL_SCANCODE_DOWN;
        case KEY_TYPE_UP: return SDL_SCANCODE_UP;
        case KEY_TYPE_KP_1: return SDL_SCANCODE_KP_1;
        case KEY_TYPE_KP_2: return SDL_SCANCODE_KP_2;
        case KEY_TYPE_KP_3: return SDL_SCANCODE_KP_3;
        case KEY_TYPE_KP_4: return SDL_SCANCODE_KP_4;
        case KEY_TYPE_KP_5: return SDL_SCANCODE_KP_5;
        case KEY_TYPE_KP_6: return SDL_SCANCODE_KP_6;
        case KEY_TYPE_KP_7: return SDL_SCANCODE_KP_7;
        case KEY_TYPE_KP_8: return SDL_SCANCODE_KP_8;
        case KEY_TYPE_KP_9: return SDL_SCANCODE_KP_9;
        case KEY_TYPE_KP_0: return SDL_SCANCODE_KP_0;
        case KEY_TYPE_KP_PERIOD: return SDL_SCANCODE_KP_PERIOD;
        case KEY_TYPE_KP_PLUS: return SDL_SCANCODE_KP_PLUS;
        case KEY_TYPE_KP_MINUS: return SDL_SCANCODE_KP_MINUS;
        case KEY_TYPE_KP_MULTIPLY: return SDL_SCANCODE_KP_MULTIPLY;
        case KEY_TYPE_KP_DIVIDE: return SDL_SCANCODE_KP_DIVIDE;
        case KEY_TYPE_NON_US: return SDL_SCANCODE_NONUSBACKSLASH;
        default: return SDL_SCANCODE_UNKNOWN;
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
        case SDLK_AC_BACK:
#ifndef __ANDROID__
            event->keysym.scancode = SDL_SCANCODE_ESCAPE;
            break;
#else
        // Hack: since Android handles the right mouse button as a back button
        // (or even as an "ESC" keypress) and SDL doesn't yet have a proper implementation 
        // for this, we'll treat the back button as a right mouse button when the mouse is active
        case SDLK_ESCAPE:
            if (!mouse_get()->is_touch) {
                mouse_set_right_down(1);
                return;
            } else {
                event->keysym.scancode = SDL_SCANCODE_ESCAPE;
            }
            break;
#endif
    }

    // handle hotkeys
    key_type key = get_key_from_scancode(event->keysym.scancode);
    key_modifier_type mod = get_modifier(event->keysym.mod);
    hotkey_key_pressed(key, mod, event->repeat);

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
            case SDLK_b:
                game_cheat_breakpoint();
                break;
            case SDLK_x:
                game_cheat_console();
        }
    }
}

void platform_handle_key_up(SDL_KeyboardEvent *event)
{
#ifdef __ANDROID__
    // Right mouse button hack: read above for explanation
    if ((event->keysym.sym == SDLK_ESCAPE || event->keysym.sym == SDLK_AC_BACK) && !mouse_get()->is_touch) {
        mouse_set_right_down(0);
        return;
    }
#endif
    key_type key = get_key_from_scancode(event->keysym.scancode);
    key_modifier_type mod = get_modifier(event->keysym.mod);
    hotkey_key_released(key, mod);
}

void platform_handle_editing_text(SDL_TextEditingEvent *event)
{
    keyboard_editing_text(event->text);
}

void platform_handle_text(SDL_TextInputEvent *event)
{
    keyboard_text(event->text);
}

key_type system_keyboard_key_for_symbol(const char *name)
{
    SDL_Keycode keycode = SDL_GetKeyFromName(name);
    if (keycode == SDLK_UNKNOWN) {
        return KEY_TYPE_NONE;
    }
    SDL_Scancode scancode = SDL_GetScancodeFromKey(keycode);
    if (scancode == SDL_SCANCODE_UNKNOWN) {
        return KEY_TYPE_NONE;
    }
    return get_key_from_scancode(scancode);
}

const char *system_keyboard_key_name(key_type key)
{
    SDL_Scancode scancode = get_scancode_from_key(key);
    return SDL_GetKeyName(SDL_GetKeyFromScancode(scancode));
}

const char *system_keyboard_key_modifier_name(key_modifier_type modifier)
{
    switch (modifier) {
        case KEY_MOD_CTRL: return "Ctrl";
        case KEY_MOD_SHIFT: return "Shift";
        case KEY_MOD_GUI:
#ifdef __APPLE__
            return "Cmd";
#else
            return "Gui";
#endif
        case KEY_MOD_ALT: return "Alt";
        default: return "";
    }
}
