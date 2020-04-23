#ifndef WIDGET_EXTRA_INFO_H
#define WIDGET_EXTRA_INFO_H

  //#include "sidebar.h"

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

void draw_extra_info_panel(int x_offset, int extra_info_height);
void draw_extra_info_buttons(int x_offset, int is_collapsed);
int calculate_extra_info_height(int is_collapsed);
int update_extra_info(int height, int is_background);
int extra_info_height_game_speed_check(int height);
#endif // WIDGET_EXTRA_INFO_H
