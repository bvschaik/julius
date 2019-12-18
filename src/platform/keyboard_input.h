#ifndef PLATFORM_KEYBOARD_INPUT_H
#define PLATFORM_KEYBOARD_INPUT_H

#include "SDL.h"

void platform_handle_key_down(SDL_KeyboardEvent *event);

void platform_handle_key_up(SDL_KeyboardEvent *event);

void platform_handle_text(SDL_TextInputEvent *event);

void platform_start_text_input(void);

void platform_stop_text_input(void);

#endif // PLATFORM_KEYBOARD_INPUT_H
