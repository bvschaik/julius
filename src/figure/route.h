#ifndef FIGURE_ROUTE_H
#define FIGURE_ROUTE_H

#include "core/buffer.h"

void figure_route_clear_all();

void figure_route_clean();

void figure_route_add(int figure_id);

void figure_route_remove(int figure_id);

int figure_route_get_direction(int path_id, int tile);

void figure_route_save_state(buffer *figures, buffer *paths);

void figure_route_load_state(buffer *figures, buffer *paths);

#endif // FIGURE_ROUTE_H
