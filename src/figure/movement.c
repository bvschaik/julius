#include "movement.h"

#include "building/building.h"
#include "building/destruction.h"
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
            f->crossCountryY--;
            break;
        case DIR_1_TOP_RIGHT:
            f->crossCountryX++;
            f->crossCountryY--;
            break;
        case DIR_2_RIGHT:
            f->crossCountryX++;
            break;
        case DIR_3_BOTTOM_RIGHT:
            f->crossCountryX++;
            f->crossCountryY++;
            break;
        case DIR_4_BOTTOM:
            f->crossCountryY++;
            break;
        case DIR_5_BOTTOM_LEFT:
            f->crossCountryX--;
            f->crossCountryY++;
            break;
        case DIR_6_LEFT:
            f->crossCountryX--;
            break;
        case DIR_7_TOP_LEFT:
            f->crossCountryX--;
            f->crossCountryY--;
            break;
        default:
            break;
    }
    if (f->heightAdjustedTicks) {
        f->heightAdjustedTicks--;
        if (f->heightAdjustedTicks > 0) {
            f->isGhost = 1;
            if (f->currentHeight < f->targetHeight) {
                f->currentHeight++;
            }
            if (f->currentHeight > f->targetHeight) {
                f->currentHeight--;
            }
        } else {
            f->isGhost = 0;
        }
    } else {
        if (f->currentHeight) {
            f->currentHeight--;
        }
    }
}

static void set_target_height_bridge(figure *f)
{
    f->heightAdjustedTicks = 18;
    f->targetHeight = map_bridge_height(f->gridOffset);
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
            f->gridOffset -= 162;
            break;
        case DIR_1_TOP_RIGHT:
            f->x++; f->y--;
            f->gridOffset -= 161;
            break;
        case DIR_2_RIGHT:
            f->x++;
            f->gridOffset += 1;
            break;
        case DIR_3_BOTTOM_RIGHT:
            f->x++; f->y++;
            f->gridOffset += 163;
            break;
        case DIR_4_BOTTOM:
            f->y++;
            f->gridOffset += 162;
            break;
        case DIR_5_BOTTOM_LEFT:
            f->x--; f->y++;
            f->gridOffset += 161;
            break;
        case DIR_6_LEFT:
            f->x--;
            f->gridOffset -= 1;
            break;
        case DIR_7_TOP_LEFT:
            f->x--; f->y--;
            f->gridOffset -= 163;
            break;
    }
    map_figure_add(f);
    if (map_terrain_is(f->gridOffset, TERRAIN_ROAD)) {
        f->isOnRoad = 1;
        if (map_terrain_is(f->gridOffset, TERRAIN_WATER)) { // bridge
            set_target_height_bridge(f);
        }
    } else {
        f->isOnRoad = 0;
    }
    figure_combat_attack_figure_at(f, f->gridOffset);
    f->previousTileX = old_x;
    f->previousTileY = old_y;
}

static void set_next_route_tile_direction(figure *f)
{
    if (f->routingPathId > 0) {
        if (f->routingPathCurrentTile < f->routingPathLength) {
            f->direction = figure_route_get_direction(f->routingPathId, f->routingPathCurrentTile);
        } else {
            figure_route_remove(f);
            f->direction = DIR_FIGURE_AT_DESTINATION;
        }
    } else { // should be at destination
        f->direction = calc_general_direction(f->x, f->y, f->destinationX, f->destinationY);
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
    int target_grid_offset = f->gridOffset + map_grid_direction_delta(f->direction);
    if (f->isBoat) {
        if (!map_terrain_is(target_grid_offset, TERRAIN_WATER)) {
            f->direction = DIR_FIGURE_REROUTE;
        }
    } else if (f->terrainUsage == TERRAIN_USAGE_ENEMY) {
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
                f->attackDirection = f->direction;
                f->direction = DIR_FIGURE_ATTACK;
                if (!(game_time_tick() & 3)) {
                    building_destroy_increase_enemy_damage(target_grid_offset, max_damage);
                }
            }
        }
    } else if (f->terrainUsage == TERRAIN_USAGE_WALLS) {
        if (!map_routing_is_wall_passable(target_grid_offset)) {
            f->direction = DIR_FIGURE_REROUTE;
        }
    } else if (map_terrain_is(target_grid_offset, TERRAIN_ROAD | TERRAIN_ACCESS_RAMP)) {
        if (roaming_enabled && map_terrain_is(target_grid_offset, TERRAIN_BUILDING)) {
            if (building_get(map_building_at(target_grid_offset))->type == BUILDING_GATEHOUSE) {
                // do not allow roaming through gatehouse
                f->direction = DIR_FIGURE_REROUTE;
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
        f->progressOnTile++;
        if (f->progressOnTile < 15) {
            advance_tick(f);
        } else {
            figure_service_provide_coverage(f);
            f->progressOnTile = 15;
            if (f->routingPathId <= 0) {
                figure_route_add(f);
            }
            set_next_route_tile_direction(f);
            advance_route_tile(f, roaming_enabled);
            if (f->direction >= 8) {
                break;
            }
            f->routingPathCurrentTile++;
            f->previousTileDirection = f->direction;
            f->progressOnTile = 0;
            move_to_next_tile(f);
            advance_tick(f);
        }
    }
}

void figure_movement_init_roaming(figure *f)
{
    building *b = building_get(f->buildingId);
    f->progressOnTile = 15;
    f->roamChooseDestination = 0;
    f->roamTicksUntilNextTurn = -1;
    f->roamTurnDirection = 2;
    int roam_dir = b->figureRoamDirection;
    b->figureRoamDirection += 2;
    if (b->figureRoamDirection > 6) {
        b->figureRoamDirection = 0;
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
        f->destinationX = x_road;
        f->destinationY = y_road;
    } else {
        f->roamChooseDestination = 1;
    }
}

static void roam_set_direction(figure *f)
{
    int grid_offset = map_grid_offset(f->x, f->y);
    int direction = calc_general_direction(f->x, f->y, f->destinationX, f->destinationY);
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
        f->roamTurnDirection = 2;
    } else {
        f->direction = road_dir2;
        f->roamTurnDirection = -2;
    }
    f->roamTicksUntilNextTurn = 5;
}

void figure_movement_move_ticks(figure *f, int num_ticks)
{
    walk_ticks(f, num_ticks, 0);
}

void figure_movement_move_ticks_tower_sentry(figure *f, int num_ticks)
{
    while (num_ticks > 0) {
        num_ticks--;
        f->progressOnTile++;
        if (f->progressOnTile < 15) {
            advance_tick(f);
        } else {
            f->progressOnTile = 15;
        }
    }
}

void figure_movement_follow_ticks(figure *f, int num_ticks)
{
    const figure *leader = figure_get(f->inFrontFigureId);
    if (f->x == f->sourceX && f->y == f->sourceY) {
        f->isGhost = 1;
    }
    while (num_ticks > 0) {
        num_ticks--;
        f->progressOnTile++;
        if (f->progressOnTile < 15) {
            advance_tick(f);
        } else {
            f->progressOnTile = 15;
            f->direction = calc_general_direction(f->x, f->y,
                leader->previousTileX, leader->previousTileY);
            if (f->direction >= 8) {
                break;
            }
            f->previousTileDirection = f->direction;
            f->progressOnTile = 0;
            move_to_next_tile(f);
            advance_tick(f);
        }
    }
}

void figure_movement_roam_ticks(figure *f, int num_ticks)
{
    if (f->roamChooseDestination == 0) {
        walk_ticks(f, num_ticks, 1);
        if (f->direction == DIR_FIGURE_AT_DESTINATION) {
            f->roamChooseDestination = 1;
            f->roamLength = 0;
        } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
            f->roamChooseDestination = 1;
        }
        if (f->roamChooseDestination) {
            f->roamTicksUntilNextTurn = 100;
            f->direction = f->previousTileDirection;
        } else {
            return;
        }
    }
    // no destination: walk to end of tile and pick a direction
    while (num_ticks > 0) {
        num_ticks--;
        f->progressOnTile++;
        if (f->progressOnTile < 15) {
            advance_tick(f);
        } else {
            f->progressOnTile = 15;
            f->roamRandomCounter++;
            int came_from_direction = (f->previousTileDirection + 4) % 8;
            if (figure_service_provide_coverage(f)) {
                return;
            }
            int road_tiles[8];
            int adjacent_road_tiles = map_get_adjacent_road_tiles_for_roaming(f->gridOffset, road_tiles);
            if (adjacent_road_tiles == 3 && map_get_diagonal_road_tiles_for_roaming(f->gridOffset, road_tiles) >= 5) {
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
            if (adjacent_road_tiles == 4 && map_get_diagonal_road_tiles_for_roaming(f->gridOffset, road_tiles) >= 8) {
                // go straight on when all surrounding tiles are road
                adjacent_road_tiles = 2;
                if (came_from_direction == DIR_0_TOP || came_from_direction == DIR_4_BOTTOM) {
                    road_tiles[2] = road_tiles[6] = 0;
                } else {
                    road_tiles[0] = road_tiles[4] = 0;
                }
            }
            if (adjacent_road_tiles <= 0) {
                f->roamLength = f->maxRoamLength; // end roaming walk
                return;
            }
            if (adjacent_road_tiles == 1) {
                int dir = 0;
                do {
                    f->direction = 2 * dir;
                } while (!road_tiles[f->direction] && dir++ < 4);
            } else if (adjacent_road_tiles == 2) {
                if (f->roamTicksUntilNextTurn == -1) {
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
                    f->direction += f->roamTurnDirection;
                    if (f->direction > 6) f->direction = 0;
                    if (f->direction < 0) f->direction = 6;
                } while (dir++ < 4);
            } else { // > 2 road tiles
                f->direction = (f->roamRandomCounter + map_random_get(f->gridOffset)) & 6;
                if (!road_tiles[f->direction] || f->direction == came_from_direction) {
                    f->roamTicksUntilNextTurn--;
                    if (f->roamTicksUntilNextTurn <= 0) {
                        roam_set_direction(f);
                        came_from_direction = -1;
                    }
                    int dir = 0;
                    do {
                        if (road_tiles[f->direction] && f->direction != came_from_direction) {
                            break;
                        }
                        f->direction += f->roamTurnDirection;
                        if (f->direction > 6) f->direction = 0;
                        if (f->direction < 0) f->direction = 6;
                    } while (dir++ < 4);
                }
            }
            f->routingPathCurrentTile++;
            f->previousTileDirection = f->direction;
            f->progressOnTile = 0;
            move_to_next_tile(f);
            advance_tick(f);
        }
    }
}

void figure_movement_advance_attack(figure *f)
{
    if (f->progressOnTile <= 5) {
        f->progressOnTile++;
        advance_tick(f);
    }
}

void figure_movement_set_cross_country_direction(figure *f, int x_src, int y_src, int x_dst, int y_dst, int is_missile)
{
    // all x/y are in 1/15th of a tile
    f->ccDestinationX = x_dst;
    f->ccDestinationY = y_dst;
    f->ccDeltaX = (x_src > x_dst) ? (x_src - x_dst) : (x_dst - x_src);
    f->ccDeltaY = (y_src > y_dst) ? (y_src - y_dst) : (y_dst - y_src);
    if (f->ccDeltaX < f->ccDeltaY) {
        f->ccDeltaXY = 2 * f->ccDeltaX - f->ccDeltaY;
    } else if (f->ccDeltaY < f->ccDeltaX) {
        f->ccDeltaXY = 2 * f->ccDeltaY - f->ccDeltaX;
    } else { // equal
        f->ccDeltaXY = 0;
    }
    if (is_missile) {
        f->direction = calc_missile_direction(x_src, y_src, x_dst, y_dst);
    } else {
        f->direction = calc_general_direction(x_src, y_src, x_dst, y_dst);
        if (f->ccDeltaY > 2 * f->ccDeltaX) {
            switch (f->direction) {
                case DIR_1_TOP_RIGHT: case DIR_7_TOP_LEFT: f->direction = DIR_0_TOP; break;
                case DIR_3_BOTTOM_RIGHT: case DIR_5_BOTTOM_LEFT: f->direction = DIR_4_BOTTOM; break;
            }
        }
        if (f->ccDeltaX > 2 * f->ccDeltaY) {
            switch (f->direction) {
                case DIR_1_TOP_RIGHT: case DIR_3_BOTTOM_RIGHT: f->direction = DIR_2_RIGHT; break;
                case DIR_5_BOTTOM_LEFT: case DIR_7_TOP_LEFT: f->direction = DIR_6_LEFT; break;
            }
        }
    }
    if (f->ccDeltaX >= f->ccDeltaY) {
        f->ccDirection = 1;
    } else {
        f->ccDirection = 2;
    }
}

void figure_movement_set_cross_country_destination(figure *f, int x_dst, int y_dst)
{
    f->destinationX = x_dst;
    f->destinationY = y_dst;
    figure_movement_set_cross_country_direction(
        f, f->crossCountryX, f->crossCountryY,
        15 * x_dst, 15 * y_dst, 0);
}

static void cross_country_update_delta(figure *f)
{
    if (f->ccDirection == 1) { // x
        if (f->ccDeltaXY >= 0) {
            f->ccDeltaXY += 2 * (f->ccDeltaY - f->ccDeltaX);
        } else {
            f->ccDeltaXY += 2 * f->ccDeltaY;
        }
        f->ccDeltaX--;
    } else { // y
        if (f->ccDeltaXY >= 0) {
            f->ccDeltaXY += 2 * (f->ccDeltaX - f->ccDeltaY);
        } else {
            f->ccDeltaXY += 2 * f->ccDeltaX;
        }
        f->ccDeltaY--;
    }
}

static void cross_country_advance_x(figure *f)
{
    if (f->crossCountryX < f->ccDestinationX) {
        f->crossCountryX++;
    } else if (f->crossCountryX > f->ccDestinationX) {
        f->crossCountryX--;
    }
}

static void cross_country_advance_y(figure *f)
{
    if (f->crossCountryY < f->ccDestinationY) {
        f->crossCountryY++;
    } else if (f->crossCountryY > f->ccDestinationY) {
        f->crossCountryY--;
    }
}

static void cross_country_advance(figure *f)
{
    cross_country_update_delta(f);
    if (f->ccDirection == 2) { // y
        cross_country_advance_y(f);
        if (f->ccDeltaXY >= 0) {
            f->ccDeltaX--;
            cross_country_advance_x(f);
        }
    } else {
        cross_country_advance_x(f);
        if (f->ccDeltaXY >= 0) {
            f->ccDeltaY--;
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
        if (f->missileDamage > 0) {
            f->missileDamage--;
        } else {
            f->missileDamage = 0;
        }
        if (f->ccDeltaX + f->ccDeltaY <= 0) {
            is_at_destination = 1;
            break;
        }
        cross_country_advance(f);
    }
    f->x = f->crossCountryX / 15;
    f->y = f->crossCountryY / 15;
    f->gridOffset = map_grid_offset(f->x, f->y);
    if (map_terrain_is(f->gridOffset, TERRAIN_BUILDING)) {
        f->inBuildingWaitTicks = 8;
    } else if (f->inBuildingWaitTicks) {
        f->inBuildingWaitTicks--;
    }
    map_figure_add(f);
    return is_at_destination;
}

int figure_movement_can_launch_cross_country_missile(int x_src, int y_src, int x_dst, int y_dst)
{
    int height = 0;
    figure *f = figure_get(0); // abuse unused figure 0 as scratch
    f->crossCountryX = 15 * x_src;
    f->crossCountryY = 15 * y_src;
    if (map_terrain_is(map_grid_offset(x_src, y_src), TERRAIN_WALL_OR_GATEHOUSE)) {
        height = 6;
    }
    figure_movement_set_cross_country_direction(f, 15 * x_src, 15 * y_src, 15 * x_dst, 15 * y_dst, 0);

    for (int guard = 0; guard < 1000; guard++) {
        for (int i = 0; i < 8; i++) {
            if (f->ccDeltaX + f->ccDeltaY <= 0) {
                return 1;
            }
            cross_country_advance(f);
        }
        f->x = f->crossCountryX / 15;
        f->y = f->crossCountryY / 15;
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
