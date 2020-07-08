#ifndef PLATFORM_SWITCH_H
#define PLATFORM_SWITCH_H

#ifdef __SWITCH__

#include "SDL.h"

#define SWITCH_DISPLAY_WIDTH 960
#define SWITCH_DISPLAY_HEIGHT 540
#define SWITCH_PIXEL_WIDTH 1920
#define SWITCH_PIXEL_HEIGHT 1080

#define PLATFORM_ENABLE_INIT_CALLBACK
void platform_init_callback(void);

#define PLATFORM_ENABLE_PER_FRAME_CALLBACK
void platform_per_frame_callback(void);

#define PLATFORM_USE_VIRTUAL_KEYBOARD
void platform_show_virtual_keyboard(const uint8_t *text, int max_length);
void platform_hide_virtual_keyboard(void);

typedef struct {
    SDL_Texture *texture;
    int hotspot_x;
    int hotspot_y;
} switch_cursor;

extern switch_cursor *current_cursor; // defined in cursor.c

#endif // __SWITCH__
#endif // PLATFORM_SWITCH_H
