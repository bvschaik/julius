#include "combat.h"

#include "core/calc.h"
#include "figure/formation.h"
#include "figure/movement.h"
#include "figure/properties.h"
#include "figure/route.h"
#include "figure/sound.h"
#include "game/difficulty.h"
#include "map/figure.h"
#include "sound/effect.h"

static int is_attacking_native(const figure *f)
{
    return f->type == FIGURE_INDIGENOUS_NATIVE && f->action_state == FIGURE_ACTION_159_NATIVE_ATTACKING;
}

void figure_combat_handle_corpse(figure *f)
{
    if (f->wait_ticks < 0) {
        f->wait_ticks = 0;
    }
    f->wait_ticks++;
    if (f->wait_ticks >= 128) {
        f->wait_ticks = 127;
        f->state = FIGURE_STATE_DEAD;
    }
}

static int attack_is_same_direction(int dir1, int dir2)
{
    if (dir1 == dir2) {
        return 1;
    }
    int dir2_off = dir2 <= 0 ? 7 : dir2 - 1;
    if (dir1 == dir2_off) {
        return 1;
    }
    dir2_off = dir2 >= 7 ? 0 : dir2 + 1;
    if (dir1 == dir2_off) {
        return 1;
    }
    return 0;
}

static void resume_activity_after_attack(figure *f)
{
    f->num_attackers = 0;
    f->action_state = f->action_state_before_attack;
    f->opponent_id = 0;
    f->attacker_id1 = 0;
    f->attacker_id2 = 0;
    figure_route_remove(f);
}

static void hit_opponent(figure *f)
{
    const formation *m = formation_get(f->formation_id);
    figure *opponent = figure_get(f->opponent_id);
    formation *opponent_formation = formation_get(opponent->formation_id);

    const figure_properties *props = figure_properties_for_type(f->type);
    const figure_properties *opponent_props = figure_properties_for_type(opponent->type);
    int cat = opponent_props->category;
    if (cat == FIGURE_CATEGORY_CITIZEN || cat == FIGURE_CATEGORY_CRIMINAL) {
        f->attack_image_offset = 12;
    } else {
        f->attack_image_offset = 0;
    }
    int figure_attack = props->attack_value;
    int opponent_defense = opponent_props->defense_value;

    // attack modifiers
    if (f->type == FIGURE_WOLF) {
        figure_attack = difficulty_adjust_wolf_attack(figure_attack);
    }
    if (opponent->opponent_id != f->id && m->figure_type != FIGURE_FORT_LEGIONARY &&
            attack_is_same_direction(f->attack_direction, opponent->attack_direction)) {
        figure_attack += 4; // attack opponent on the (exposed) back
        sound_effect_play(SOUND_EFFECT_SWORD_SWING);
    }
    if (m->is_halted && m->figure_type == FIGURE_FORT_LEGIONARY &&
            attack_is_same_direction(f->attack_direction, m->direction)) {
        figure_attack += 4; // coordinated formation attack bonus
    }
    // defense modifiers
    if (opponent_formation->is_halted &&
            (opponent_formation->figure_type == FIGURE_FORT_LEGIONARY ||
             opponent_formation->figure_type == FIGURE_ENEMY_CAESAR_LEGIONARY)) {
        if (!attack_is_same_direction(opponent->attack_direction, opponent_formation->direction)) {
            opponent_defense -= 4; // opponent not attacking in coordinated formation
        } else if (opponent_formation->layout == FORMATION_COLUMN) {
            opponent_defense += 7;
        } else if (opponent_formation->layout == FORMATION_DOUBLE_LINE_1 ||
                   opponent_formation->layout == FORMATION_DOUBLE_LINE_2) {
            opponent_defense += 4;
        }
    }

    int max_damage = opponent_props->max_damage;
    int net_attack = figure_attack - opponent_defense;
    if (net_attack < 0) {
        net_attack = 0;
    }
    opponent->damage += net_attack;
    if (opponent->damage <= max_damage) {
        figure_play_hit_sound(f->type);
    } else {
        opponent->action_state = FIGURE_ACTION_149_CORPSE;
        opponent->wait_ticks = 0;
        figure_play_die_sound(opponent);
        formation_update_morale_after_death(opponent_formation);
    }
}

void figure_combat_handle_attack(figure *f)
{
    figure_movement_advance_attack(f);
    if (f->num_attackers == 0) {
        resume_activity_after_attack(f);
        return;
    }
    if (f->num_attackers == 1) {
        figure *target = figure_get(f->opponent_id);
        if (figure_is_dead(target)) {
            resume_activity_after_attack(f);
            return;
        }
    } else if (f->num_attackers == 2) {
        if (figure_is_dead(figure_get(f->opponent_id))) {
            if (f->opponent_id == f->attacker_id1) {
                f->opponent_id = f->attacker_id2;
            } else if (f->opponent_id == f->attacker_id2) {
                f->opponent_id = f->attacker_id1;
            }
            if (figure_is_dead(figure_get(f->opponent_id))) {
                resume_activity_after_attack(f);
                return;
            }
            f->num_attackers = 1;
            f->attacker_id1 = f->opponent_id;
            f->attacker_id2 = 0;
        }
    }
    f->attack_image_offset++;
    if (f->attack_image_offset >= 24) {
        hit_opponent(f);
    }
}

int figure_combat_get_target_for_soldier(int x, int y, int max_distance)
{
    int min_figure_id = 0;
    int min_distance = 10000;
    for (int i = 1; i < MAX_FIGURES; i++) {
        figure *f = figure_get(i);
        if (figure_is_dead(f)) {
            continue;
        }
        if (figure_is_enemy(f) || f->type == FIGURE_RIOTER || is_attacking_native(f)) {
            int distance = calc_maximum_distance(x, y, f->x, f->y);
            if (distance <= max_distance) {
                if (f->targeted_by_figure_id) {
                    distance *= 2; // penalty
                }
                if (distance < min_distance) {
                    min_distance = distance;
                    min_figure_id = i;
                }
            }
        }
    }
    if (min_figure_id) {
        return min_figure_id;
    }
    for (int i = 1; i < MAX_FIGURES; i++) {
        figure *f = figure_get(i);
        if (figure_is_dead(f)) {
            continue;
        }
        if (figure_is_enemy(f) || f->type == FIGURE_RIOTER || is_attacking_native(f)) {
            return i;
        }
    }
    return 0;
}

int figure_combat_get_target_for_wolf(int x, int y, int max_distance)
{
    int min_figure_id = 0;
    int min_distance = 10000;
    for (int i = 1; i < MAX_FIGURES; i++) {
        figure *f = figure_get(i);
        if (figure_is_dead(f) || !f->type) {
            continue;
        }
        switch (f->type) {
            case FIGURE_EXPLOSION:
            case FIGURE_FORT_STANDARD:
            case FIGURE_TRADE_SHIP:
            case FIGURE_FISHING_BOAT:
            case FIGURE_MAP_FLAG:
            case FIGURE_FLOTSAM:
            case FIGURE_SHIPWRECK:
            case FIGURE_INDIGENOUS_NATIVE:
            case FIGURE_TOWER_SENTRY:
            case FIGURE_NATIVE_TRADER:
            case FIGURE_ARROW:
            case FIGURE_JAVELIN:
            case FIGURE_BOLT:
            case FIGURE_BALLISTA:
            case FIGURE_CREATURE:
                continue;
        }
        if (figure_is_enemy(f) || figure_is_herd(f)) {
            continue;
        }
        if (figure_is_legion(f) && f->action_state == FIGURE_ACTION_80_SOLDIER_AT_REST) {
            continue;
        }
        int distance = calc_maximum_distance(x, y, f->x, f->y);
        if (f->targeted_by_figure_id) {
            distance *= 2;
        }
        if (distance < min_distance) {
            min_distance = distance;
            min_figure_id = i;
        }
    }
    if (min_distance <= max_distance && min_figure_id) {
        return min_figure_id;
    }
    return 0;
}

int figure_combat_get_target_for_enemy(int x, int y)
{
    int min_figure_id = 0;
    int min_distance = 10000;
    for (int i = 1; i < MAX_FIGURES; i++) {
        figure *f = figure_get(i);
        if (figure_is_dead(f)) {
            continue;
        }
        if (!f->targeted_by_figure_id && figure_is_legion(f)) {
            int distance = calc_maximum_distance(x, y, f->x, f->y);
            if (distance < min_distance) {
                min_distance = distance;
                min_figure_id = i;
            }
        }
    }
    if (min_figure_id) {
        return min_figure_id;
    }
    // no 'free' soldier found, take first one
    for (int i = 1; i < MAX_FIGURES; i++) {
        figure *f = figure_get(i);
        if (figure_is_dead(f)) {
            continue;
        }
        if (figure_is_legion(f)) {
            return i;
        }
    }
    return 0;
}

int figure_combat_get_missile_target_for_soldier(figure *shooter, int max_distance, map_point *tile)
{
    int x = shooter->x;
    int y = shooter->y;

    int min_distance = max_distance;
    figure *min_figure = 0;
    for (int i = 1; i < MAX_FIGURES; i++) {
        figure *f = figure_get(i);
        if (figure_is_dead(f)) {
            continue;
        }
        if (figure_is_enemy(f) || figure_is_herd(f) || is_attacking_native(f)) {
            int distance = calc_maximum_distance(x, y, f->x, f->y);
            if (distance < min_distance && figure_movement_can_launch_cross_country_missile(x, y, f->x, f->y)) {
                min_distance = distance;
                min_figure = f;
            }
        }
    }
    if (min_figure) {
        map_point_store_result(min_figure->x, min_figure->y, tile);
        return min_figure->id;
    }
    return 0;
}

int figure_combat_get_missile_target_for_enemy(figure *enemy, int max_distance, int attack_citizens,
                                               map_point *tile)
{
    int x = enemy->x;
    int y = enemy->y;

    figure *min_figure = 0;
    int min_distance = max_distance;
    for (int i = 1; i < MAX_FIGURES; i++) {
        figure *f = figure_get(i);
        if (figure_is_dead(f) || !f->type) {
            continue;
        }
        switch (f->type) {
            case FIGURE_EXPLOSION:
            case FIGURE_FORT_STANDARD:
            case FIGURE_MAP_FLAG:
            case FIGURE_FLOTSAM:
            case FIGURE_INDIGENOUS_NATIVE:
            case FIGURE_NATIVE_TRADER:
            case FIGURE_ARROW:
            case FIGURE_JAVELIN:
            case FIGURE_BOLT:
            case FIGURE_BALLISTA:
            case FIGURE_CREATURE:
            case FIGURE_FISH_GULLS:
            case FIGURE_SHIPWRECK:
            case FIGURE_SHEEP:
            case FIGURE_WOLF:
            case FIGURE_ZEBRA:
            case FIGURE_SPEAR:
                continue;
        }
        int distance;
        if (figure_is_legion(f)) {
            distance = calc_maximum_distance(x, y, f->x, f->y);
        } else if (attack_citizens && f->is_friendly) {
            distance = calc_maximum_distance(x, y, f->x, f->y) + 5;
        } else {
            continue;
        }
        if (distance < min_distance && figure_movement_can_launch_cross_country_missile(x, y, f->x, f->y)) {
            min_distance = distance;
            min_figure = f;
        }
    }
    if (min_figure) {
        map_point_store_result(min_figure->x, min_figure->y, tile);
        return min_figure->id;
    }
    return 0;
}

void figure_combat_attack_figure_at(figure *f, int grid_offset)
{
    int figure_category = figure_properties_for_type(f->type)->category;
    if (figure_category <= FIGURE_CATEGORY_INACTIVE || figure_category >= FIGURE_CATEGORY_CRIMINAL ||
            f->action_state == FIGURE_ACTION_150_ATTACK) {
        return;
    }
    int guard = 0;
    int opponent_id = map_figure_at(grid_offset);
    while (1) {
        if (++guard >= MAX_FIGURES || opponent_id <= 0) {
            break;
        }
        figure *opponent = figure_get(opponent_id);
        if (opponent_id == f->id) {
            opponent_id = opponent->next_figure_id_on_same_tile;
            continue;
        }

        int opponent_category = figure_properties_for_type(opponent->type)->category;
        int attack = 0;
        if (opponent->state != FIGURE_STATE_ALIVE) {
            attack = 0;
        } else if (opponent->action_state == FIGURE_ACTION_149_CORPSE) {
            attack = 0;
        } else if (figure_category == FIGURE_CATEGORY_ARMED && opponent_category == FIGURE_CATEGORY_NATIVE) {
            if (opponent->action_state == FIGURE_ACTION_159_NATIVE_ATTACKING) {
                attack = 1;
            }
        } else if (figure_category == FIGURE_CATEGORY_ARMED && opponent_category == FIGURE_CATEGORY_CRIMINAL) {
            attack = 1;
        } else if (figure_category == FIGURE_CATEGORY_ARMED && opponent_category == FIGURE_CATEGORY_HOSTILE) {
            attack = 1;
        } else if (figure_category == FIGURE_CATEGORY_HOSTILE && opponent_category == FIGURE_CATEGORY_CITIZEN) {
            attack = 1;
        } else if (figure_category == FIGURE_CATEGORY_HOSTILE && opponent_category == FIGURE_CATEGORY_ARMED) {
            attack = 1;
        } else if (figure_category == FIGURE_CATEGORY_HOSTILE && opponent_category == FIGURE_CATEGORY_CRIMINAL) {
            attack = 1;
        } else if (figure_category == FIGURE_CATEGORY_ARMED && opponent_category == FIGURE_CATEGORY_ANIMAL) {
            attack = 1;
        } else if (figure_category == FIGURE_CATEGORY_HOSTILE && opponent_category == FIGURE_CATEGORY_ANIMAL) {
            attack = 1;
        }
        if (attack && opponent->action_state == FIGURE_ACTION_150_ATTACK && opponent->num_attackers >= 2) {
            attack = 0;
        }
        if (attack) {
            f->action_state_before_attack = f->action_state;
            f->action_state = FIGURE_ACTION_150_ATTACK;
            f->opponent_id = opponent_id;
            f->attacker_id1 = opponent_id;
            f->num_attackers = 1;
            f->attack_image_offset = 12;
            if (opponent->x != opponent->destination_x || opponent->y != opponent->destination_y) {
                f->attack_direction = calc_general_direction(f->previous_tile_x, f->previous_tile_y,
                    opponent->previous_tile_x, opponent->previous_tile_y);
            } else {
                f->attack_direction = calc_general_direction(f->previous_tile_x, f->previous_tile_y,
                    opponent->x, opponent->y);
            }
            if (f->attack_direction >= 8) {
                f->attack_direction = 0;
            }
            if (opponent->action_state != FIGURE_ACTION_150_ATTACK) {
                opponent->action_state_before_attack = opponent->action_state;
                opponent->action_state = FIGURE_ACTION_150_ATTACK;
                opponent->attack_image_offset = 0;
                opponent->attack_direction = (f->attack_direction + 4) % 8;
            }
            if (opponent->num_attackers == 0) {
                opponent->attacker_id1 = f->id;
                opponent->opponent_id = f->id;
                opponent->num_attackers = 1;
            } else if (opponent->num_attackers == 1) {
                opponent->attacker_id2 = f->id;
                opponent->num_attackers = 2;
            }
            return;
        }
        opponent_id = opponent->next_figure_id_on_same_tile;
    }
}
