#ifndef FIGURE_ENEMY_ARMY_H
#define FIGURE_ENEMY_ARMY_H

#include "core/buffer.h"

typedef struct {
    int formation_id;
    int layout;
    int home_x;
    int home_y;
    int destination_x;
    int destination_y;
    int destination_building_id;
    int num_legions;
    int ignore_roman_soldiers;
} enemy_army;

void enemy_armies_clear();

const enemy_army *enemy_army_get(int invasion_id);

enemy_army *enemy_army_get_editable(int invasion_id);

void enemy_armies_clear_ignore_roman_soldiers();

void enemy_armies_clear_formations();


int enemy_army_total_enemy_formations();

void enemy_army_totals_clear();

void enemy_army_totals_add_legion_formation(int strength);

void enemy_army_totals_add_enemy_formation(int strength);

int enemy_army_totals_should_calculate_roman_influence();

int enemy_army_is_stronger_than_legions();

void enemy_armies_save_state(buffer *buf, buffer *totals);

void enemy_armies_load_state(buffer *buf, buffer *totals);

#endif // FIGURE_ENEMY_ARMY_H
