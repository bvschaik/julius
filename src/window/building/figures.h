#ifndef WINDOW_BUILDING_FIGURES_H
#define WINDOW_BUILDING_FIGURES_H

#include "common.h"
#include "input/mouse.h"

void window_building_prepare_figure_list(BuildingInfoContext *c);

void window_building_draw_figure_list(BuildingInfoContext *c);

void window_building_handle_mouse_figure_list(const mouse *m, BuildingInfoContext *c);

void window_building_play_figure_phrase(BuildingInfoContext *c);

#endif // WINDOW_BUILDING_FIGURES_H
