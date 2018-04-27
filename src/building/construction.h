#ifndef BUILDING_CONSTRUCTION_H
#define BUILDING_CONSTRUCTION_H

#include "building/type.h"

void building_construction_set_type(building_type type);

void building_construction_clear_type();

building_type building_construction_type();

int building_construction_cost();

int building_construction_in_progress();

void building_construction_start(int x, int y, int grid_offset);

void building_construction_update(int x, int y, int grid_offset);

void building_construction_place();

int building_construction_can_place_on_terrain(int x, int y, int *warning_id);

void building_construction_update_road_orientation();
int building_construction_road_orientation();

void building_construction_record_view_position(int view_x, int view_y, int grid_offset);
void building_construction_get_view_position(int *view_x, int *view_y);

void building_construction_reset_draw_as_constructing();
int building_construction_draw_as_constructing();

#endif // BUILDING_CONSTRUCTION_H
