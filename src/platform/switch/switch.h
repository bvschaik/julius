#ifndef PLATFORM_SWITCH_H
#define PLATFORM_SWITCH_H

#include "SDL.h"

#define SWITCH_DISPLAY_WIDTH 960
#define SWITCH_DISPLAY_HEIGHT 540
#define SWITCH_PIXEL_WIDTH 1920
#define SWITCH_PIXEL_HEIGHT 1080

typedef struct {
    SDL_Texture *texture;
    int hotspot_x;
    int hotspot_y;
} switch_cursor;

extern switch_cursor *current_cursor; // defined in cursor.c

#endif // PLATFORM_SWITCH_H