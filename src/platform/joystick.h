#ifndef PLATFORM_JOYSTICK_H
#define PLATFORM_JOYSTICK_H

#include "SDL.h"

void platform_joystick_init(void);
void platform_joystick_device_changed(int id, int is_connected);
void platform_joystick_handle_axis(SDL_JoyAxisEvent *event);
void platform_joystick_handle_trackball(SDL_JoyBallEvent *event);
void platform_joystick_handle_hat(SDL_JoyHatEvent *event);
void platform_joystick_handle_button(SDL_JoyButtonEvent *event, int is_down);

#endif // PLATFORM_JOYSTICK_H
