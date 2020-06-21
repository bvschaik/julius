#ifndef BUILDING_ROTATION_H
#define BUILDING_ROTATION_H

void force_two_orientations(void);
int get_building_orientation(int);
int get_delta_with_rotation(int default_delta);
void get_offset_with_rotation(int offset, int rotation, int * x, int * y);
int get_rotation(void);

int get_corner(int rot);

void rotate(void);
void reset_rotation(void);

#endif // BUILDING_ROTATION_H