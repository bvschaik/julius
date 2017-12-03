#ifndef MAP_ROUTING_H
#define MAP_ROUTING_H

#include "core/buffer.h"

typedef enum {
    ROUTED_BUILDING_ROAD = 0,
    ROUTED_BUILDING_WALL = 1,
    ROUTED_BUILDING_AQUEDUCT = 2,
    ROUTED_BUILDING_AQUEDUCT_WITHOUT_GRAPHIC = 4,
} routed_building_type;

void map_routing_calculate_distances(int x, int y);
void map_routing_calculate_distances_water_boat(int x, int y);
void map_routing_calculate_distances_water_flotsam(int x, int y);

int map_routing_calculate_distances_for_building(routed_building_type type, int x, int y);

void map_routing_delete_first_wall_or_aqueduct(int x, int y);

int map_routing_distance(int grid_offset);

int map_routing_is_wall_passable(int grid_offset);

int map_routing_citizen_is_passable(int grid_offset);
int map_routing_citizen_is_road(int grid_offset);
int map_routing_citizen_is_passable_terrain(int grid_offset);

int map_routing_noncitizen_terrain(int grid_offset);

int map_routing_citizen_can_travel_over_land(int src_x, int src_y, int dst_x, int dst_y);
int map_routing_citizen_can_travel_over_road_garden(int src_x, int src_y, int dst_x, int dst_y);
int map_routing_can_travel_over_walls(int src_x, int src_y, int dst_x, int dst_y);

int map_routing_noncitizen_can_travel_over_land(int src_x, int src_y, int dst_x, int dst_y, int onlyThroughBuildingId, int maxTiles);
int map_routing_noncitizen_can_travel_through_everything(int src_x, int src_y, int dst_x, int dst_y);

void map_routing_save_state(buffer *buf);

void map_routing_load_state(buffer *buf);

#endif // MAP_ROUTING_H
