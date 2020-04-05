#include <SDL_mouse.h>
#include "mouse.h"

void platform_mouse_get_relative_state(int *x, int *y)
{
    SDL_GetRelativeMouseState(x, y);
}

void platform_mouse_set_relative_mode(int enabled)
{
    SDL_SetRelativeMouseMode(enabled);
}
