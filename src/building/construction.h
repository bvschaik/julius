#ifndef BUILDING_CONSTRUCTION_H
#define BUILDING_CONSTRUCTION_H

#include "building/type.h"

int building_construction_cycle_forward(void);

int building_construction_cycle_back(void);

int building_construction_is_connecting(void);

void building_construction_set_cost(int cost);

void building_construction_set_type(building_type type);

void building_construction_clear_type(void);

int building_construction_can_rotate(void);

building_type building_construction_type(void);

int building_construction_cost(void);

int building_construction_size(int *x, int *y);

int building_construction_in_progress(void);

void building_construction_start(int x, int y, int grid_offset);

int building_construction_is_updatable(void);

void building_construction_cancel(void);

void building_construction_update(int x, int y, int grid_offset);

void building_construction_place(void);

int building_construction_can_place_on_terrain(int x, int y, int *warning_id);

void building_construction_record_view_position(int view_x, int view_y, int grid_offset);
void building_construction_get_view_position(int *view_x, int *view_y);
int building_construction_get_start_grid_offset(void);

void building_construction_reset_draw_as_constructing(void);
int building_construction_draw_as_constructing(void);

#endif // BUILDING_CONSTRUCTION_H
