#ifndef WINDOW_TOP_MENU_H
#define WINDOW_TOP_MENU_H

#include "graphics/tooltip.h"
#include "input/hotkey.h"
#include "input/mouse.h"

void widget_top_menu_draw(int force);
int widget_top_menu_handle_input(const mouse *m, const hotkeys *h);
int widget_top_menu_get_tooltip_text(tooltip_context *c);

#endif // WINDOW_TOP_MENU_H
