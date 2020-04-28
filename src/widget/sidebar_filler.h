#ifndef WIDGET_SIDEBAR_FILLER_H
#define WIDGET_SIDEBAR_FILLER_H

#include "input/mouse.h"

void sidebar_filler_draw_foreground(int x_offset, int y_offset, int width, int is_collapsed);
void sidebar_filler_draw_background(int x_offset, int y_offset, int width, int height, int is_collapsed);

int sidebar_filler_get_height(void);
int sidebar_filler_handle_mouse(const mouse *m, int x_offset, int y_offset);

#endif // WIDGET_SIDEBAR_FILLER_H
