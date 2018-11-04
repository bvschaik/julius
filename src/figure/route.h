#ifndef FIGURE_ROUTE_H
#define FIGURE_ROUTE_H

#include "core/buffer.h"
#include "figure/figure.h"

void figure_route_clear_all(void);

void figure_route_clean(void);

void figure_route_add(figure *f);

void figure_route_remove(figure *f);

int figure_route_get_direction(int path_id, int index);

void figure_route_save_state(buffer *figures, buffer *paths);

void figure_route_load_state(buffer *figures, buffer *paths);

#endif // FIGURE_ROUTE_H
