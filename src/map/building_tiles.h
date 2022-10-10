#ifndef MAP_BUILDING_TILES_H
#define MAP_BUILDING_TILES_H

void map_building_tiles_add_remove(int building_id, int x, int y, int size, int image_id, int terrain_to_add, int terrain_to_remove);

void map_building_tiles_add(int building_id, int x, int y, int size, int image_id, int terrain);

void map_building_tiles_add_farm(int building_id, int x, int y, int crop_image_id, int progress);

int map_building_tiles_add_aqueduct(int x, int y);

void map_building_tiles_remove(int building_id, int x, int y);

void map_building_tiles_set_rubble(int building_id, int x, int y, int size);

void map_building_tiles_mark_deleting(int grid_offset);

int map_building_tiles_mark_construction(int x, int y, int size, int terrain, int absolute_xy);

int map_building_tiles_are_clear(int x, int y, int size, int terrain);

#endif // MAP_BUILDING_TILES_H
