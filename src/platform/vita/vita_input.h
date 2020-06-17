#ifndef VITA_INPUT_H
#define VITA_INPUT_H

#include <SDL2/SDL.h>

int vita_poll_event(SDL_Event *event);
void vita_handle_analog_sticks(void);

#endif /* VITA_INPUT_H */
