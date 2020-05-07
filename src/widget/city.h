#ifndef WIDGET_CITY_H
#define WIDGET_CITY_H

#include "graphics/tooltip.h"
#include "input/hotkey.h"
#include "input/mouse.h"

typedef struct {
    int x;
    int y;
} pixel_coordinate;

void widget_city_draw(void);
void widget_city_draw_for_figure(int figure_id, pixel_coordinate *coord);

int widget_city_draw_construction_cost_and_size(void);

int widget_city_has_input(void);
void widget_city_handle_input(const mouse *m, const hotkeys *h);
void widget_city_handle_input_military(const mouse *m, const hotkeys *h, int legion_formation_id);

void widget_city_get_tooltip(tooltip_context *c);

void widget_city_clear_current_tile(void);

#endif // WIDGET_CITY_H
