#ifndef WIDGET_MINIMAP_H
#define WIDGET_MINIMAP_H

#include "input/mouse.h"

void widget_minimap_invalidate(void);

void widget_minimap_draw(int x_offset, int y_offset, int width_tiles, int height_tiles, int force);

int widget_minimap_handle_mouse(const mouse *m);

#endif // WIDGET_MINIMAP_H
