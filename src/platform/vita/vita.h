#ifndef PLATFORM_VITA_H
#define PLATFORM_VITA_H

#ifdef __vita__

#include <stddef.h>

#define VITA_PATH_PREFIX "ux0:/data/julius/"
#define VITA_DISPLAY_WIDTH 960
#define VITA_DISPLAY_HEIGHT 544

enum {
    TOUCH_MODE_TOUCHPAD             = 0, // drag the pointer and tap-click like on a touchpad (default)
    TOUCH_MODE_DIRECT               = 1, // pointer jumps to finger but doesn't click on tap
    TOUCH_MODE_ORIGINAL             = 2, // original julius touch mode as on any other system: pointer jumps to finger and clicks on tap
    NUM_TOUCH_MODES                 = 3
};

extern int last_mouse_x; // defined in vita_input.c
extern int last_mouse_y; // defined in vita_input.c
extern int touch_mode; // defined in vita_input.c

int chdir(const char *path);
char *getcwd(char *buf, size_t size);

char* vita_prepend_path(const char *path);

#endif // __vita__
#endif // PLATFORM_VITA_H