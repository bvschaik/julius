#include "game/system.h"

// The following includes are all required as they define PLATFORM_USE_VIRTUAL_KEYBOARD
#include "android/android.h"
#include "switch/switch.h"
#include "vita/vita.h"

#include "SDL.h"

void system_keyboard_set_input_rect(int x, int y, int width, int height)
{
    SDL_Rect rect = {x, y, width, height};
    SDL_SetTextInputRect(&rect);
}

void system_keyboard_show(void)
{
#ifdef PLATFORM_USE_VIRTUAL_KEYBOARD
    platform_show_virtual_keyboard();
#endif
}

void system_keyboard_hide(void)
{
#ifdef PLATFORM_USE_VIRTUAL_KEYBOARD
    platform_hide_virtual_keyboard();
#endif
}

void system_start_text_input(void)
{
#ifndef PLATFORM_USE_VIRTUAL_KEYBOARD
    SDL_StartTextInput();
#endif
}

void system_stop_text_input(void)
{
#ifndef PLATFORM_USE_VIRTUAL_KEYBOARD
    SDL_StopTextInput();
#endif
}
