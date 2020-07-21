#include "enemy_army.h"

#define MAX_ENEMY_ARMIES 125

#include "figure/formation.h"
#include "map/soldier_strength.h"

static enemy_army enemy_armies[MAX_ENEMY_ARMIES];

static struct {
    int enemy_formations;
    int enemy_strength;
    int legion_formations;
    int legion_strength;

    int days_since_roman_influence_calculation;
} totals;

void enemy_armies_clear(void)
{
    for (int i = 0; i < MAX_ENEMY_ARMIES; i++) {
        enemy_armies[i].formation_id = 0;
        enemy_armies[i].layout = 0;
        enemy_armies[i].home_x = 0;
        enemy_armies[i].home_y = 0;
        enemy_armies[i].destination_x = 0;
        enemy_armies[i].destination_y = 0;
        enemy_armies[i].destination_building_id = 0;
        enemy_armies[i].ignore_roman_soldiers = 0;
    }
    totals.enemy_formations = 0;
    totals.enemy_strength = 0;
    totals.legion_formations = 0;
    totals.legion_strength = 0;
    totals.days_since_roman_influence_calculation = 0;
}

const enemy_army *enemy_army_get(int invasion_id)
{
    return &enemy_armies[invasion_id];
}

enemy_army *enemy_army_get_editable(int invasion_id)
{
    return &enemy_armies[invasion_id];
}

void enemy_armies_clear_ignore_roman_soldiers(void)
{
    for (int i = 0; i < MAX_ENEMY_ARMIES; i++) {
        enemy_armies[i].ignore_roman_soldiers = 0;
    }
}

void enemy_armies_clear_formations(void)
{
    for (int i = 0; i < MAX_ENEMY_ARMIES; i++) {
        enemy_armies[i].formation_id = 0;
        enemy_armies[i].num_legions = 0;
    }
}

void enemy_army_totals_clear(void)
{
    totals.legion_formations = 0;
    totals.legion_strength = 0;
    totals.enemy_formations = 0;
    totals.enemy_strength = 0;
}

void enemy_army_totals_add_legion_formation(int strength)
{
    totals.legion_formations++;
    totals.legion_strength += strength;
}

void enemy_army_totals_add_enemy_formation(int strength)
{
    totals.enemy_formations++;
    totals.enemy_strength += strength;
}

int enemy_army_total_enemy_formations(void)
{
    return totals.enemy_formations;
}

void enemy_army_calculate_roman_influence(void)
{
    totals.days_since_roman_influence_calculation++;
    if (totals.days_since_roman_influence_calculation > 4) {
        totals.days_since_roman_influence_calculation = 0;
    } else {
        return;
    }
    map_soldier_strength_clear();
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        const formation *m = formation_get(i);
        if (m->in_use != 1 || !m->is_legion) {
            continue;
        }
        if (m->num_figures > 0) {
            map_soldier_strength_add(m->x_home, m->y_home, 7, 1);
        }
        if (m->num_figures > 3) {
            map_soldier_strength_add(m->x_home, m->y_home, 6, 1);
        }
        if (m->num_figures > 6) {
            map_soldier_strength_add(m->x_home, m->y_home, 5, 1);
        }
        if (m->num_figures > 9) {
            map_soldier_strength_add(m->x_home, m->y_home, 4, 1);
        }
        if (m->num_figures > 12) {
            map_soldier_strength_add(m->x_home, m->y_home, 3, 1);
        }
        if (m->num_figures > 15) {
            map_soldier_strength_add(m->x_home, m->y_home, 2, 1);
        }
    }
}

int enemy_army_is_stronger_than_legions(void)
{
    return totals.enemy_strength > 2 * totals.legion_strength;
}

void enemy_armies_save_state(buffer *buf, buffer *totals_buf)
{
    for (int i = 0; i < MAX_ENEMY_ARMIES; i++) {
        buffer_write_i32(buf, enemy_armies[i].formation_id);
    }
    for (int i = 0; i < MAX_ENEMY_ARMIES; i++) {
        buffer_write_i32(buf, enemy_armies[i].home_x);
    }
    for (int i = 0; i < MAX_ENEMY_ARMIES; i++) {
        buffer_write_i32(buf, enemy_armies[i].home_y);
    }
    for (int i = 0; i < MAX_ENEMY_ARMIES; i++) {
        buffer_write_i32(buf, enemy_armies[i].layout);
    }
    for (int i = 0; i < MAX_ENEMY_ARMIES; i++) {
        buffer_write_i32(buf, enemy_armies[i].destination_x);
    }
    for (int i = 0; i < MAX_ENEMY_ARMIES; i++) {
        buffer_write_i32(buf, enemy_armies[i].destination_y);
    }
    for (int i = 0; i < MAX_ENEMY_ARMIES; i++) {
        buffer_write_i32(buf, enemy_armies[i].destination_building_id);
    }
    for (int i = 0; i < MAX_ENEMY_ARMIES; i++) {
        buffer_write_i32(buf, enemy_armies[i].num_legions);
    }
    for (int i = 0; i < MAX_ENEMY_ARMIES; i++) {
        buffer_write_i32(buf, enemy_armies[i].ignore_roman_soldiers);
    }
    buffer_write_i32(totals_buf, totals.enemy_formations);
    buffer_write_i32(totals_buf, totals.enemy_strength);
    buffer_write_i32(totals_buf, totals.legion_formations);
    buffer_write_i32(totals_buf, totals.legion_strength);
    buffer_write_i32(totals_buf, totals.days_since_roman_influence_calculation);
}

void enemy_armies_load_state(buffer *buf, buffer *totals_buf)
{
    for (int i = 0; i < MAX_ENEMY_ARMIES; i++) {
        enemy_armies[i].formation_id = buffer_read_i32(buf);
    }
    for (int i = 0; i < MAX_ENEMY_ARMIES; i++) {
        enemy_armies[i].home_x = buffer_read_i32(buf);
    }
    for (int i = 0; i < MAX_ENEMY_ARMIES; i++) {
        enemy_armies[i].home_y = buffer_read_i32(buf);
    }
    for (int i = 0; i < MAX_ENEMY_ARMIES; i++) {
        enemy_armies[i].layout = buffer_read_i32(buf);
    }
    for (int i = 0; i < MAX_ENEMY_ARMIES; i++) {
        enemy_armies[i].destination_x = buffer_read_i32(buf);
    }
    for (int i = 0; i < MAX_ENEMY_ARMIES; i++) {
        enemy_armies[i].destination_y = buffer_read_i32(buf);
    }
    for (int i = 0; i < MAX_ENEMY_ARMIES; i++) {
        enemy_armies[i].destination_building_id = buffer_read_i32(buf);
    }
    for (int i = 0; i < MAX_ENEMY_ARMIES; i++) {
        enemy_armies[i].num_legions = buffer_read_i32(buf);
    }
    for (int i = 0; i < MAX_ENEMY_ARMIES; i++) {
        enemy_armies[i].ignore_roman_soldiers = buffer_read_i32(buf);
    }
    totals.enemy_formations = buffer_read_i32(totals_buf);
    totals.enemy_strength = buffer_read_i32(totals_buf);
    totals.legion_formations = buffer_read_i32(totals_buf);
    totals.legion_strength = buffer_read_i32(totals_buf);
    totals.days_since_roman_influence_calculation = buffer_read_i32(totals_buf);
}
