#ifndef FIGURETYPE_WATER_H
#define FIGURETYPE_WATER_H

#include "figure/figure.h"

void figure_create_flotsam();

void figure_flotsam_action(figure *f);

void figure_shipwreck_action(figure *f);

void figure_fishing_boat_action(figure *f);

void figure_sink_all_ships();

#endif // FIGURETYPE_WATER_H
