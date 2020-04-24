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

// used in extra_info.c as well as sidebar.c
#define SIDEBAR_COLLAPSED_WIDTH 42
#define SIDEBAR_EXPANDED_WIDTH 162
#define FILLER_Y_OFFSET 474

#endif // WIDGET_SIDEBAR_H
