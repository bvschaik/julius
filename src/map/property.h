#ifndef MAP_PROPERTY_H
#define MAP_PROPERTY_H

int map_property_is_draw_tile(int grid_offset);
void map_property_mark_draw_tile(int grid_offset);
void map_property_clear_draw_tile(int grid_offset);

int map_property_is_native_land(int grid_offset);
void map_property_mark_native_land(int grid_offset);
void map_property_clear_all_native_land();

void map_property_set_multi_tile_xy(int grid_offset, int x, int y, int is_draw_tile);
void map_property_clear_multi_tile_xy(int grid_offset);
int map_property_multi_tile_xy(int grid_offset);
int map_property_multi_tile_x(int grid_offset);
int map_property_multi_tile_y(int grid_offset);
int map_property_is_multi_tile_xy(int grid_offset, int x, int y);

#endif // MAP_PROPERTY_H
