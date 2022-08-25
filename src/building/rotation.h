#ifndef BUILDING_ROTATION_H
#define BUILDING_ROTATION_H

#include "building/type.h"

int building_rotation_get_road_orientation(void);

void building_rotation_force_two_orientations(void);
int building_rotation_get_building_orientation(int);
int building_rotation_get_delta_with_rotation(int default_delta);
void building_rotation_get_offset_with_rotation(int offset, int rotation, int *x, int *y);
int building_rotation_get_rotation(void);

int building_rotation_get_rotation_with_limit(int limit);

int building_rotation_get_corner(int rot);

void building_rotation_rotate_forward(void);
void building_rotation_rotate_backward(void);
void building_rotation_reset_rotation(void);
void building_rotation_setup_rotation(void);
void building_rotation_remove_rotation(void);

int building_rotation_type_has_rotations(building_type type);

#endif // BUILDING_ROTATION_H