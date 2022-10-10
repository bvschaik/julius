#include "wall.h"

#include "building/building.h"
#include "city/view.h"
#include "core/calc.h"
#include "core/config.h"
#include "core/image.h"
#include "figure/combat.h"
#include "figure/enemy_army.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/properties.h"
#include "figure/route.h"
#include "figuretype/missile.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/road_access.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "sound/effect.h"

#define BALLISTA_RANGE 15
#define WATCHTOWER_RANGE 12

static const int BALLISTA_FIRING_OFFSETS[] = {
    0, 1, 2, 3, 4, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const int TOWER_SENTRY_FIRING_OFFSETS[] = {
    0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void figure_ballista_action(figure *f)
{
    building *b = building_get(f->building_id);
    f->terrain_usage = TERRAIN_USAGE_WALLS;
    f->use_cross_country = 0;
    f->is_ghost = 1;
    f->height_adjusted_ticks = 10;
    f->current_height = 45;

    if (b->state != BUILDING_STATE_IN_USE || b->figure_id4 != f->id) {
        f->state = FIGURE_STATE_DEAD;
    }
    if (b->num_workers <= 0 || b->figure_id <= 0) {
        f->state = FIGURE_STATE_DEAD;
    }
    map_figure_delete(f);
    switch (city_view_orientation()) {
        case DIR_0_TOP: f->x = b->x; f->y = b->y; break;
        case DIR_2_RIGHT: f->x = b->x + 1; f->y = b->y; break;
        case DIR_4_BOTTOM: f->x = b->x + 1; f->y = b->y + 1; break;
        case DIR_6_LEFT: f->x = b->x; f->y = b->y + 1; break;
    }
    f->grid_offset = map_grid_offset(f->x, f->y);
    map_figure_add(f);

    switch (f->action_state) {
        case FIGURE_ACTION_149_CORPSE:
            f->state = FIGURE_STATE_DEAD;
            break;
        case FIGURE_ACTION_180_BALLISTA_CREATED:
            f->wait_ticks++;
            if (f->wait_ticks > 20) {
                f->wait_ticks = 0;
                map_point tile;
                if (figure_combat_get_missile_target_for_soldier(f, BALLISTA_RANGE, &tile)) {
                    f->action_state = FIGURE_ACTION_181_BALLISTA_FIRING;
                    f->wait_ticks_missile = figure_properties_for_type(f->type)->missile_delay;
                }
            }
            break;
        case FIGURE_ACTION_181_BALLISTA_FIRING:
            f->wait_ticks_missile++;
            if (f->wait_ticks_missile > figure_properties_for_type(f->type)->missile_delay) {
                map_point tile;
                if (figure_combat_get_missile_target_for_soldier(f, BALLISTA_RANGE, &tile)) {
                    f->direction = calc_missile_shooter_direction(f->x, f->y, tile.x, tile.y);
                    f->wait_ticks_missile = 0;
                    int figure_id = f->id;
                    figure_create_missile(figure_id, f->x, f->y, tile.x, tile.y, FIGURE_BOLT);
                    figure_get(figure_id);
                    sound_effect_play(SOUND_EFFECT_BALLISTA_SHOOT);
                } else {
                    f->action_state = FIGURE_ACTION_180_BALLISTA_CREATED;
                }
            }
            break;
    }
    int dir = figure_image_direction(f);
    if (f->action_state == FIGURE_ACTION_181_BALLISTA_FIRING) {
        f->image_id = image_group(GROUP_FIGURE_BALLISTA) + dir +
            8 * BALLISTA_FIRING_OFFSETS[f->wait_ticks_missile / 4];
    } else {
        f->image_id = image_group(GROUP_FIGURE_BALLISTA) + dir;
    }
}

static void tower_sentry_pick_target(figure *f)
{
    if (enemy_army_total_enemy_formations() <= 0) {
        return;
    }
    if (f->action_state == FIGURE_ACTION_150_ATTACK ||
        f->action_state == FIGURE_ACTION_149_CORPSE ||
        f->action_state == FIGURE_ACTION_174_TOWER_SENTRY_GOING_TO_TOWER) {
        return;
    }
    if (f->in_building_wait_ticks) {
        return;
    }

    f->wait_ticks_next_target++;
    if (f->wait_ticks_next_target >= 40) {
        f->wait_ticks_next_target = 0;
        map_point tile;
        if (figure_combat_get_missile_target_for_soldier(f, 10, &tile)) {
            f->action_state = FIGURE_ACTION_172_TOWER_SENTRY_FIRING;
            f->destination_x = f->x;
            f->destination_y = f->y;
        }
    }
}

static int tower_sentry_init_patrol(building *b, int *x_tile, int *y_tile)
{
    int dir = b->figure_roam_direction;
    int x = b->x;
    int y = b->y;
    switch (dir) {
        case DIR_0_TOP: y -= 8; break;
        case DIR_2_RIGHT: x += 8; break;
        case DIR_4_BOTTOM: y += 8; break;
        case DIR_6_LEFT: x -= 8; break;
    }
    map_grid_bound(&x, &y);

    if (map_routing_wall_tile_in_radius(x, y, 6, x_tile, y_tile)) {
        b->figure_roam_direction += 2;
        if (b->figure_roam_direction > 6) {
            b->figure_roam_direction = 0;
        }
        return 1;
    }
    for (int i = 0; i < 4; i++) {
        dir = b->figure_roam_direction;
        b->figure_roam_direction += 2;
        if (b->figure_roam_direction > 6) {
            b->figure_roam_direction = 0;
        }
        x = b->x;
        y = b->y;
        switch (dir) {
            case DIR_0_TOP: y -= 3; break;
            case DIR_2_RIGHT: x += 3; break;
            case DIR_4_BOTTOM: y += 3; break;
            case DIR_6_LEFT: x -= 3; break;
        }
        map_grid_bound(&x, &y);
        if (map_routing_wall_tile_in_radius(x, y, 6, x_tile, y_tile)) {
            return 1;
        }
    }
    return 0;
}

static void figure_watchtower_archer_spawn(building *b)
{
    if (b->figure_id4 || b->type != BUILDING_WATCHTOWER) {
        return;
    }
    figure *f = figure_create(FIGURE_WATCHTOWER_ARCHER, b->x, b->y, DIR_0_TOP);
    f->building_id = b->id;
    f->action_state = FIGURE_ACTION_223_ARCHER_GUARDING;
    b->figure_id4 = f->id;
}

void figure_tower_sentry_set_image(figure *f)
{
    int dir = figure_image_direction(f);
    if (f->action_state == FIGURE_ACTION_149_CORPSE) {
        f->image_id = image_group(GROUP_FIGURE_TOWER_SENTRY) +
            136 + figure_image_corpse_offset(f);
    } else if (f->action_state == FIGURE_ACTION_172_TOWER_SENTRY_FIRING) {
        f->image_id = image_group(GROUP_FIGURE_TOWER_SENTRY) +
            dir + 96 + 8 * TOWER_SENTRY_FIRING_OFFSETS[f->wait_ticks_missile / 2];
    } else if (f->action_state == FIGURE_ACTION_225_WATCHMAN_SHOOTING) {
        dir = figure_image_normalize_direction(f->attack_direction);
        f->image_id = image_group(GROUP_FIGURE_TOWER_SENTRY) +
            dir + 96 + 8 * TOWER_SENTRY_FIRING_OFFSETS[f->wait_ticks_missile / 2];
    } else if (f->action_state == FIGURE_ACTION_150_ATTACK) {
        int image_id = image_group(GROUP_FIGURE_TOWER_SENTRY);
        if (f->attack_image_offset < 16) {
            f->image_id = image_id + 96 + dir;
        } else {
            f->image_id = image_id + 96 + dir + 8 * ((f->attack_image_offset - 16) / 2);
        }
    } else {
        f->image_id = image_group(GROUP_FIGURE_TOWER_SENTRY) +
            dir + 8 * f->image_offset;
    }
}

void figure_tower_sentry_action(figure *f)
{
    building *b = building_get(f->building_id);
    f->terrain_usage = TERRAIN_USAGE_WALLS;
    f->use_cross_country = 0;
    f->is_ghost = 1;
    if (f->action_state != FIGURE_ACTION_174_TOWER_SENTRY_GOING_TO_TOWER) {
        f->height_adjusted_ticks = 10;
    }
    f->max_roam_length = 800;
    if (b->state != BUILDING_STATE_IN_USE || b->figure_id != f->id) {
        f->state = FIGURE_STATE_DEAD;
    }
    figure_image_increase_offset(f, 12);

    tower_sentry_pick_target(f);
    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_170_TOWER_SENTRY_AT_REST:

            if (b->type != BUILDING_TOWER) {
                f->state = FIGURE_STATE_DEAD;
            }

            f->image_offset = 0;
            f->wait_ticks++;
            if (f->wait_ticks > 40) {
                f->wait_ticks = 0;
                int x_tile, y_tile;
                if (tower_sentry_init_patrol(b, &x_tile, &y_tile)) {
                    f->action_state = FIGURE_ACTION_171_TOWER_SENTRY_PATROLLING;
                    f->destination_x = x_tile;
                    f->destination_y = y_tile;
                    figure_route_remove(f);
                } else {
                    f->state = FIGURE_STATE_DEAD;            
                }
            }
            break;
        case FIGURE_ACTION_171_TOWER_SENTRY_PATROLLING:
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_173_TOWER_SENTRY_RETURNING;
                f->destination_x = f->source_x;
                f->destination_y = f->source_y;
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->action_state = FIGURE_ACTION_170_TOWER_SENTRY_AT_REST;
            }
            break;
        case FIGURE_ACTION_172_TOWER_SENTRY_FIRING:
            figure_movement_move_ticks_tower_sentry(f, 1);
            f->wait_ticks_missile++;
            if (f->wait_ticks_missile > figure_properties_for_type(f->type)->missile_delay) {
                map_point tile;
                if (figure_combat_get_missile_target_for_soldier(f, 10, &tile)) {
                    f->direction = calc_missile_shooter_direction(f->x, f->y, tile.x, tile.y);
                    f->wait_ticks_missile = 0;
                    int figure_id = f->id;
                    figure_create_missile(f->id, f->x, f->y, tile.x, tile.y, FIGURE_JAVELIN);
                    f = figure_get(figure_id);
                } else {
                    f->action_state = FIGURE_ACTION_173_TOWER_SENTRY_RETURNING;
                    f->destination_x = f->source_x;
                    f->destination_y = f->source_y;
                    figure_route_remove(f);
                }
            }
            break;
        case FIGURE_ACTION_173_TOWER_SENTRY_RETURNING:
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_170_TOWER_SENTRY_AT_REST;
            } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_174_TOWER_SENTRY_GOING_TO_TOWER:
            f->terrain_usage = TERRAIN_USAGE_ROADS_HIGHWAY;
            if (config_get(CONFIG_GP_CH_TOWER_SENTRIES_GO_OFFROAD)) {
                f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS_HIGHWAY;
            }

            f->is_ghost = 0;
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                if (b->type == BUILDING_WATCHTOWER) {
                    figure_watchtower_archer_spawn(b);
                    figure_route_remove(f);
                    f->state = FIGURE_STATE_DEAD;
                } else { // if Tower
                    map_figure_delete(f);
                    f->source_x = f->x = b->x;
                    f->source_y = f->y = b->y;
                    f->grid_offset = map_grid_offset(f->x, f->y);
                    map_figure_add(f);
                    f->action_state = FIGURE_ACTION_170_TOWER_SENTRY_AT_REST;
                    figure_route_remove(f);
                }
            } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
    }
    if (map_terrain_is(f->grid_offset, TERRAIN_WALL)) {
        f->current_height = 18;
    } else if (map_terrain_is(f->grid_offset, TERRAIN_GATEHOUSE)) {
        f->in_building_wait_ticks = 24;
    }
    if (f->in_building_wait_ticks) {
        f->in_building_wait_ticks--;
        f->height_adjusted_ticks = 0;
    }
    figure_tower_sentry_set_image(f);
}

void figure_tower_sentry_reroute(void)
{
    for (int i = 1; i < figure_count(); i++) {
        figure *f = figure_get(i);
        if (f->type != FIGURE_TOWER_SENTRY || map_routing_is_wall_passable(f->grid_offset)) {
            continue;
        }
        if (f->action_state == FIGURE_ACTION_174_TOWER_SENTRY_GOING_TO_TOWER ||
            (f->action_state == FIGURE_ACTION_150_ATTACK && f->action_state_before_attack == FIGURE_ACTION_174_TOWER_SENTRY_GOING_TO_TOWER)) {
            continue;
        }
        // tower sentry got off wall due to rotation
        int x_tile, y_tile;
        if (map_routing_wall_tile_in_radius(f->x, f->y, 2, &x_tile, &y_tile)) {
            figure_route_remove(f);
            f->progress_on_tile = 0;
            map_figure_delete(f);
            f->previous_tile_x = f->x = x_tile;
            f->previous_tile_y = f->y = y_tile;
            f->cross_country_x = 15 * x_tile;
            f->cross_country_y = 15 * y_tile;
            f->grid_offset = map_grid_offset(x_tile, y_tile);
            map_figure_add(f);
            f->action_state = FIGURE_ACTION_173_TOWER_SENTRY_RETURNING;
            f->destination_x = f->source_x;
            f->destination_y = f->source_y;
        } else {
            // Teleport back to tower
            map_figure_delete(f);
            building *b = building_get(f->building_id);
            f->source_x = f->x = b->x;
            f->source_y = f->y = b->y;
            f->grid_offset = map_grid_offset(f->x, f->y);
            map_figure_add(f);
            f->action_state = FIGURE_ACTION_170_TOWER_SENTRY_AT_REST;
            figure_route_remove(f);
        }
    }
}

static void watchman_pick_target(figure *f)
{
    if (f->action_state == FIGURE_ACTION_150_ATTACK ||
        f->action_state == FIGURE_ACTION_149_CORPSE) {
        return;
    }
    f->wait_ticks_next_target++;
    if (f->wait_ticks_next_target >= 40) {
        f->wait_ticks_next_target = 0;
        map_point tile;
        if (figure_combat_get_missile_target_for_soldier(f, 10, &tile)) {
            f->action_state = FIGURE_ACTION_225_WATCHMAN_SHOOTING;
        }
    }
}


void figure_watchman_action(figure *f)
{
    building *b = building_get(f->building_id);

    f->terrain_usage = TERRAIN_USAGE_ROADS;
    f->use_cross_country = 0;
    f->max_roam_length = 640;
    if (b->state != BUILDING_STATE_IN_USE || (b->figure_id != f->id && b->figure_id2 != f->id)) {
        f->state = FIGURE_STATE_DEAD;
    }
    figure_image_increase_offset(f, 12);
    watchman_pick_target(f);
    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_220_WATCHMAN_PATROL_INITIATE:
            f->roam_length = 0;
            figure_movement_init_roaming(f);
            if (b->figure_id2 == f->id) { // Skip one roaming cycle for the second Watchman, so they go in the opposite directions
                figure_movement_init_roaming(f);
            }
            f->action_state = FIGURE_ACTION_221_WATCHMAN_PATROLLING;
            break;
        case FIGURE_ACTION_221_WATCHMAN_PATROLLING:
            f->is_ghost = 0;
            f->roam_length++;
            if (f->roam_length >= f->max_roam_length) {
                int x_road, y_road;
                if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                    f->action_state = FIGURE_ACTION_222_WATCHMAN_RETURNING;
                    f->destination_x = x_road;
                    f->destination_y = y_road;
                    figure_route_remove(f);
                } else {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            figure_movement_roam_ticks(f, 1);
            break;
        case FIGURE_ACTION_222_WATCHMAN_RETURNING:
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST || f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_225_WATCHMAN_SHOOTING:
            f->wait_ticks_missile++;
            if (f->wait_ticks_missile > figure_properties_for_type(f->type)->missile_delay) {
                map_point tile;
                if (figure_combat_get_missile_target_for_soldier(f, 10, &tile)) {
                    f->attack_direction = calc_missile_shooter_direction(f->x, f->y, tile.x, tile.y);
                    f->wait_ticks_missile = 0;
                    int figure_id = f->id;
                    figure_create_missile(f->id, f->x, f->y, tile.x, tile.y, FIGURE_JAVELIN);
                    f = figure_get(figure_id);
                } else {
                    f->action_state = FIGURE_ACTION_221_WATCHMAN_PATROLLING;
                }
            }
            break;
    }
    figure_tower_sentry_set_image(f);
}

void figure_watchtower_archer_action(figure *f)
{
    building *b = building_get(f->building_id);
    if (b->state != BUILDING_STATE_IN_USE || b->figure_id4 != f->id) {
        f->state = FIGURE_STATE_DEAD;
    }
    switch (f->action_state) {
        case FIGURE_ACTION_149_CORPSE:
            f->state = FIGURE_STATE_DEAD;
            break;
        case FIGURE_ACTION_223_ARCHER_GUARDING:
            f->wait_ticks++;
            if (f->wait_ticks > 20) {
                f->wait_ticks = 0;
                map_point tile;
                if (figure_combat_get_missile_target_for_soldier(f, WATCHTOWER_RANGE, &tile)) {
                    f->action_state = FIGURE_ACTION_224_ARCHER_SHOOTING;
                    f->wait_ticks_missile = figure_properties_for_type(f->type)->missile_delay;
                }
            }
            break;
        case FIGURE_ACTION_224_ARCHER_SHOOTING:
            f->wait_ticks_missile++;
            if (f->wait_ticks_missile > figure_properties_for_type(f->type)->missile_delay) {
                map_point tile;
                if (figure_combat_get_missile_target_for_soldier(f, WATCHTOWER_RANGE, &tile)) {
                    f->direction = calc_missile_shooter_direction(f->x, f->y, tile.x, tile.y);
                    f->wait_ticks_missile = 0;
                    figure_create_missile(f->id, f->x, f->y, tile.x, tile.y, FIGURE_FRIENDLY_ARROW);
                    sound_effect_play(SOUND_EFFECT_ARROW);
                } else {
                    f->action_state = FIGURE_ACTION_223_ARCHER_GUARDING;
                }
            }
            break;
    }
}


void figure_kill_tower_sentries_at(int x, int y)
{
    for (int i = 0; i < figure_count(); i++) {
        figure *f = figure_get(i);
        if (!figure_is_dead(f) && f->type == FIGURE_TOWER_SENTRY) {
            if (calc_maximum_distance(f->x, f->y, x, y) <= 1) {
                f->state = FIGURE_STATE_DEAD;
            }
        }
    }
}
