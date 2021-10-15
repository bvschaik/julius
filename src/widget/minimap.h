#ifndef WIDGET_MINIMAP_H
#define WIDGET_MINIMAP_H

#include "graphics/color.h"
#include "input/mouse.h"

void widget_minimap_invalidate(void);

void widget_minimap_draw(int x_offset, int y_offset, int width, int height, int force);

void widget_minimap_draw_from_buffer(int x, int y, int width, int height, const color_t *buffer);

int widget_minimap_handle_mouse(const mouse *m);

#endif // WIDGET_MINIMAP_H
