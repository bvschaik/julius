#ifndef WIDGET_SIDEBAR_FILLER_H
#define WIDGET_SIDEBAR_FILLER_H

#include "graphics/arrow_button.h"

typedef struct {
    int value;
    int target;
} objective;

typedef struct {
    int height;
    int game_speed;
    int unemployment_percentage;
    int unemployment_amount;
    objective culture;
    objective prosperity;
    objective peace;
    objective favor;
    objective population;
} extra_info;

void sidebar_filler_draw_extra_info_panel(int x_offset, int extra_info_height);
void sidebar_filler_draw_extra_info_buttons(int x_offset, int is_collapsed);
int sidebar_filler_calculate_extra_info_height(int is_collapsed);
int sidebar_filler_update_extra_info(int height, int is_background);
int sidebar_filler_extra_info_height_game_speed_check(int height);
arrow_button *sidebar_filler_get_arrow_buttons_speed(void);

#endif // WIDGET_SIDEBAR_FILLER_H
