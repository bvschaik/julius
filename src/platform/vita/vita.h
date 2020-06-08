#ifndef PLATFORM_VITA_H
#define PLATFORM_VITA_H

#ifdef __vita__

#include <vita2d.h>

#define VITA_PATH_PREFIX "ux0:/data/julius/"
#define VITA_DISPLAY_WIDTH 960
#define VITA_DISPLAY_HEIGHT 544

typedef struct {
    vita2d_texture *texture;
    int hotspot_x;
    int hotspot_y;
} vita_cursor;

extern vita_cursor *current_cursor; // defined in cursor.c

int chdir(const char *path);

char* vita_prepend_path(const char *path);

#endif // __vita__
#endif // PLATFORM_VITA_H