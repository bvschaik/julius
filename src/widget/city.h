#ifndef WIDGET_CITY_H
#define WIDGET_CITY_H

#include "input/mouse.h"
#include "graphics/tooltip.h"

typedef struct {
    int x;
    int y;
} pixel_coordinate;

void widget_city_draw(int x, int y);
void widget_city_draw_for_figure(int x, int y, int figure_id, pixel_coordinate *coord);

void widget_city_draw_construction_cost();

void widget_city_handle_mouse(const mouse *m);
void widget_city_handle_mouse_military(const mouse *m);

void widget_city_get_tooltip(tooltip_context *c);

#endif // WIDGET_CITY_H
