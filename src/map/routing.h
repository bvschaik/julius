#ifndef MAP_ROUTING_H
#define MAP_ROUTING_H

int map_routing_distance(int grid_offset);

int map_routing_is_wall_passable(int grid_offset);

int map_routing_citizen_is_passable(int grid_offset);
int map_routing_citizen_is_road(int grid_offset);
int map_routing_citizen_is_passable_terrain(int grid_offset);

int map_routing_noncitizen_terrain(int grid_offset);

#endif // MAP_ROUTING_H
