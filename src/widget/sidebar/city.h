#ifndef WIDGET_SIDEBAR_CITY_H
#define WIDGET_SIDEBAR_CITY_H

#include "graphics/tooltip.h"
#include "input/mouse.h"

void widget_sidebar_city_draw_background(void);
void widget_sidebar_city_draw_foreground(void);

int widget_sidebar_city_handle_mouse(const mouse *m);
int widget_sidebar_city_handle_mouse_build_menu(const mouse *m);

int widget_sidebar_city_get_tooltip_text(tooltip_context *c);

#endif // WIDGET_SIDEBAR_CITY_H
