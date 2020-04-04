#include "joystick.h"

#include "input/joystick.h"

#include "SDL.h"

#define INVALID_JOYSTICK -1

static int add_joystick(int index)
{
    SDL_JoystickOpen(index);
    return INVALID_JOYSTICK;
}

void platform_joystick_init(void)
{
    if (SDL_JoystickEventState(SDL_ENABLE) != SDL_ENABLE) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Joystick events could not be enabled: %s", SDL_GetError());
    }
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        add_joystick(i);
    }
}

void platform_joystick_device_changed(int id, int is_connected)
{
    if (is_connected) {
        add_joystick(id);
    } else {
        // remove_joystick(id);
    }
}

void platform_joystick_handle_axis(void)
{}

void platform_joystick_handle_trackball(void)
{}

void platform_joystick_handle_hat(void)
{}

void platform_joystick_handle_button(int is_down)
{}
