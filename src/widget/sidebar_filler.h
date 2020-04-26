#ifndef WIDGET_SIDEBAR_FILLER_H
#define WIDGET_SIDEBAR_FILLER_H

#include "input/mouse.h"

void sidebar_filler_draw_extra_info_panel(int x_offset, int extra_info_height);
void sidebar_filler_draw_extra_info_buttons(int x_offset, int is_collapsed);
void sidebar_filler_draw_sidebar_filler(int x_offset, int y_offset, int is_collapsed);

int sidebar_filler_calculate_extra_info_height(int is_collapsed);
int sidebar_filler_update_extra_info(int height, int is_background);
int sidebar_filler_enabled(void);
int sidebar_filler_handle_mouse(const mouse *m, int x_offset, int y_offset);

#endif // WIDGET_SIDEBAR_FILLER_H
