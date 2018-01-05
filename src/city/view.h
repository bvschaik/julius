#ifndef CITY_VIEW_H
#define CITY_VIEW_H

#include "core/buffer.h"

void city_view_init();

int city_view_orientation();

void city_view_reset_orientation();

void city_view_get_camera(int *x, int *y);

void city_view_set_camera(int x, int y);

int city_view_scroll(int direction);

int city_view_to_grid_offset(int x_view, int y_view);

void city_view_grid_offset_to_xy_view(int grid_offset, int *x_view, int *y_view);

int city_view_pixels_to_grid_offset(int x_pixels, int y_pixels);

void city_view_go_to_grid_offset(int grid_offset);

void city_view_rotate_left();

void city_view_rotate_right();

void city_view_set_viewport(int screen_width, int screen_height);

int city_view_is_sidebar_collapsed();

void city_view_start_sidebar_toggle();

void city_view_toggle_sidebar();

void city_view_save_state(buffer *orientation, buffer *camera);

void city_view_load_state(buffer *orientation, buffer *camera);

void city_view_load_scenario_state(buffer *camera);

#endif // CITY_VIEW_H
