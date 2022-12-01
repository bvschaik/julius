#ifndef FIGURE_FORMATION_LAYOUT_H
#define FIGURE_FORMATION_LAYOUT_H

#include "figure/formation.h"

#include <stdint.h>

int formation_layout_position_x(int layout, int index);

int formation_layout_position_y(int layout, int index);

void formation_layout_position(int layout, int index, formation *m, uint8_t *x, uint8_t *y);

#endif // FIGURE_FORMATION_LAYOUT_H
