#ifndef PLATFORM_SWITCH_H
#define PLATFORM_SWITCH_H

#define SWITCH_DISPLAY_WIDTH 960
#define SWITCH_DISPLAY_HEIGHT 540
#define SWITCH_PIXEL_WIDTH 1920
#define SWITCH_PIXEL_HEIGHT 1080

enum {
    TOUCH_MODE_TOUCHPAD             = 0, // drag the pointer and tap-click like on a touchpad (default)
    TOUCH_MODE_DIRECT               = 1, // pointer jumps to finger but doesn't click on tap
    TOUCH_MODE_ORIGINAL             = 2, // original julius touch mode as on any other system: pointer jumps to finger and clicks on tap
    NUM_TOUCH_MODES                 = 3
};

extern int last_mouse_x; // defined in switch_input.c
extern int last_mouse_y; // defined in switch_input.c
extern int touch_mode; // defined in switch_input.c

#endif // PLATFORM_SWITCH_H