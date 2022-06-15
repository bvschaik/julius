#ifndef GRAPHICS_SCREENSHOT_H
#define GRAPHICS_SCREENSHOT_H

enum {
    SCREENSHOT_FULL_CITY = 0,
    SCREENSHOT_DISPLAY = 1,
    SCREENSHOT_MINIMAP = 2,
    SCREENSHOT_MAX = 3
};

void graphics_save_screenshot(int screenshot_type);

#endif // GRAPHICS_SCREENSHOT_H
