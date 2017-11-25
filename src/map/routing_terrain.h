#ifndef MAP_ROUTING_TERRAIN_H
#define MAP_ROUTING_TERRAIN_H

void map_routing_update_all();
void map_routing_update_land();
void map_routing_update_land_citizen();
void map_routing_update_water();
void map_routing_update_walls();

void map_routing_block(int x, int y, int size);

int map_routing_is_wall_passable(int grid_offset);

int map_routing_citizen_is_passable(int grid_offset);
int map_routing_citizen_is_road(int grid_offset);
int map_routing_citizen_is_passable_terrain(int grid_offset);

int map_routing_noncitizen_is_passable(int grid_offset);
int map_routing_is_destroyable(int grid_offset);

enum {
    DESTROYABLE_BUILDING,
    DESTROYABLE_AQUEDUCT_GARDEN,
    DESTROYABLE_WALL,
    DESTROYABLE_GATEHOUSE,
    DESTROYABLE_NONE
};
int map_routing_get_destroyable(int grid_offset);

#endif // MAP_ROUTING_TERRAIN_H
