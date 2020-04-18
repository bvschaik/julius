#ifndef WIDGET_SIDEBAR_EDITOR_H
#define WIDGET_SIDEBAR_EDITOR_H

#include "input/mouse.h"

void widget_sidebar_editor_draw_background(void);

void widget_sidebar_editor_draw_foreground(void);

int widget_sidebar_editor_handle_mouse(const mouse *m);
int widget_sidebar_editor_handle_mouse_attributes(const mouse *m);
int widget_sidebar_editor_handle_mouse_build_menu(const mouse *m);

#endif // WIDGET_SIDEBAR_EDITOR_H
