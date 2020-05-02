#ifndef WIDGET_SIDEBAR_FILLER_H
#define WIDGET_SIDEBAR_FILLER_H

#include "input/mouse.h"

/**
 * @return The actual height of the extra info
 */
int sidebar_extra_draw_background(int x_offset, int y_offset, int width, int height, int is_collapsed);
void sidebar_extra_draw_foreground(int x_offset, int y_offset, int width, int is_collapsed);

int sidebar_extra_handle_mouse(const mouse *m, int x_offset, int y_offset);

#endif // WIDGET_SIDEBAR_FILLER_H
