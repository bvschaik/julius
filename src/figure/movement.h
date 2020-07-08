#ifndef FIGURE_MOVEMENT_H
#define FIGURE_MOVEMENT_H

#include "figure/figure.h"

void figure_movement_init_roaming(figure *f);

void figure_movement_move_ticks(figure *f, int num_ticks);

void figure_movement_move_ticks_tower_sentry(figure *f, int num_ticks);

void figure_movement_roam_ticks(figure *f, int num_ticks);

void figure_movement_follow_ticks(figure *f, int num_ticks);

void figure_movement_advance_attack(figure *f);

void figure_movement_set_cross_country_direction(figure *f, int x_src, int y_src, int x_dst, int y_dst, int is_missile);

void figure_movement_set_cross_country_destination(figure *f, int x_dst, int y_dst);

int figure_movement_move_ticks_cross_country(figure *f, int num_ticks);

int figure_movement_can_launch_cross_country_missile(int x_src, int y_src, int x_dst, int y_dst);

#endif // FIGURE_MOVEMENT_H
