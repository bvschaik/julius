#ifndef WIDGET_CITY_FIGURE_H
#define WIDGET_CITY_FIGURE_H

#include "figure/figure.h"

#include "UI/CityBuildings.h"

void city_draw_figure(const figure *f, int x, int y);

void city_draw_selected_figure(const figure *f, int x, int y, struct UI_CityPixelCoordinate *coord);

#endif // WIDGET_CITY_FIGURE_H
