#ifndef FIGURETYPE_WALL_H
#define FIGURETYPE_WALL_H

#include "figure/figure.h"

void figure_ballista_action(figure *f);

void figure_tower_sentry_action(figure *f);

void figure_tower_sentry_reroute();

void figure_kill_tower_sentries_at(int x, int y);

#endif // FIGURETYPE_WALL_H
