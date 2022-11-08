#ifndef MAP_TILES_H
#define MAP_TILES_H

void map_tiles_update_all_rocks(void);

void map_tiles_update_region_trees(int x_min, int y_min, int x_max, int y_max);
void map_tiles_update_region_shrub(int x_min, int y_min, int x_max, int y_max);

void map_tiles_update_all_gardens(void);
void map_tiles_determine_gardens(void);

void map_tiles_update_all_plazas(void);

void map_tiles_update_all_walls(void);
void map_tiles_update_area_walls(int x, int y, int size);
int map_tiles_set_wall(int x, int y);

int map_tiles_is_paved_road(int grid_offset);
void map_tiles_update_all_roads(void);
void map_tiles_update_area_roads(int x, int y, int size);
int map_tiles_set_road(int x, int y);

int map_tiles_highway_get_aqueduct_image(int grid_offset);
void map_tiles_update_all_highways(void);
void map_tiles_update_area_highways(int x, int y, int size);
int map_tiles_set_highway(int x, int y);
int map_tiles_clear_highway(int grid_offset, int measure_only);

void map_tiles_update_all_empty_land(void);
void map_tiles_update_region_empty_land(int x_min, int y_min, int x_max, int y_max);

void map_tiles_update_all_meadow(void);
void map_tiles_update_region_meadow(int x_min, int y_min, int x_max, int y_max);

void map_tiles_update_all_water(void);
void map_tiles_update_region_water(int x_min, int y_min, int x_max, int y_max);
void map_tiles_set_water(int x, int y);

void map_tiles_update_all_aqueducts(int include_construction);
void map_tiles_update_region_aqueducts(int x_min, int y_min, int x_max, int y_max);

void map_tiles_update_all_earthquake(void);
void map_tiles_set_earthquake(int x, int y);

void map_tiles_update_all_rubble(void);
void map_tiles_update_region_rubble(int x_min, int y_min, int x_max, int y_max);

void map_tiles_update_all_elevation(void);

int map_tiles_are_clear(int x, int y, int size, int disallowed_terrain);

void map_tiles_add_entry_exit_flags(void);
void map_tiles_remove_entry_exit_flags(void);

void map_tiles_update_all(void);

#endif // MAP_TILES_H
