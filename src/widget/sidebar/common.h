#ifndef WIDGET_SIDEBAR_COMMON_H
#define WIDGET_SIDEBAR_COMMON_H

#include "graphics/menu.h"

#define SIDEBAR_COLLAPSED_WIDTH 42
#define SIDEBAR_EXPANDED_WIDTH 162
#define SIDEBAR_MAIN_SECTION_HEIGHT 450
#define SIDEBAR_FILLER_Y_OFFSET (SIDEBAR_MAIN_SECTION_HEIGHT + TOP_MENU_HEIGHT)
#define MINIMAP_WIDTH 73
#define MINIMAP_HEIGHT 111

int sidebar_common_get_x_offset_expanded(void);

int sidebar_common_get_x_offset_collapsed(void);

int sidebar_common_get_height(void);

void sidebar_common_draw_relief(int x_offset, int y_offset, int image, int is_collapsed);

#endif // WIDGET_SIDEBAR_COMMON_H
