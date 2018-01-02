#ifndef MAP_TILES_H
#define MAP_TILES_H

void map_tiles_update_all_rocks();
void map_tiles_update_all_gardens();
void map_tiles_update_all_plazas();

void map_tiles_update_all_walls();
void map_tiles_update_area_walls(int x, int y, int size);
int map_tiles_set_wall(int x, int y);

int map_tiles_is_paved_road(int grid_offset);
void map_tiles_update_all_roads();
void map_tiles_update_area_roads(int x, int y, int size);
int map_tiles_set_road(int x, int y);

#endif // MAP_TILES_H
