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

void figure_combat_handle_corpse(figure *f)
{
    if (f->waitTicks < 0) {
        f->waitTicks = 0;
    }
    f->waitTicks++;
    if (f->waitTicks >= 128) {
        f->waitTicks = 127;
        f->state = FigureState_Dead;
    }
}

static int attack_is_same_direction(int dir1, int dir2)
{
    if (dir1 == dir2) {
        return 1;
    }
    int dir2Off = dir2 <= 0 ? 7 : dir2 - 1;
    if (dir1 == dir2Off) {
        return 1;
    }
    dir2Off = dir2 >= 7 ? 0 : dir2 + 1;
    if (dir1 == dir2Off) {
        return 1;
    }
    return 0;
}

static void resume_activity_after_attack(figure *f)
{
    f->numAttackers = 0;
    f->actionState = f->actionStateBeforeAttack;
    f->opponentId = 0;
    f->attackerId1 = 0;
    f->attackerId2 = 0;
    figure_route_remove(f);
}

static void hit_opponent(figure *f)
{
    const formation *m = formation_get(f->formationId);
    figure *opponent = figure_get(f->opponentId);
    formation *opponent_formation = formation_get(opponent->formationId);
    
    const figure_properties *props = figure_properties_for_type(f->type);
    const figure_properties *opponent_props = figure_properties_for_type(opponent->type);
    int cat = opponent_props->category;
    if (cat == FIGURE_CATEGORY_CITIZEN || cat == FIGURE_CATEGORY_CRIMINAL) {
        f->attackGraphicOffset = 12;
    } else {
        f->attackGraphicOffset = 0;
    }
    int figure_attack = props->attack_value;
    int opponent_defense = opponent_props->defense_value;
    
    // attack modifiers
    if (f->type == FIGURE_WOLF) {
        figure_attack = difficulty_adjust_wolf_attack(figure_attack);
    }
    if (opponent->opponentId != f->id && m->figure_type != FIGURE_FORT_LEGIONARY &&
            attack_is_same_direction(f->attackDirection, opponent->attackDirection)) {
        figure_attack += 4; // attack opponent on the (exposed) back
        sound_effect_play(SOUND_EFFECT_SWORD_SWING);
    }
    if (m->is_halted && m->figure_type == FIGURE_FORT_LEGIONARY &&
            attack_is_same_direction(f->attackDirection, m->direction)) {
        figure_attack += 4; // coordinated formation attack bonus
    }
    // defense modifiers
    if (opponent_formation->is_halted &&
            (opponent_formation->figure_type == FIGURE_FORT_LEGIONARY ||
             opponent_formation->figure_type == FIGURE_ENEMY_CAESAR_LEGIONARY)) {
        if (!attack_is_same_direction(opponent->attackDirection, opponent_formation->direction)) {
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
        opponent->actionState = FIGURE_ACTION_149_CORPSE;
        opponent->waitTicks = 0;
        figure_play_die_sound(opponent->type);
        formation_update_morale_after_death(opponent_formation);
    }
}

void figure_combat_handle_attack(figure *f)
{
    figure_movement_advance_attack(f);
    if (f->numAttackers == 0) {
        resume_activity_after_attack(f);
        return;
    }
    if (f->numAttackers == 1) {
        figure *target = figure_get(f->opponentId);
        if (figure_is_dead(target)) {
            resume_activity_after_attack(f);
            return;
        }
    } else if (f->numAttackers == 2) {
        if (figure_is_dead(figure_get(f->opponentId))) {
            if (f->opponentId == f->attackerId1) {
                f->opponentId = f->attackerId2;
            } else if (f->opponentId == f->attackerId2) {
                f->opponentId = f->attackerId1;
            }
            if (figure_is_dead(figure_get(f->opponentId))) {
                resume_activity_after_attack(f);
                return;
            }
            f->numAttackers = 1;
            f->attackerId1 = f->opponentId;
            f->attackerId2 = 0;
        }
    }
    f->attackGraphicOffset++;
    if (f->attackGraphicOffset >= 24) {
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
        if (FigureIsEnemy(f->type) || f->type == FIGURE_RIOTER ||
            (f->type == FIGURE_INDIGENOUS_NATIVE && f->actionState == FIGURE_ACTION_159_NATIVE_ATTACKING)) {
            int distance = calc_maximum_distance(x, y, f->x, f->y);
            if (distance <= max_distance) {
                if (f->targetedByFigureId) {
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
        if (FigureIsEnemy(f->type) || f->type == FIGURE_RIOTER ||
            (f->type == FIGURE_INDIGENOUS_NATIVE && f->actionState == FIGURE_ACTION_159_NATIVE_ATTACKING)) {
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
        if (FigureIsEnemy(f->type) || FigureIsHerd(f->type)) {
            continue;
        }
        if (FigureIsLegion(f->type) && f->actionState == FIGURE_ACTION_80_SOLDIER_AT_REST) {
            continue;
        }
        int distance = calc_maximum_distance(x, y, f->x, f->y);
        if (f->targetedByFigureId) {
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
        if (!f->targetedByFigureId && FigureIsLegion(f->type)) {
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
        if (FigureIsLegion(f->type)) {
            return i;
        }
    }
    return 0;
}

int figure_combat_get_missile_target_for_soldier(figure *shooter, int max_distance, int *x_tile, int *y_tile)
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
        if (FigureIsEnemy(f->type) || FigureIsHerd(f->type) ||
            (f->type == FIGURE_INDIGENOUS_NATIVE && f->actionState == FIGURE_ACTION_159_NATIVE_ATTACKING)) {
            int distance = calc_maximum_distance(x, y, f->x, f->y);
            if (distance < min_distance && figure_movement_can_launch_cross_country_missile(x, y, f->x, f->y)) {
                min_distance = distance;
                min_figure = f;
            }
        }
    }
    if (min_figure) {
        *x_tile = min_figure->x;
        *y_tile = min_figure->y;
        return min_figure->id;
    }
    return 0;
}

int figure_combat_get_missile_target_for_enemy(figure *enemy, int max_distance, int attack_citizens,
                                               int *x_tile, int *y_tile)
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
        if (FigureIsLegion(f->type)) {
            distance = calc_maximum_distance(x, y, f->x, f->y);
        } else if (attack_citizens && f->isFriendly) {
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
        *x_tile = min_figure->x;
        *y_tile = min_figure->y;
        return min_figure->id;
    }
    return 0;
}

void figure_combat_attack_figure_at(figure *f, int grid_offset)
{
    int figure_category = figure_properties_for_type(f->type)->category;
    if (figure_category <= FIGURE_CATEGORY_INACTIVE || figure_category >= FIGURE_CATEGORY_CRIMINAL ||
            f->actionState == FIGURE_ACTION_150_ATTACK) {
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
            opponent_id = opponent->nextFigureIdOnSameTile;
            continue;
        }
        
        int opponent_category = figure_properties_for_type(opponent->type)->category;
        int attack = 0;
        if (opponent->state != FigureState_Alive) {
            attack = 0;
        } else if (opponent->actionState == FIGURE_ACTION_149_CORPSE) {
            attack = 0;
        } else if (figure_category == FIGURE_CATEGORY_ARMED && opponent_category == FIGURE_CATEGORY_NATIVE) {
            if (opponent->actionState == FIGURE_ACTION_159_NATIVE_ATTACKING) {
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
        if (attack && opponent->actionState == FIGURE_ACTION_150_ATTACK && opponent->numAttackers >= 2) {
            attack = 0;
        }
        if (attack) {
            f->actionStateBeforeAttack = f->actionState;
            f->actionState = FIGURE_ACTION_150_ATTACK;
            f->opponentId = opponent_id;
            f->attackerId1 = opponent_id;
            f->numAttackers = 1;
            f->attackGraphicOffset = 12;
            if (opponent->x != opponent->destinationX || opponent->y != opponent->destinationY) {
                f->attackDirection = calc_general_direction(f->previousTileX, f->previousTileY,
                    opponent->previousTileX, opponent->previousTileY);
            } else {
                f->attackDirection = calc_general_direction(f->previousTileX, f->previousTileY,
                    opponent->x, opponent->y);
            }
            if (f->attackDirection >= 8) {
                f->attackDirection = 0;
            }
            if (opponent->actionState != FIGURE_ACTION_150_ATTACK) {
                opponent->actionStateBeforeAttack = opponent->actionState;
                opponent->actionState = FIGURE_ACTION_150_ATTACK;
                opponent->attackGraphicOffset = 0;
                opponent->attackDirection = (f->attackDirection + 4) % 8;
            }
            if (opponent->numAttackers == 0) {
                opponent->attackerId1 = f->id;
                opponent->opponentId = f->id;
                opponent->numAttackers = 1;
            } else if (opponent->numAttackers == 1) {
                opponent->attackerId2 = f->id;
                opponent->numAttackers = 2;
            }
            return;
        }
        opponent_id = opponent->nextFigureIdOnSameTile;
    }
}
