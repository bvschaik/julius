#ifndef FIGURETYPE_MISSILE_H
#define FIGURETYPE_MISSILE_H

#include "figure/figure.h"

void figure_create_explosion_cloud(int x, int y, int size);

void figure_create_missile(int building_id, int x, int y, int x_dst, int y_dst, figure_type type);

void figure_explosion_cloud_action(figure *f);

void figure_arrow_action(figure *f);

void figure_friendly_arrow_action(figure* f);

void figure_spear_action(figure *f);

void figure_javelin_action(figure *f);

void figure_bolt_action(figure *f);

#endif // FIGURETYPE_MISSILE_H
