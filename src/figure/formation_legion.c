#include "formation_legion.h"

#include "city/figures.h"
#include "city/games.h"
#include "city/military.h"
#include "city/warning.h"
#include "core/calc.h"
#include "figure/enemy_army.h"
#include "figure/figure.h"
#include "figure/route.h"
#include "map/building.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/routing.h"
#include "scenario/distant_battle.h"

int formation_legion_create_for_fort(building *fort)
{
    formation_calculate_legion_totals();

    formation *m = formation_create_legion(fort->id, fort->x, fort->y, fort->subtype.fort_figure_type);
    if (!m->id) {
        return 0;
    }
    figure *standard = figure_create(FIGURE_FORT_STANDARD, 0, 0, DIR_0_TOP);
    standard->building_id = fort->id;
    standard->formation_id = m->id;
    m->standard_figure_id = standard->id;

    return m->id;
}

void formation_legion_delete_for_fort(building *fort)
{
    if (fort->formation_id > 0) {
        formation *m = formation_get(fort->formation_id);
        if (m->in_use) {
            if (m->standard_figure_id) {
                figure_delete(figure_get(m->standard_figure_id));
            }
            for (int i = 0; i < m->num_figures; ++i) {
                figure_get(m->figures[i])->action_state = FIGURE_ACTION_149_CORPSE;
            }
            formation_clear(fort->formation_id);
            formation_calculate_legion_totals();
        }
    }
}

int formation_legion_recruits_needed(void)
{
    for (int i = 1; i < formation_count(); i++) {
        formation *m = formation_get(i);
        if (m->in_use && m->is_legion && m->legion_recruit_type != LEGION_RECRUIT_NONE) {
            return 1;
        }
    }
    return 0;
}

void formation_legion_update_recruit_status(building *fort)
{
    formation *m = formation_get(fort->formation_id);
    m->legion_recruit_type = LEGION_RECRUIT_NONE;
    if (!m->is_at_fort || m->cursed_by_mars || m->num_figures == m->max_figures) {
        return;
    }
    if (m->num_figures < m->max_figures) {
        int type = fort->subtype.fort_figure_type;
        if (type == FIGURE_FORT_LEGIONARY) {
            m->legion_recruit_type = LEGION_RECRUIT_LEGIONARY;
        } else if (type == FIGURE_FORT_JAVELIN) {
            m->legion_recruit_type = LEGION_RECRUIT_JAVELIN;
        } else if (type == FIGURE_FORT_MOUNTED) {
            m->legion_recruit_type = LEGION_RECRUIT_MOUNTED;
        }
    } else { // too many figures
        int too_many = m->num_figures - m->max_figures;
        for (int i = MAX_FORMATION_FIGURES - 1; i >= 0 && too_many > 0; i--) {
            if (m->figures[i]) {
                figure_get(m->figures[i])->action_state = FIGURE_ACTION_82_SOLDIER_RETURNING_TO_BARRACKS;
                too_many--;
            }
        }
        formation_calculate_figures();
    }
}

void formation_legion_change_layout(formation *m, int new_layout)
{
    if (new_layout == FORMATION_MOP_UP && m->layout != FORMATION_MOP_UP) {
        m->prev.layout = m->layout;
    }
    m->layout = new_layout;
}

void formation_legion_restore_layout(formation *m)
{
    if (m->layout == FORMATION_MOP_UP) {
        m->layout = m->prev.layout;
    }
}

static int prepare_to_move(formation *m)
{
    if (m->months_very_low_morale || m->months_low_morale > 1) {
        return 0;
    }
    if (m->months_low_morale == 1) {
        formation_change_morale(m, 10); // yay, we can move!
    }
    return 1;
}

void formation_legion_move_to(formation *m, const map_tile *tile)
{
    map_routing_calculate_distances(m->x_home, m->y_home);
    if (map_routing_distance(tile->grid_offset) <= 0) {
        return; // unable to route there
    }
    if (tile->x == m->x_home && tile->y == m->y_home) {
        return; // use formation_legion_return_home
    }
    if (m->cursed_by_mars) {
        return;
    }
    m->standard_x = tile->x;
    m->standard_y = tile->y;
    m->is_at_fort = 0;
    m->target_formation_id = 0;

    int figure_id = map_figure_at(tile->grid_offset);
    while (figure_id) {
        figure *f = figure_get(figure_id);
        if (f->formation_id) {
            formation *formation = formation_get(f->formation_id);
            if (!formation->is_legion) {
                m->target_formation_id = formation->id;
                break;
            }
        }
        
        figure_id = f->next_figure_id_on_same_tile;
    }

    if (m->morale <= 20) {
        city_warning_show(WARNING_LEGION_MORALE_TOO_LOW, NEW_WARNING_SLOT);
    }
    for (int i = 0; i < MAX_FORMATION_FIGURES && m->figures[i]; i++) {
        figure *f = figure_get(m->figures[i]);
        if (f->action_state == FIGURE_ACTION_149_CORPSE ||
            f->action_state == FIGURE_ACTION_150_ATTACK) {
            continue;
        }
        if (prepare_to_move(m)) {
            f->alternative_location_index = 0;
            f->action_state = FIGURE_ACTION_83_SOLDIER_GOING_TO_STANDARD;
            figure_route_remove(f);
        }
    }
}

void formation_legion_return_home(formation *m)
{
    map_routing_calculate_distances(m->x_home, m->y_home);
    if (map_routing_distance(map_grid_offset(m->x, m->y)) <= 0) {
        return; // unable to route home
    }
    if (m->cursed_by_mars) {
        return;
    }
    m->is_at_fort = 1;
    formation_legion_restore_layout(m);
    for (int i = 0; i < MAX_FORMATION_FIGURES && m->figures[i]; i++) {
        figure *f = figure_get(m->figures[i]);
        if (f->action_state == FIGURE_ACTION_149_CORPSE ||
            f->action_state == FIGURE_ACTION_150_ATTACK) {
            continue;
        }
        if (prepare_to_move(m)) {
            f->action_state = FIGURE_ACTION_81_SOLDIER_GOING_TO_FORT;
            figure_route_remove(f);
            f->formation_at_rest = 1;
        }
    }
}

static int dispatch_soldiers(formation *m)
{
    m->in_distant_battle = 1;
    m->is_at_fort = 0;
    for (int fig = 0; fig < m->num_figures; fig++) {
        if (m->figures[fig] > 0) {
            figure *f = figure_get(m->figures[fig]);
            if (!figure_is_dead(f)) {
                f->action_state = FIGURE_ACTION_87_SOLDIER_GOING_TO_DISTANT_BATTLE;
            }
        }
    }
    int strength_factor;
    if (m->has_military_training) {
        strength_factor = m->figure_type == FIGURE_FORT_LEGIONARY ? 3 : 2;
    } else {
        strength_factor = m->figure_type == FIGURE_FORT_LEGIONARY ? 2 : 1;
    }
    if (city_games_naval_battle_distant_battle_bonus_active()) {
        strength_factor += 1;
    }
    return strength_factor * m->num_figures;
}

void formation_legions_dispatch_to_distant_battle(void)
{
    int num_legions = 0;
    int roman_strength = 0;
    for (int i = 1; i < formation_count(); i++) {
        formation *m = formation_get(i);
        if (m->in_use && m->is_legion && m->empire_service && m->num_figures > 0) {
            roman_strength += dispatch_soldiers(m);
            num_legions++;
        }
    }
    // Protect from overflow -> only stores 1 unsigned byte
    if (roman_strength > 255) {
        roman_strength = 255;
    }
    if (num_legions > 0) {
        city_games_remove_naval_battle_distant_battle_bonus();
        city_military_dispatch_to_distant_battle(roman_strength);
    }
}

static void kill_soldiers(formation *m, int kill_percentage)
{
    formation_change_morale(m, -75);
    int soldiers_total = 0;
    for (int fig = 0; fig < m->num_figures; fig++) {
        if (m->figures[fig] > 0) {
            figure *f = figure_get(m->figures[fig]);
            if (!figure_is_dead(f)) {
                soldiers_total++;
            }
        }
    }
    int soldiers_to_kill = calc_adjust_with_percentage(soldiers_total, kill_percentage);
    if (soldiers_to_kill >= soldiers_total) {
        m->is_at_fort = 1;
        m->in_distant_battle = 0;
    }
    for (int fig = 0; fig < m->num_figures; fig++) {
        if (m->figures[fig] > 0) {
            figure *f = figure_get(m->figures[fig]);
            if (!figure_is_dead(f)) {
                if (soldiers_to_kill) {
                    soldiers_to_kill--;
                    f->state = FIGURE_STATE_DEAD;
                }
            }
        }
    }
}

void formation_legions_kill_in_distant_battle(int kill_percentage)
{
    for (int i = 1; i < formation_count(); i++) {
        formation *m = formation_get(i);
        if (m->in_use && m->is_legion && m->in_distant_battle) {
            kill_soldiers(m, kill_percentage);
        }
    }
}

static void return_soldiers(formation *m)
{
    m->in_distant_battle = 0;
    for (int fig = 0; fig < m->num_figures; fig++) {
        if (m->figures[fig] > 0) {
            figure *f = figure_get(m->figures[fig]);
            if (!figure_is_dead(f)) {
                f->action_state = FIGURE_ACTION_88_SOLDIER_RETURNING_FROM_DISTANT_BATTLE;
                f->formation_at_rest = 1;
            }
        }
    }
}

void formation_legions_return_from_distant_battle(void)
{
    for (int i = 1; i < formation_count(); i++) {
        formation *m = formation_get(i);
        if (m->in_use && m->is_legion && m->in_distant_battle) {
            return_soldiers(m);
        }
    }
}

int formation_legion_curse(void)
{
    formation *best_legion = 0;
    int best_legion_weight = 0;
    for (int i = 1; i < formation_count(); i++) {
        formation *m = formation_get(i);
        if (m->in_use == 1 && m->is_legion) {
            int weight = m->num_figures;
            if (m->figure_type == FIGURE_FORT_LEGIONARY) {
                weight *= 2;
            }
            if (weight > best_legion_weight) {
                best_legion_weight = weight;
                best_legion = m;
            }
        }
    }
    if (!best_legion) {
        return 0;
    }
    for (int i = 0; i < MAX_FORMATION_FIGURES; i++) {
        if (best_legion->figures[i] > 0) {
            figure_get(best_legion->figures[i])->action_state = FIGURE_ACTION_82_SOLDIER_RETURNING_TO_BARRACKS;
        }
    }
    best_legion->cursed_by_mars = 96;
    formation_calculate_figures();
    return 1;
}

static int is_legion(figure *f)
{
    if (figure_is_legion(f) || f->type == FIGURE_FORT_STANDARD) {
        return f->formation_id;
    }
    return 0;
}

static int is_herd(figure *f)
{
    if (figure_is_herd(f)) {
        return f->formation_id;
    }
    return 0;
}

int formation_legion_at_grid_offset(int grid_offset)
{
    return map_figure_foreach_until(grid_offset, is_legion);
}

int formation_legion_or_herd_at_grid_offset(int grid_offset)
{
    int formation_id = map_figure_foreach_until(grid_offset, is_legion);
    if (formation_id) {
        return formation_id;
    }
    return map_figure_foreach_until(grid_offset, is_herd);
}

int formation_legion_at_building(int grid_offset)
{
    int building_id = map_building_at(grid_offset);
    if (building_id > 0) {
        building *b = building_get(building_id);
        if (b->state == BUILDING_STATE_IN_USE && (b->type == BUILDING_FORT || b->type == BUILDING_FORT_GROUND)) {
            return b->formation_id;
        }
    }
    return 0;
}

void formation_legion_update(void)
{
    for (int i = 1; i < formation_count(); i++) {
        formation *m = formation_get(i);
        if (m->in_use != 1 || !m->is_legion) {
            continue;
        }
        formation_decrease_monthly_counters(m);
        if (city_figures_enemies() <= 0) {
            formation_clear_monthly_counters(m);
        }
        for (int n = 0; n < MAX_FORMATION_FIGURES; n++) {
            if (figure_get(m->figures[n])->action_state == FIGURE_ACTION_150_ATTACK) {
                formation_record_fight(m);
            }
        }
        if (formation_has_low_morale(m)) {
            // flee back to fort
            for (int n = 0; n < MAX_FORMATION_FIGURES; n++) {
                figure *f = figure_get(m->figures[n]);
                if (f->action_state != FIGURE_ACTION_150_ATTACK &&
                    f->action_state != FIGURE_ACTION_149_CORPSE &&
                    f->action_state != FIGURE_ACTION_148_FLEEING) {
                    f->action_state = FIGURE_ACTION_148_FLEEING;
                    figure_route_remove(f);
                }
            }
        } else if (m->layout == FORMATION_MOP_UP) {
            if (enemy_army_total_enemy_formations() +
                city_figures_rioters() +
                city_figures_attacking_natives() > 0) {
                for (int n = 0; n < MAX_FORMATION_FIGURES; n++) {
                    if (m->figures[n] != 0) {
                        figure *f = figure_get(m->figures[n]);
                        if (f->action_state != FIGURE_ACTION_150_ATTACK &&
                            f->action_state != FIGURE_ACTION_149_CORPSE) {
                            f->action_state = FIGURE_ACTION_86_SOLDIER_MOPPING_UP;
                        }
                    }
                }
            } else {
                formation_legion_restore_layout(m);
            }
        }
    }
}

void formation_legion_decrease_damage(void)
{
    for (int i = 1; i < figure_count(); i++) {
        figure *f = figure_get(i);
        if (f->state == FIGURE_STATE_ALIVE && figure_is_legion(f)) {
            if (f->action_state == FIGURE_ACTION_80_SOLDIER_AT_REST) {
                if (f->damage) {
                    f->damage--;
                }
            }
        }
    }
}
