#include "movement.h"

#include "building/building.h"
#include "building/destruction.h"
#include "building/roadblock.h"
#include "core/calc.h"
#include "figure/combat.h"
#include "figure/route.h"
#include "figure/service.h"
#include "game/time.h"
#include "map/bridge.h"
#include "map/building.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/property.h"
#include "map/random.h"
#include "map/road_access.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"

static void advance_tick(figure *f)
{
    switch (f->direction) {
        case DIR_0_TOP:
            f->cross_country_y--;
            break;
        case DIR_1_TOP_RIGHT:
            f->cross_country_x++;
            f->cross_country_y--;
            break;
        case DIR_2_RIGHT:
            f->cross_country_x++;
            break;
        case DIR_3_BOTTOM_RIGHT:
            f->cross_country_x++;
            f->cross_country_y++;
            break;
        case DIR_4_BOTTOM:
            f->cross_country_y++;
            break;
        case DIR_5_BOTTOM_LEFT:
            f->cross_country_x--;
            f->cross_country_y++;
            break;
        case DIR_6_LEFT:
            f->cross_country_x--;
            break;
        case DIR_7_TOP_LEFT:
            f->cross_country_x--;
            f->cross_country_y--;
            break;
        default:
            break;
    }
    if (f->height_adjusted_ticks) {
        f->height_adjusted_ticks--;
        if (f->height_adjusted_ticks > 0) {
            f->is_ghost = 1;
            if (f->current_height < f->target_height) {
                f->current_height++;
            }
            if (f->current_height > f->target_height) {
                f->current_height--;
            }
        } else {
            f->is_ghost = 0;
        }
    } else {
        if (f->current_height) {
            f->current_height--;
        }
    }
}

static void set_target_height_bridge(figure *f)
{
    f->height_adjusted_ticks = 18;
    f->target_height = map_bridge_height(f->grid_offset);
}

static roadblock_permission get_permission_for_figure_type(figure* f)
{
    switch (f->type) {
    case FIGURE_ENGINEER:
    case FIGURE_PREFECT:
        return PERMISSION_MAINTENANCE;
        break;
    case FIGURE_PRIEST:
        return PERMISSION_PRIEST;
        break;
    case FIGURE_MARKET_TRADER:
        return PERMISSION_MARKET;
        break;
    case FIGURE_GLADIATOR:
    case FIGURE_CHARIOTEER:
    case FIGURE_ACTOR:
    case FIGURE_LION_TAMER:
        return PERMISSION_ENTERTAINER;
        break;
    default:
        return PERMISSION_NONE;
        break;
    }
}

static void move_to_next_tile(figure *f)
{
    int old_x = f->x;
    int old_y = f->y;
    map_figure_delete(f);
    switch (f->direction) {
        default:
            return;
        case DIR_0_TOP:
            f->y--;
            break;
        case DIR_1_TOP_RIGHT:
            f->x++; f->y--;
            break;
        case DIR_2_RIGHT:
            f->x++;
            break;
        case DIR_3_BOTTOM_RIGHT:
            f->x++; f->y++;
            break;
        case DIR_4_BOTTOM:
            f->y++;
            break;
        case DIR_5_BOTTOM_LEFT:
            f->x--; f->y++;
            break;
        case DIR_6_LEFT:
            f->x--;
            break;
        case DIR_7_TOP_LEFT:
            f->x--; f->y--;
            break;
    }
    f->grid_offset += map_grid_direction_delta(f->direction);
    map_figure_add(f);
    if (map_terrain_is(f->grid_offset, TERRAIN_ROAD)) {
        f->is_on_road = 1;
        if (map_terrain_is(f->grid_offset, TERRAIN_WATER)) { // bridge
            set_target_height_bridge(f);
        }
    } else {
        f->is_on_road = 0;
    }
    figure_combat_attack_figure_at(f, f->grid_offset);
    f->previous_tile_x = old_x;
    f->previous_tile_y = old_y;
}

static void set_next_route_tile_direction(figure *f)
{
    if (f->routing_path_id > 0) {
        if (f->routing_path_current_tile < f->routing_path_length) {
            f->direction = figure_route_get_direction(f->routing_path_id, f->routing_path_current_tile);
        } else {
            figure_route_remove(f);
            f->direction = DIR_FIGURE_AT_DESTINATION;
        }
    } else { // should be at destination
        f->direction = calc_general_direction(f->x, f->y, f->destination_x, f->destination_y);
        if (f->direction != DIR_FIGURE_AT_DESTINATION) {
            f->direction = DIR_FIGURE_LOST;
        }
    }
}

static void advance_route_tile(figure *f, int roaming_enabled)
{
    if (f->direction >= 8) {
        return;
    }
    int target_grid_offset = f->grid_offset + map_grid_direction_delta(f->direction);
    if (f->is_boat) {
        if (!map_terrain_is(target_grid_offset, TERRAIN_WATER)) {
            f->direction = DIR_FIGURE_REROUTE;
        }
    } else if (f->terrain_usage == TERRAIN_USAGE_ENEMY) {
        if (!map_routing_noncitizen_is_passable(target_grid_offset)) {
            f->direction = DIR_FIGURE_REROUTE;
        } else if (map_routing_is_destroyable(target_grid_offset)) {
            int cause_damage = 1;
            int max_damage = 0;
            switch (map_routing_get_destroyable(target_grid_offset)) {
                case DESTROYABLE_BUILDING:
                    max_damage = 10;
                    break;
                case DESTROYABLE_AQUEDUCT_GARDEN:
                    if (map_terrain_is(target_grid_offset, TERRAIN_GARDEN | TERRAIN_ACCESS_RAMP | TERRAIN_RUBBLE)) {
                        cause_damage = 0;
                    } else {
                        max_damage = 10;
                    }
                    break;
                case DESTROYABLE_WALL:
                    max_damage = 200;
                    break;
                case DESTROYABLE_GATEHOUSE:
                    max_damage = 150;
                    break;
            }
            if (cause_damage) {
                f->attack_direction = f->direction;
                f->direction = DIR_FIGURE_ATTACK;
                if (!(game_time_tick() & 3)) {
                    building_destroy_increase_enemy_damage(target_grid_offset, max_damage);
                }
            }
        }
    } else if (f->terrain_usage == TERRAIN_USAGE_WALLS) {
        if (!map_routing_is_wall_passable(target_grid_offset)) {
            f->direction = DIR_FIGURE_REROUTE;
        }
    } else if (map_terrain_is(target_grid_offset, TERRAIN_ROAD | TERRAIN_ACCESS_RAMP)) {
        if (roaming_enabled && map_terrain_is(target_grid_offset, TERRAIN_BUILDING)) {
            building* b = building_get(map_building_at(target_grid_offset));
            if (b->type == BUILDING_GATEHOUSE) {
                // do not allow roaming through gatehouse
                f->direction = DIR_FIGURE_REROUTE;
            }
            if (b->type == BUILDING_ROADBLOCK) {
                // do not allow roaming through roadblock
                int permission = get_permission_for_figure_type(f);
                if (!building_roadblock_get_permission(permission, b)) {
                    f->direction = DIR_FIGURE_REROUTE;
                }
            }
        }
    } else if (map_terrain_is(target_grid_offset, TERRAIN_BUILDING)) {
        int type = building_get(map_building_at(target_grid_offset))->type;
        switch (type) {
            case BUILDING_WAREHOUSE:
            case BUILDING_GRANARY:
            case BUILDING_TRIUMPHAL_ARCH:
            case BUILDING_FORT_GROUND:
                break; // OK to walk
            default:
                f->direction = DIR_FIGURE_REROUTE;
        }
    } else if (map_terrain_is(target_grid_offset, TERRAIN_IMPASSABLE)) {
        f->direction = DIR_FIGURE_REROUTE;
    }
}


static void walk_ticks(figure *f, int num_ticks, int roaming_enabled)
{
    while (num_ticks > 0) {
        num_ticks--;
        f->progress_on_tile++;
        if (f->progress_on_tile < 15) {
            advance_tick(f);
        } else {
            figure_service_provide_coverage(f);
            f->progress_on_tile = 15;
            if (f->routing_path_id <= 0) {
                figure_route_add(f);
            }
            set_next_route_tile_direction(f);
            advance_route_tile(f, roaming_enabled);
            if (f->direction >= 8) {
                break;
            }
            f->routing_path_current_tile++;
            f->previous_tile_direction = f->direction;
            f->progress_on_tile = 0;
            move_to_next_tile(f);
            advance_tick(f);
        }
    }
}

void figure_movement_init_roaming(figure *f)
{
    building *b = building_get(f->building_id);
    f->progress_on_tile = 15;
    f->roam_choose_destination = 0;
    f->roam_ticks_until_next_turn = -1;
    f->roam_turn_direction = 2;
    int roam_dir = b->figure_roam_direction;
    b->figure_roam_direction += 2;
    if (b->figure_roam_direction > 6) {
        b->figure_roam_direction = 0;
    }
    int x = b->x;
    int y = b->y;
    switch (roam_dir) {
        case DIR_0_TOP: y -= 8; break;
        case DIR_2_RIGHT: x += 8; break;
        case DIR_4_BOTTOM: y += 8; break;
        case DIR_6_LEFT: x -= 8; break;
    }
    map_grid_bound(&x, &y);
    int x_road, y_road;
    if (map_closest_road_within_radius(x, y, 1, 6, &x_road, &y_road)) {
        f->destination_x = x_road;
        f->destination_y = y_road;
    } else {
        f->roam_choose_destination = 1;
    }
}

static void roam_set_direction(figure *f)
{
    int grid_offset = map_grid_offset(f->x, f->y);
    int direction = calc_general_direction(f->x, f->y, f->destination_x, f->destination_y);
    if (direction >= 8) {
        direction = 0;
    }
    int road_offset_dir1 = 0;
    int road_dir1 = 0;
    for (int i = 0, dir = direction; i < 8; i++) {
        if (dir % 2 == 0 && map_terrain_is(grid_offset + map_grid_direction_delta(dir), TERRAIN_ROAD)) {
            road_dir1 = dir;
            break;
        }
        dir++;
        if (dir > 7) dir = 0;
        road_offset_dir1++;
    }
    int road_offset_dir2 = 0;
    int road_dir2 = 0;
    for (int i = 0, dir = direction; i < 8; i++) {
        if (dir % 2 == 0 && map_terrain_is(grid_offset + map_grid_direction_delta(dir), TERRAIN_ROAD)) {
            road_dir2 = dir;
            break;
        }
        dir--;
        if (dir < 0) dir = 7;
        road_offset_dir2++;
    }
    if (road_offset_dir1 <= road_offset_dir2) {
        f->direction = road_dir1;
        f->roam_turn_direction = 2;
    } else {
        f->direction = road_dir2;
        f->roam_turn_direction = -2;
    }
    f->roam_ticks_until_next_turn = 5;
}

void figure_movement_move_ticks(figure *f, int num_ticks)
{
    walk_ticks(f, num_ticks, 0);
}

void figure_movement_move_ticks_tower_sentry(figure *f, int num_ticks)
{
    while (num_ticks > 0) {
        num_ticks--;
        f->progress_on_tile++;
        if (f->progress_on_tile < 15) {
            advance_tick(f);
        } else {
            f->progress_on_tile = 15;
        }
    }
}

void figure_movement_follow_ticks(figure *f, int num_ticks)
{
    const figure *leader = figure_get(f->leading_figure_id);
    if (f->x == f->source_x && f->y == f->source_y) {
        f->is_ghost = 1;
    }
    while (num_ticks > 0) {
        num_ticks--;
        f->progress_on_tile++;
        if (f->progress_on_tile < 15) {
            advance_tick(f);
        } else {
            f->progress_on_tile = 15;
            f->direction = calc_general_direction(f->x, f->y,
                leader->previous_tile_x, leader->previous_tile_y);
            if (f->direction >= 8) {
                break;
            }
            f->previous_tile_direction = f->direction;
            f->progress_on_tile = 0;
            move_to_next_tile(f);
            advance_tick(f);
        }
    }
}

void figure_movement_roam_ticks(figure *f, int num_ticks)
{
    if (f->roam_choose_destination == 0) {
        walk_ticks(f, num_ticks, 1);
        if (f->direction == DIR_FIGURE_AT_DESTINATION) {
            f->roam_choose_destination = 1;
            f->roam_length = 0;
        } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
            f->roam_choose_destination = 1;
        }
        if (f->roam_choose_destination) {
            f->roam_ticks_until_next_turn = 100;
            f->direction = f->previous_tile_direction;
        } else {
            return;
        }
    }
    // no destination: walk to end of tile and pick a direction
    while (num_ticks > 0) {
        num_ticks--;
        f->progress_on_tile++;
        if (f->progress_on_tile < 15) {
            advance_tick(f);
        } else {
            f->progress_on_tile = 15;
            f->roam_random_counter++;
            int came_from_direction = (f->previous_tile_direction + 4) % 8;
            if (figure_service_provide_coverage(f)) {
                return;
            }
            int road_tiles[8];
            int permission = get_permission_for_figure_type(f);
            int adjacent_road_tiles = map_get_adjacent_road_tiles_for_roaming(f->grid_offset, road_tiles, permission);
            if (adjacent_road_tiles == 3 && map_get_diagonal_road_tiles_for_roaming(f->grid_offset, road_tiles) >= 5) {
                // go in the straight direction of a double-wide road
                adjacent_road_tiles = 2;
                if (came_from_direction == DIR_0_TOP || came_from_direction == DIR_4_BOTTOM) {
                    if (road_tiles[0] && road_tiles[4]) {
                        road_tiles[2] = road_tiles[6] = 0;
                    } else {
                        road_tiles[0] = road_tiles[4] = 0;
                    }
                } else {
                    if (road_tiles[2] && road_tiles[6]) {
                        road_tiles[0] = road_tiles[4] = 0;
                    } else {
                        road_tiles[2] = road_tiles[6] = 0;
                    }
                }
            }
            if (adjacent_road_tiles == 4 && map_get_diagonal_road_tiles_for_roaming(f->grid_offset, road_tiles) >= 8) {
                // go straight on when all surrounding tiles are road
                adjacent_road_tiles = 2;
                if (came_from_direction == DIR_0_TOP || came_from_direction == DIR_4_BOTTOM) {
                    road_tiles[2] = road_tiles[6] = 0;
                } else {
                    road_tiles[0] = road_tiles[4] = 0;
                }
            }
            if (adjacent_road_tiles <= 0) {
                f->roam_length = f->max_roam_length; // end roaming walk
                return;
            }
            if (adjacent_road_tiles == 1) {
                int dir = 0;
                do {
                    f->direction = 2 * dir;
                } while (!road_tiles[f->direction] && dir++ < 4);
            } else if (adjacent_road_tiles == 2) {
                if (f->roam_ticks_until_next_turn == -1) {
                    roam_set_direction(f);
                    came_from_direction = -1;
                }
                // 1. continue in the same direction
                // 2. turn in the direction given by roam_turn_direction
                int dir = 0;
                do {
                    if (road_tiles[f->direction] && f->direction != came_from_direction) {
                        break;
                    }
                    f->direction += f->roam_turn_direction;
                    if (f->direction > 6) f->direction = 0;
                    if (f->direction < 0) f->direction = 6;
                } while (dir++ < 4);
            } else { // > 2 road tiles
                f->direction = (f->roam_random_counter + map_random_get(f->grid_offset)) & 6;
                if (!road_tiles[f->direction] || f->direction == came_from_direction) {
                    f->roam_ticks_until_next_turn--;
                    if (f->roam_ticks_until_next_turn <= 0) {
                        roam_set_direction(f);
                        came_from_direction = -1;
                    }
                    int dir = 0;
                    do {
                        if (road_tiles[f->direction] && f->direction != came_from_direction) {
                            break;
                        }
                        f->direction += f->roam_turn_direction;
                        if (f->direction > 6) f->direction = 0;
                        if (f->direction < 0) f->direction = 6;
                    } while (dir++ < 4);
                }
            }
            f->routing_path_current_tile++;
            f->previous_tile_direction = f->direction;
            f->progress_on_tile = 0;
            move_to_next_tile(f);
            advance_tick(f);
        }
    }
}

void figure_movement_advance_attack(figure *f)
{
    if (f->progress_on_tile <= 5) {
        f->progress_on_tile++;
        advance_tick(f);
    }
}

void figure_movement_set_cross_country_direction(figure *f, int x_src, int y_src, int x_dst, int y_dst, int is_missile)
{
    // all x/y are in 1/15th of a tile
    f->cc_destination_x = x_dst;
    f->cc_destination_y = y_dst;
    f->cc_delta_x = (x_src > x_dst) ? (x_src - x_dst) : (x_dst - x_src);
    f->cc_delta_y = (y_src > y_dst) ? (y_src - y_dst) : (y_dst - y_src);
    if (f->cc_delta_x < f->cc_delta_y) {
        f->cc_delta_xy = 2 * f->cc_delta_x - f->cc_delta_y;
    } else if (f->cc_delta_y < f->cc_delta_x) {
        f->cc_delta_xy = 2 * f->cc_delta_y - f->cc_delta_x;
    } else { // equal
        f->cc_delta_xy = 0;
    }
    if (is_missile) {
        f->direction = calc_missile_direction(x_src, y_src, x_dst, y_dst);
    } else {
        f->direction = calc_general_direction(x_src, y_src, x_dst, y_dst);
        if (f->cc_delta_y > 2 * f->cc_delta_x) {
            switch (f->direction) {
                case DIR_1_TOP_RIGHT: case DIR_7_TOP_LEFT: f->direction = DIR_0_TOP; break;
                case DIR_3_BOTTOM_RIGHT: case DIR_5_BOTTOM_LEFT: f->direction = DIR_4_BOTTOM; break;
            }
        }
        if (f->cc_delta_x > 2 * f->cc_delta_y) {
            switch (f->direction) {
                case DIR_1_TOP_RIGHT: case DIR_3_BOTTOM_RIGHT: f->direction = DIR_2_RIGHT; break;
                case DIR_5_BOTTOM_LEFT: case DIR_7_TOP_LEFT: f->direction = DIR_6_LEFT; break;
            }
        }
    }
    if (f->cc_delta_x >= f->cc_delta_y) {
        f->cc_direction = 1;
    } else {
        f->cc_direction = 2;
    }
}

void figure_movement_set_cross_country_destination(figure *f, int x_dst, int y_dst)
{
    f->destination_x = x_dst;
    f->destination_y = y_dst;
    figure_movement_set_cross_country_direction(
        f, f->cross_country_x, f->cross_country_y,
        15 * x_dst, 15 * y_dst, 0);
}

static void cross_country_update_delta(figure *f)
{
    if (f->cc_direction == 1) { // x
        if (f->cc_delta_xy >= 0) {
            f->cc_delta_xy += 2 * (f->cc_delta_y - f->cc_delta_x);
        } else {
            f->cc_delta_xy += 2 * f->cc_delta_y;
        }
        f->cc_delta_x--;
    } else { // y
        if (f->cc_delta_xy >= 0) {
            f->cc_delta_xy += 2 * (f->cc_delta_x - f->cc_delta_y);
        } else {
            f->cc_delta_xy += 2 * f->cc_delta_x;
        }
        f->cc_delta_y--;
    }
}

static void cross_country_advance_x(figure *f)
{
    if (f->cross_country_x < f->cc_destination_x) {
        f->cross_country_x++;
    } else if (f->cross_country_x > f->cc_destination_x) {
        f->cross_country_x--;
    }
}

static void cross_country_advance_y(figure *f)
{
    if (f->cross_country_y < f->cc_destination_y) {
        f->cross_country_y++;
    } else if (f->cross_country_y > f->cc_destination_y) {
        f->cross_country_y--;
    }
}

static void cross_country_advance(figure *f)
{
    cross_country_update_delta(f);
    if (f->cc_direction == 2) { // y
        cross_country_advance_y(f);
        if (f->cc_delta_xy >= 0) {
            f->cc_delta_x--;
            cross_country_advance_x(f);
        }
    } else {
        cross_country_advance_x(f);
        if (f->cc_delta_xy >= 0) {
            f->cc_delta_y--;
            cross_country_advance_y(f);
        }
    }
}

int figure_movement_move_ticks_cross_country(figure *f, int num_ticks)
{
    map_figure_delete(f);
    int is_at_destination = 0;
    while (num_ticks > 0) {
        num_ticks--;
        if (f->missile_damage > 0) {
            f->missile_damage--;
        } else {
            f->missile_damage = 0;
        }
        if (f->cc_delta_x + f->cc_delta_y <= 0) {
            is_at_destination = 1;
            break;
        }
        cross_country_advance(f);
    }
    f->x = f->cross_country_x / 15;
    f->y = f->cross_country_y / 15;
    f->grid_offset = map_grid_offset(f->x, f->y);
    if (map_terrain_is(f->grid_offset, TERRAIN_BUILDING)) {
        f->in_building_wait_ticks = 8;
    } else if (f->in_building_wait_ticks) {
        f->in_building_wait_ticks--;
    }
    map_figure_add(f);
    return is_at_destination;
}

int figure_movement_can_launch_cross_country_missile(int x_src, int y_src, int x_dst, int y_dst)
{
    int height = 0;
    figure *f = figure_get(0); // abuse unused figure 0 as scratch
    f->cross_country_x = 15 * x_src;
    f->cross_country_y = 15 * y_src;
    if (map_terrain_is(map_grid_offset(x_src, y_src), TERRAIN_WALL_OR_GATEHOUSE)) {
        height = 6;
    }
    figure_movement_set_cross_country_direction(f, 15 * x_src, 15 * y_src, 15 * x_dst, 15 * y_dst, 0);

    for (int guard = 0; guard < 1000; guard++) {
        for (int i = 0; i < 8; i++) {
            if (f->cc_delta_x + f->cc_delta_y <= 0) {
                return 1;
            }
            cross_country_advance(f);
        }
        f->x = f->cross_country_x / 15;
        f->y = f->cross_country_y / 15;
        if (height) {
            height--;
        } else {
            int grid_offset = map_grid_offset(f->x, f->y);
            if (map_terrain_is(grid_offset, TERRAIN_WALL | TERRAIN_GATEHOUSE | TERRAIN_TREE)) {
                break;
            }
            if (map_terrain_is(grid_offset, TERRAIN_BUILDING) && map_property_multi_tile_size(grid_offset) > 1) {
                break;
            }
        }
    }
    return 0;
}
