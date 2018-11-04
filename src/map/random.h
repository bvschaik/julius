#ifndef MAP_RANDOM_H
#define MAP_RANDOM_H

#include "core/buffer.h"

void map_random_clear(void);

void map_random_init(void);

int map_random_get(int grid_offset);

void map_random_save_state(buffer *buf);

void map_random_load_state(buffer *buf);

#endif // MAP_RANDOM_H
