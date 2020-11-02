#ifndef WIDGET_SIDEBAR_MILITARY_H
#define WIDGET_SIDEBAR_MILITARY_H

#include "graphics/tooltip.h"
#include "input/mouse.h"

int widget_sidebar_military_enter(int formation_id);
int widget_sidebar_military_exit(void);

void widget_sidebar_military_draw_background(void);
void widget_sidebar_military_draw_foreground(void);

int widget_sidebar_military_handle_input(const mouse *m);

int widget_sidebar_military_get_tooltip_text(tooltip_context *c);

#endif // WIDGET_SIDEBAR_MILITARY_H
