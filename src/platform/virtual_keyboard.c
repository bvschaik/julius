#include "virtual_keyboard.h"

#include "game/system.h"

#include "SDL.h"

void platform_virtual_keyboard_show(void)
{
    if (system_use_virtual_keyboard()) {
        SDL_StartTextInput();
    }
}

void platform_virtual_keyboard_hide(void)
{
    if (system_use_virtual_keyboard()) {
        SDL_StopTextInput();
    }
}

int platform_virtual_keyboard_showing(void)
{
    return SDL_IsTextInputActive();
}
