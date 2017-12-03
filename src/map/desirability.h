#ifndef MAP_DESIRABILITY_H
#define MAP_DESIRABILITY_H

#include "core/buffer.h"

void map_desirability_clear();

void map_desirability_update();

int map_desirability_get(int grid_offset);

int map_desirability_get_max(int x, int y, int size);

void map_desirability_save_state(buffer *buf);

void map_desirability_load_state(buffer *buf);

#endif // MAP_DESIRABILITY_H
