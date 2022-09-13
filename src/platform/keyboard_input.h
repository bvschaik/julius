#ifndef PLATFORM_KEYBOARD_INPUT_H
#define PLATFORM_KEYBOARD_INPUT_H

#include "SDL.h"

void platform_handle_key_down(SDL_KeyboardEvent *event);

void platform_handle_key_up(SDL_KeyboardEvent *event);

void platform_handle_editing_text(SDL_TextEditingEvent *event);

void platform_handle_text(SDL_TextInputEvent *event);

#endif // PLATFORM_KEYBOARD_INPUT_H
