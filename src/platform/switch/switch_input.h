#ifndef SWITCH_INPUT_H
#define SWITCH_INPUT_H

#include <SDL2/SDL.h>

int switch_poll_event(SDL_Event *event);
void switch_handle_analog_sticks(void);

#endif /* SWITCH_INPUT_H */
