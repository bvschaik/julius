#ifndef WIDGET_CITY_WITHOUT_OVERLAY_H
#define WIDGET_CITY_WITHOUT_OVERLAY_H

#include "widget/city.h"

void city_without_overlay_draw(int selected_figure_id, pixel_coordinate *figure_coord);

void city_draw_bridge(int x, int y, int grid_offset);

#endif // WIDGET_CITY_WITHOUT_OVERLAY_H
