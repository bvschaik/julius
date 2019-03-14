#ifndef VITA_TOUCH_H
#define VITA_TOUCH_H

#include <SDL2/SDL.h>
#include <psp2/touch.h>
#include <stdbool.h>

void vita_handle_touch(SDL_Event *event);
void vita_finish_simulated_mouse_clicks(void);

#endif /* VITA_TOUCH_H */
