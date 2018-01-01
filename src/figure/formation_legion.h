#ifndef FIGURE_FORMATION_LEGION_H
#define FIGURE_FORMATION_LEGION_H

#include "building/building.h"
#include "figure/formation.h"

int formation_legion_create_for_fort(building *fort);

void formation_legion_delete_for_fort(building *fort);

int formation_legion_recruits_needed();

void formation_legion_update_recruit_status(building *fort);

void formation_legion_change_layout(formation *m, int new_layout);

void formation_legion_restore_layout(formation *m);

void formation_legion_move_to(formation *m, int x, int y);

void formation_legion_return_home(formation *m);

void formation_legions_dispatch_to_distant_battle();

void formation_legions_kill_in_distant_battle(int kill_percentage);

void formation_legions_return_from_distant_battle();

#endif // FIGURE_FORMATION_LEGION_H
