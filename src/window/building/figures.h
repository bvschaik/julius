#ifndef WINDOW_BUILDING_FIGURES_H
#define WINDOW_BUILDING_FIGURES_H

#include "common.h"
#include "input/mouse.h"

void window_building_prepare_figure_list(building_info_context *c);

void window_building_draw_figure_list(building_info_context *c);

void window_building_handle_mouse_figure_list(const mouse *m, building_info_context *c);

void window_building_play_figure_phrase(building_info_context *c);

#endif // WINDOW_BUILDING_FIGURES_H
