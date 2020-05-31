#include "game/system.h"
#include "input/mouse.h"
#include "platform/screen.h"

#include <SDL_mouse.h>

static struct {
    int x;
    int y;
    int enabled;
} data;

void system_mouse_get_relative_state(int *x, int *y)
{
    SDL_GetRelativeMouseState(x, y);
}

void system_mouse_set_relative_mode(int enabled)
{
    if (enabled == data.enabled) {
        return;
    }
    if (enabled) {
        SDL_GetMouseState(&data.x, &data.y);
        SDL_SetRelativeMouseMode(SDL_TRUE);
        // Discard the first value, which is incorrect
        // (the first one gives the relative position to center of window)
        system_mouse_get_relative_state(NULL, NULL);
    } else {
        SDL_SetRelativeMouseMode(SDL_FALSE);
        platform_screen_warp_mouse(data.x, data.y);
        mouse_set_position(data.x, data.y);
    }
    data.enabled = enabled;
}
