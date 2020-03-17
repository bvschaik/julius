#ifndef WIDGET_MAP_EDITOR_H
#define WIDGET_MAP_EDITOR_H

#include "input/mouse.h"

void widget_map_editor_draw(void);

void widget_map_editor_handle_mouse(const mouse *m);

void widget_map_editor_clear_current_tile(void);

#endif // WIDGET_MAP_EDITOR_H
