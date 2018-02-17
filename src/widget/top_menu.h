#ifndef WINDOW_TOP_MENU_H
#define WINDOW_TOP_MENU_H

#include "input/mouse.h"
#include "graphics/tooltip.h"

void widget_top_menu_draw(int force);
int widget_top_menu_handle_mouse(const mouse *m);
int widget_top_menu_get_tooltip_text(tooltip_context *c);

#endif // WINDOW_TOP_MENU_H
