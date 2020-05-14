#ifndef WIDGET_SIDEBAR_FILLER_H
#define WIDGET_SIDEBAR_FILLER_H

#include "input/mouse.h"

typedef enum {
    SIDEBAR_EXTRA_DISPLAY_NONE = 0,
    SIDEBAR_EXTRA_DISPLAY_GAME_SPEED = 1,
    SIDEBAR_EXTRA_DISPLAY_UNEMPLOYMENT = 2,
    SIDEBAR_EXTRA_DISPLAY_RATINGS = 4,
    SIDEBAR_EXTRA_DISPLAY_ALL = 7
} sidebar_extra_display;

/**
 * @return The actual height of the extra info
 */
int sidebar_extra_draw_background(int x_offset, int y_offset, int width, int height, int is_collapsed, sidebar_extra_display info_to_display);
void sidebar_extra_draw_foreground(void);

int sidebar_extra_handle_mouse(const mouse *m);

#endif // WIDGET_SIDEBAR_FILLER_H
