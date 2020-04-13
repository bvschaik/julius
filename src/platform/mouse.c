#include <SDL_mouse.h>
#include "mouse.h"

void platform_mouse_get_relative_state(int *x, int *y)
{
    SDL_GetRelativeMouseState(x, y);
}

void platform_mouse_set_relative_mode(int enabled)
{
    SDL_SetRelativeMouseMode(enabled);

    if (enabled) {
        // Discard the first value, which is incorrect (the first one gives the relative position to center of window)
        platform_mouse_get_relative_state(NULL, NULL);
    }
}
