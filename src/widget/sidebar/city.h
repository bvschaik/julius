#ifndef WIDGET_SIDEBAR_CITY_H
#define WIDGET_SIDEBAR_CITY_H

#include "input/mouse.h"

void widget_sidebar_city_draw_background(void);

void widget_sidebar_city_draw_foreground(void);
void widget_sidebar_city_draw_foreground_military(void);

int widget_sidebar_city_handle_mouse(const mouse *m);
int widget_sidebar_city_handle_mouse_build_menu(const mouse *m);

int widget_sidebar_city_get_tooltip_text(void);

#endif // WIDGET_SIDEBAR_CITY_H
