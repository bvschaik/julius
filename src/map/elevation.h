#ifndef MAP_ELEVATION_H
#define MAP_ELEVATION_H

#include "core/buffer.h"

int map_elevation_at(int grid_offset);

void map_elevation_clear(void);

void map_elevation_save_state(buffer *buf);

void map_elevation_load_state(buffer *buf);

#endif // MAP_ELEVATION_H
