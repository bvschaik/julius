#include "game/system.h"

#include "switch/switch.h"
#include "vita/vita.h"

#include "SDL.h"

void system_keyboard_set_input_rect(int x, int y, int width, int height)
{
    SDL_Rect rect = {x, y, width, height};
    SDL_SetTextInputRect(&rect);
}

void system_keyboard_show(const uint8_t *text, int max_length)
{
#ifdef PLATFORM_USE_VIRTUAL_KEYBOARD
    platform_show_virtual_keyboard(text, max_length);
#endif
}

void system_keyboard_hide(void)
{
#ifdef PLATFORM_USE_VIRTUAL_KEYBOARD
    platform_hide_virtual_keyboard();
#endif
}
