#ifndef PLATFORM_TOUCH_H
#define PLATFORM_TOUCH_H

#include "SDL.h"

void platform_touch_start(SDL_TouchFingerEvent *event);
void platform_touch_update(SDL_TouchFingerEvent *event, int has_ended);

#endif // PLATFORM_TOUCH_H
