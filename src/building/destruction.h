#ifndef BUILDING_DESTRUCTION_H
#define BUILDING_DESTRUCTION_H

#include "building/building.h"

void building_destroy_by_collapse(building *b);

void building_destroy_by_poor_maintenance(building *b);

void building_destroy_by_fire(building *b);

void building_destroy_by_plague(building *b);

void building_destroy_by_rioter(building *b);

int building_destroy_first_of_type(building_type type);

void building_destroy_last_placed(void);

void building_destroy_increase_enemy_damage(int grid_offset, int max_damage);

void building_destroy_by_enemy(int x, int y, int grid_offset);

#endif // BUILDING_DESTRUCTION_H
