#ifndef MAP_ROAD_AQUEDUCT_H
#define MAP_ROAD_AQUEDUCT_H

int map_can_place_road_under_aqueduct(int grid_offset);

int map_can_place_aqueduct_on_road(int grid_offset);

int map_get_aqueduct_with_road_image(int grid_offset);

int map_is_straight_road_for_aqueduct(int grid_offset);

#endif // MAP_ROAD_AQUEDUCT_H
