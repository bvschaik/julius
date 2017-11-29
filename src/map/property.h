#ifndef MAP_PROPERTY_H
#define MAP_PROPERTY_H

int map_property_is_draw_tile(int grid_offset);
void map_property_mark_draw_tile(int grid_offset);
void map_property_clear_draw_tile(int grid_offset);

int map_property_is_native_land(int grid_offset);
void map_property_mark_native_land(int grid_offset);
void map_property_clear_all_native_land();

int map_property_multi_tile_xy(int grid_offset);
int map_property_multi_tile_x(int grid_offset);
int map_property_multi_tile_y(int grid_offset);
int map_property_is_multi_tile_xy(int grid_offset, int x, int y);
void map_property_set_multi_tile_xy(int grid_offset, int x, int y, int is_draw_tile);
void map_property_clear_multi_tile_xy(int grid_offset);

int map_property_multi_tile_size(int grid_offset);
void map_property_set_multi_tile_size(int grid_offset, int size);

int map_property_is_alternate_terrain(int grid_offset);
void map_property_set_alternate_terrain(int grid_offset);

int map_property_is_plaza_or_earthquake(int grid_offset);
void map_property_mark_plaza_or_earthquake(int grid_offset);
void map_property_clear_plaza_or_earthquake(int grid_offset);

int map_property_is_constructing(int grid_offset);
void map_property_mark_constructing(int grid_offset);
void map_property_clear_constructing(int grid_offset);

int map_property_is_deleted(int grid_offset);
void map_property_mark_deleted(int grid_offset);
void map_property_clear_deleted(int grid_offset);

#endif // MAP_PROPERTY_H
