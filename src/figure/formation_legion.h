#ifndef FIGURE_FORMATION_LEGION_H
#define FIGURE_FORMATION_LEGION_H

#include "building/building.h"
#include "figure/formation.h"

int formation_legion_create_for_fort(building *fort);

void formation_legion_delete_for_fort(building *fort);

int formation_legion_recruits_needed();

void formation_legion_update_recruit_status(building *fort);

void formation_legion_move_to(formation *m, int x, int y);

void formation_legion_return_home(formation *m);

#endif // FIGURE_FORMATION_LEGION_H
