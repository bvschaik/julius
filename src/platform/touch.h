#ifndef PLATFORM_TOUCH_H
#define PLATFORM_TOUCH_H

#include "SDL.h"

void platform_touch_start(SDL_TouchFingerEvent *event);
void platform_touch_move(SDL_TouchFingerEvent *event);
void platform_touch_end(SDL_TouchFingerEvent *event);

#endif // PLATFORM_TOUCH_H
