#ifndef MAP_ROUTING_H
#define MAP_ROUTING_H

#include "core/buffer.h"

int map_routing_distance(int grid_offset);

int map_routing_is_wall_passable(int grid_offset);

int map_routing_citizen_is_passable(int grid_offset);
int map_routing_citizen_is_road(int grid_offset);
int map_routing_citizen_is_passable_terrain(int grid_offset);

int map_routing_noncitizen_terrain(int grid_offset);

void map_routing_save_state(buffer *buf);

void map_routing_load_state(buffer *buf);

#endif // MAP_ROUTING_H
