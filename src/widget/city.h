#ifndef WIDGET_CITY_H
#define WIDGET_CITY_H

#include "input/mouse.h"
#include "graphics/tooltip.h"

typedef struct {
    int x;
    int y;
} pixel_coordinate;

void widget_city_draw(void);
void widget_city_draw_for_figure(int figure_id, pixel_coordinate *coord);

void widget_city_draw_construction_cost_and_size(void);

int widget_city_has_input(void);
void widget_city_handle_mouse(const mouse *m);
void widget_city_handle_mouse_military(const mouse *m, int legion_formation_id);

void widget_city_get_tooltip(tooltip_context *c);

void widget_city_clear_current_tile(void);

#endif // WIDGET_CITY_H
