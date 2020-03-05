#include "virtual_keyboard.h"

#include "game/system.h"

#include "SDL.h"

static int active;

void platform_virtual_keyboard_start(void)
{
    if (system_use_virtual_keyboard()) {
        SDL_StopTextInput();
    }
    active = 1;
}

void platform_virtual_keyboard_resume(void)
{
    active = 1;
}

void platform_virtual_keyboard_stop(void)
{
    if (system_use_virtual_keyboard()) {
        SDL_StopTextInput();
    }
    active = 0;
}

void platform_virtual_keyboard_show(void)
{
    if (active && system_use_virtual_keyboard()) {
        SDL_StartTextInput();
    }
}

void platform_virtual_keyboard_hide(void)
{
    if (active && system_use_virtual_keyboard()) {
        SDL_StopTextInput();
    }
}

int platform_virtual_keyboard_showing(void)
{
    return SDL_IsTextInputActive();
}