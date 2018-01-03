#ifndef MAP_TILES_H
#define MAP_TILES_H

void map_tiles_update_all_rocks();

void map_tiles_update_all_gardens();
void map_tiles_determine_gardens();

void map_tiles_update_all_plazas();

void map_tiles_update_all_walls();
void map_tiles_update_area_walls(int x, int y, int size);
int map_tiles_set_wall(int x, int y);

int map_tiles_is_paved_road(int grid_offset);
void map_tiles_update_all_roads();
void map_tiles_update_area_roads(int x, int y, int size);
int map_tiles_set_road(int x, int y);

void map_tiles_update_all_empty_land();
void map_tiles_update_region_empty_land(int x_min, int y_min, int x_max, int y_max);

void map_tiles_update_all_meadow();
void map_tiles_update_region_meadow(int x_min, int y_min, int x_max, int y_max);

void map_tiles_update_all_water();
void map_tiles_set_water(int x, int y);

void map_tiles_update_all_aqueducts(int include_construction);
void map_tiles_update_region_aqueducts(int x_min, int y_min, int x_max, int y_max);

void map_tiles_update_all_earthquake();
void map_tiles_set_earthquake(int x, int y);

void map_tiles_update_all_rubble();
void map_tiles_update_region_rubble(int x_min, int y_min, int x_max, int y_max);

void map_tiles_update_all_elevation();

#endif // MAP_TILES_H
