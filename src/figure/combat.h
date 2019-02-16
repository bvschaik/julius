#ifndef FIGURE_COMBAT_H
#define FIGURE_COMBAT_H

#include "figure/figure.h"
#include "map/point.h"

void figure_combat_handle_corpse(figure *f);
void figure_combat_handle_attack(figure *f);

int figure_combat_get_target_for_soldier(int x, int y, int max_distance);
int figure_combat_get_target_for_wolf(int x, int y, int max_distance);
int figure_combat_get_target_for_enemy(int x, int y);

int figure_combat_get_missile_target_for_soldier(figure *shooter, int max_distance, map_point *tile);
int figure_combat_get_missile_target_for_enemy(figure *enemy, int max_distance, int attack_citizens, map_point *tile);

void figure_combat_attack_figure_at(figure *f, int grid_offset);

#endif // FIGURE_COMBAT_H
