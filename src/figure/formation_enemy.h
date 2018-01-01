#ifndef FIGURE_FORMATION_ENEMY_H
#define FIGURE_FORMATION_ENEMY_H

#include "figure/formation.h"

int formation_rioter_get_target_building(int *x_tile, int *y_tile);

int formation_enemy_move_formation_to(const formation *m, int x, int y, int *x_tile, int *y_tile);

void formation_enemy_update();

#endif // FIGURE_FORMATION_ENEMY_H
