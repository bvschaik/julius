#ifndef WIDGET_SIDEBAR_H
#define WIDGET_SIDEBAR_H

#include "input/mouse.h"
#include "widget/sidebar_filler.h"

void widget_sidebar_draw_background(void);

void widget_sidebar_draw_foreground(void);
void widget_sidebar_draw_foreground_military(void);

int widget_sidebar_handle_mouse(const mouse *m);
int widget_sidebar_handle_mouse_build_menu(const mouse *m);

int widget_sidebar_get_tooltip_text(void);
int widget_sidebar_get_expanded_width(void);

#endif // WIDGET_SIDEBAR_H
