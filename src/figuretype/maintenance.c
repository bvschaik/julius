#include "maintenance.h"

#include "building/building.h"
#include "building/list.h"
#include "building/maintenance.h"
#include "city/figures.h"
#include "core/calc.h"
#include "core/image.h"
#include "figure/combat.h"
#include "figure/enemy_army.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "map/building.h"
#include "map/road_access.h"
#include "sound/effect.h"

void figure_engineer_action(figure *f)
{
    building *b = building_get(f->building_id);

    f->terrain_usage = TERRAIN_USAGE_ROADS;
    f->use_cross_country = 0;
    f->max_roam_length = 640;
    if (b->state != BUILDING_STATE_IN_USE || b->figure_id != f->id) {
        f->state = FIGURE_STATE_DEAD;
    }
    figure_image_increase_offset(f, 12);

    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_60_ENGINEER_CREATED:
            f->is_ghost = 1;
            f->image_offset = 0;
            f->wait_ticks--;
            if (f->wait_ticks <= 0) {
                int x_road, y_road;
                if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                    f->action_state = FIGURE_ACTION_61_ENGINEER_ENTERING_EXITING;
                    figure_movement_set_cross_country_destination(f, x_road, y_road);
                    f->roam_length = 0;
                } else {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            break;
        case FIGURE_ACTION_61_ENGINEER_ENTERING_EXITING:
            f->use_cross_country = 1;
            f->is_ghost = 1;
            if (figure_movement_move_ticks_cross_country(f, 1) == 1) {
                if (map_building_at(f->grid_offset) == f->building_id) {
                    // returned to own building
                    f->state = FIGURE_STATE_DEAD;
                } else {
                    f->action_state = FIGURE_ACTION_62_ENGINEER_ROAMING;
                    figure_movement_init_roaming(f);
                    f->roam_length = 0;
                }
            }
            break;
        case FIGURE_ACTION_62_ENGINEER_ROAMING:
            f->is_ghost = 0;
            f->roam_length++;
            if (f->roam_length >= f->max_roam_length) {
                int x_road, y_road;
                if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                    f->action_state = FIGURE_ACTION_63_ENGINEER_RETURNING;
                    f->destination_x = x_road;
                    f->destination_y = y_road;
                } else {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            figure_movement_roam_ticks(f, 1);
            break;
        case FIGURE_ACTION_63_ENGINEER_RETURNING:
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_61_ENGINEER_ENTERING_EXITING;
                figure_movement_set_cross_country_destination(f, b->x, b->y);
                f->roam_length = 0;
            } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
    }
    figure_image_update(f, image_group(GROUP_FIGURE_ENGINEER));
}

static int get_nearest_enemy(int x, int y, int *distance)
{
    int min_enemy_id = 0;
    int min_dist = 10000;
    for (int i = 1; i < MAX_FIGURES; i++) {
        figure *f = figure_get(i);
        if (f->state != FIGURE_STATE_ALIVE || f->targeted_by_figure_id) {
            continue;
        }
        int dist;
        if (f->type == FIGURE_RIOTER || f->type == FIGURE_ENEMY54_GLADIATOR) {
            dist = calc_maximum_distance(x, y, f->x, f->y);
        } else if (f->type == FIGURE_INDIGENOUS_NATIVE && f->action_state == FIGURE_ACTION_159_NATIVE_ATTACKING) {
            dist = calc_maximum_distance(x, y, f->x, f->y);
        } else if (figure_is_enemy(f)) {
            dist = 3 * calc_maximum_distance(x, y, f->x, f->y);
        } else if (f->type == FIGURE_WOLF) {
            dist = 4 * calc_maximum_distance(x, y, f->x, f->y);
        } else {
            continue;
        }
        if (dist < min_dist) {
            min_dist = dist;
            min_enemy_id = i;
        }
    }
    *distance = min_dist;
    return min_enemy_id;
}

static int fight_enemy(figure *f)
{
    if (!city_figures_has_security_breach() && enemy_army_total_enemy_formations() <= 0) {
        return 0;
    }
    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
        case FIGURE_ACTION_149_CORPSE:
        case FIGURE_ACTION_70_PREFECT_CREATED:
        case FIGURE_ACTION_71_PREFECT_ENTERING_EXITING:
        case FIGURE_ACTION_74_PREFECT_GOING_TO_FIRE:
        case FIGURE_ACTION_75_PREFECT_AT_FIRE:
        case FIGURE_ACTION_76_PREFECT_GOING_TO_ENEMY:
        case FIGURE_ACTION_77_PREFECT_AT_ENEMY:
            return 0;
    }
    f->wait_ticks_next_target++;
    if (f->wait_ticks_next_target < 10) {
        return 0;
    }
    f->wait_ticks_next_target = 0;
    int distance;
    int enemy_id = get_nearest_enemy(f->x, f->y, &distance);
    if (enemy_id > 0 && distance <= 30) {
        figure *enemy = figure_get(enemy_id);
        f->wait_ticks_next_target = 0;
        f->action_state = FIGURE_ACTION_76_PREFECT_GOING_TO_ENEMY;
        f->destination_x = enemy->x;
        f->destination_y = enemy->y;
        f->target_figure_id = enemy_id;
        enemy->targeted_by_figure_id = f->id;
        f->target_figure_created_sequence = enemy->created_sequence;
        figure_route_remove(f);
        return 1;
    }
    return 0;
}

static int fight_fire(figure *f)
{
    if (building_list_burning_size() <= 0) {
        return 0;
    }
    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
        case FIGURE_ACTION_149_CORPSE:
        case FIGURE_ACTION_70_PREFECT_CREATED:
        case FIGURE_ACTION_71_PREFECT_ENTERING_EXITING:
        case FIGURE_ACTION_74_PREFECT_GOING_TO_FIRE:
        case FIGURE_ACTION_75_PREFECT_AT_FIRE:
        case FIGURE_ACTION_76_PREFECT_GOING_TO_ENEMY:
        case FIGURE_ACTION_77_PREFECT_AT_ENEMY:
            return 0;
    }
    f->wait_ticks_missile++;
    if (f->wait_ticks_missile < 20) {
        return 0;
    }
    int distance;
    int ruin_id = building_maintenance_get_closest_burning_ruin(f->x, f->y, &distance);
    if (ruin_id > 0 && distance <= 25) {
        building *ruin = building_get(ruin_id);
        f->wait_ticks_missile = 0;
        f->action_state = FIGURE_ACTION_74_PREFECT_GOING_TO_FIRE;
        f->destination_x = ruin->road_access_x;
        f->destination_y = ruin->road_access_y;
        f->destination_building_id = ruin_id;
        figure_route_remove(f);
        ruin->figure_id4 = f->id;
        return 1;
    }
    return 0;
}

static void extinguish_fire(figure *f)
{
    building *burn = building_get(f->destination_building_id);
    int distance = calc_maximum_distance(f->x, f->y, burn->x, burn->y);
    if (burn->state == BUILDING_STATE_IN_USE && burn->type == BUILDING_BURNING_RUIN && distance < 2) {
        burn->fire_duration = 32;
        sound_effect_play(SOUND_EFFECT_FIRE_SPLASH);
    } else {
        f->wait_ticks = 1;
    }
    f->attack_direction = calc_general_direction(f->x, f->y, burn->x, burn->y);
    if (f->attack_direction >= 8) {
        f->attack_direction = 0;
    }
    f->wait_ticks--;
    if (f->wait_ticks <= 0) {
        f->wait_ticks_missile = 20;
        if (!fight_fire(f)) {
            building *b = building_get(f->building_id);
            int x_road, y_road;
            if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                f->action_state = FIGURE_ACTION_73_PREFECT_RETURNING;
                f->destination_x = x_road;
                f->destination_y = y_road;
                figure_route_remove(f);
            } else {
                f->state = FIGURE_STATE_DEAD;
            }
        }
    }
}

static int target_is_alive(figure *f)
{
    if (f->target_figure_id <= 0) {
        return 0;
    }
    figure *target = figure_get(f->target_figure_id);
    if (!figure_is_dead(target) && target->created_sequence == f->target_figure_created_sequence) {
        return 1;
    }
    return 0;
}

void figure_prefect_action(figure *f)
{
    building *b = building_get(f->building_id);

    f->terrain_usage = TERRAIN_USAGE_ROADS;
    f->use_cross_country = 0;
    f->max_roam_length = 640;
    if (b->state != BUILDING_STATE_IN_USE || b->figure_id != f->id) {
        f->state = FIGURE_STATE_DEAD;
    }
    figure_image_increase_offset(f, 12);

    // special actions
    if (!fight_enemy(f)) {
        fight_fire(f);
    }
    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_70_PREFECT_CREATED:
            f->is_ghost = 1;
            f->image_offset = 0;
            f->wait_ticks--;
            if (f->wait_ticks <= 0) {
                int x_road, y_road;
                if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                    f->action_state = FIGURE_ACTION_71_PREFECT_ENTERING_EXITING;
                    figure_movement_set_cross_country_destination(f, x_road, y_road);
                    f->roam_length = 0;
                } else {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            break;
        case FIGURE_ACTION_71_PREFECT_ENTERING_EXITING:
            f->use_cross_country = 1;
            f->is_ghost = 1;
            if (figure_movement_move_ticks_cross_country(f, 1) == 1) {
                if (map_building_at(f->grid_offset) == f->building_id) {
                    // returned to own building
                    f->state = FIGURE_STATE_DEAD;
                } else {
                    f->action_state = FIGURE_ACTION_72_PREFECT_ROAMING;
                    figure_movement_init_roaming(f);
                    f->roam_length = 0;
                }
            }
            break;
        case FIGURE_ACTION_72_PREFECT_ROAMING:
            f->is_ghost = 0;
            f->roam_length++;
            if (f->roam_length >= f->max_roam_length) {
                int x_road, y_road;
                if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                    f->action_state = FIGURE_ACTION_73_PREFECT_RETURNING;
                    f->destination_x = x_road;
                    f->destination_y = y_road;
                    figure_route_remove(f);
                } else {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            figure_movement_roam_ticks(f, 1);
            break;
        case FIGURE_ACTION_73_PREFECT_RETURNING:
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_71_PREFECT_ENTERING_EXITING;
                figure_movement_set_cross_country_destination(f, b->x, b->y);
                f->roam_length = 0;
            } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_74_PREFECT_GOING_TO_FIRE:
            f->terrain_usage = TERRAIN_USAGE_ANY;
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_75_PREFECT_AT_FIRE;
                figure_route_remove(f);
                f->roam_length = 0;
                f->wait_ticks = 50;
            } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_75_PREFECT_AT_FIRE:
            extinguish_fire(f);
            break;
        case FIGURE_ACTION_76_PREFECT_GOING_TO_ENEMY:
            f->terrain_usage = TERRAIN_USAGE_ANY;
            if (!target_is_alive(f)) {
                int x_road, y_road;
                if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                    f->action_state = FIGURE_ACTION_73_PREFECT_RETURNING;
                    f->destination_x = x_road;
                    f->destination_y = y_road;
                    figure_route_remove(f);
                    f->roam_length = 0;
                } else {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                figure *target = figure_get(f->target_figure_id);
                f->destination_x = target->x;
                f->destination_y = target->y;
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
    }
    // graphic id
    int dir;
    if (f->action_state == FIGURE_ACTION_75_PREFECT_AT_FIRE ||
        f->action_state == FIGURE_ACTION_150_ATTACK) {
        dir = f->attack_direction;
    } else if (f->direction < 8) {
        dir = f->direction;
    } else {
        dir = f->previous_tile_direction;
    }
    dir = figure_image_normalize_direction(dir);
    switch (f->action_state) {
        case FIGURE_ACTION_74_PREFECT_GOING_TO_FIRE:
            f->image_id = image_group(GROUP_FIGURE_PREFECT_WITH_BUCKET) +
                dir + 8 * f->image_offset;
            break;
        case FIGURE_ACTION_75_PREFECT_AT_FIRE:
            f->image_id = image_group(GROUP_FIGURE_PREFECT_WITH_BUCKET) +
                dir + 96 + 8 * (f->image_offset / 2);
            break;
        case FIGURE_ACTION_150_ATTACK:
            if (f->attack_image_offset >= 12) {
                f->image_id = image_group(GROUP_FIGURE_PREFECT) +
                    104 + dir + 8 * ((f->attack_image_offset - 12) / 2);
            } else {
                f->image_id = image_group(GROUP_FIGURE_PREFECT) + 104 + dir;
            }
            break;
        case FIGURE_ACTION_149_CORPSE:
            f->image_id = image_group(GROUP_FIGURE_PREFECT) +
                96 + figure_image_corpse_offset(f);
            break;
        default:
            f->image_id = image_group(GROUP_FIGURE_PREFECT) +
                dir + 8 * f->image_offset;
            break;
    }
}

void figure_worker_action(figure *f)
{
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    f->use_cross_country = 0;
    f->max_roam_length = 384;
    building *b = building_get(f->building_id);
    if (b->state != BUILDING_STATE_IN_USE || b->figure_id != f->id) {
        f->state = FIGURE_STATE_DEAD;
    }
}
