#ifndef WIDGET_CITY_FIGURE_H
#define WIDGET_CITY_FIGURE_H

#include "figure/figure.h"
#include "widget/city.h"

void city_draw_figure(const figure *f, int x, int y, float scale, int highlight);

void city_draw_selected_figure(const figure *f, int x, int y, float scale, pixel_coordinate *coord);

#endif // WIDGET_CITY_FIGURE_H
