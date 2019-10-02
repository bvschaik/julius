#ifndef MAP_PROPERTY_H
#define MAP_PROPERTY_H

#include "core/buffer.h"

enum {
    EDGE_X0Y0 = 0,
    EDGE_X1Y0 = 1,
    EDGE_X2Y0 = 2,
    EDGE_X0Y1 = 8,
    EDGE_X1Y1 = 9,
    EDGE_X2Y1 = 10,
    EDGE_X0Y2 = 16,
    EDGE_X1Y2 = 17,
    EDGE_X2Y2 = 18
};

int map_property_is_draw_tile(int grid_offset);
void map_property_mark_draw_tile(int grid_offset);
void map_property_clear_draw_tile(int grid_offset);

int map_property_is_native_land(int grid_offset);
void map_property_mark_native_land(int grid_offset);
void map_property_clear_all_native_land(void);

int map_property_multi_tile_xy(int grid_offset);
int map_property_multi_tile_x(int grid_offset);
int map_property_multi_tile_y(int grid_offset);
int map_property_is_multi_tile_xy(int grid_offset, int x, int y);
void map_property_set_multi_tile_xy(int grid_offset, int x, int y, int is_draw_tile);
void map_property_clear_multi_tile_xy(int grid_offset);

int map_property_multi_tile_size(int grid_offset);
void map_property_set_multi_tile_size(int grid_offset, int size);

void map_property_init_alternate_terrain(void);
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

void map_property_clear_constructing_and_deleted(void);

void map_property_clear(void);

void map_property_backup(void);
void map_property_restore(void);

void map_property_save_state(buffer *bitfields, buffer *edge);
void map_property_load_state(buffer *bitfields, buffer *edge);

#endif // MAP_PROPERTY_H
