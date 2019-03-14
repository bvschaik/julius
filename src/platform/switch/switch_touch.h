#ifndef SWITCH_TOUCH_H
#define SWITCH_TOUCH_H

#include <SDL2/SDL.h>
#include <stdbool.h>

void switch_handle_touch(SDL_Event *event);
void switch_finish_simulated_mouse_clicks(void);

#endif /* SWITCH_TOUCH_H */
