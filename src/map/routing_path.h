#ifndef MAP_ROUTING_PATH_H
#define MAP_ROUTING_PATH_H

#include <stdint.h>

int map_routing_get_path(uint8_t *path, int src_x, int src_y, int dst_x, int dst_y, int num_directions);

int map_routing_get_path_on_water(uint8_t *path, int dst_x, int dst_y, int is_flotsam);

#endif // MAP_ROUTING_PATH_H
