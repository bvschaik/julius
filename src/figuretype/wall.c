#include "wall.h"

#include "building/building.h"
#include "core/calc.h"
#include "figure/combat.h"
#include "figure/enemy_army.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/properties.h"
#include "figure/route.h"
#include "figuretype/missile.h"
#include "graphics/image.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "sound/effect.h"

#include "Data/State.h"
#include "Terrain.h"

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
    building *b = building_get(f->buildingId);
    f->terrainUsage = FigureTerrainUsage_Walls;
    f->useCrossCountry = 0;
    f->isGhost = 1;
    f->heightAdjustedTicks = 10;
    f->currentHeight = 45;
    
    if (!BuildingIsInUse(b) || b->figureId4 != f->id) {
        f->state = FigureState_Dead;
    }
    if (b->numWorkers <= 0 || b->figureId <= 0) {
        f->state = FigureState_Dead;
    }
    map_figure_delete(f);
    switch (Data_State.map.orientation) {
        case DIR_0_TOP: f->x = b->x; f->y = b->y; break;
        case DIR_2_RIGHT: f->x = b->x + 1; f->y = b->y; break;
        case DIR_4_BOTTOM: f->x = b->x + 1; f->y = b->y + 1; break;
        case DIR_6_LEFT: f->x = b->x; f->y = b->y + 1; break;
    }
    f->gridOffset = map_grid_offset(f->x, f->y);
    map_figure_add(f);

    switch (f->actionState) {
        case FIGURE_ACTION_149_CORPSE:
            f->state = FigureState_Dead;
            break;
        case FIGURE_ACTION_180_BALLISTA_CREATED:
            f->waitTicks++;
            if (f->waitTicks > 20) {
                f->waitTicks = 0;
                int x_tile, y_tile;
                if (figure_combat_get_missile_target_for_soldier(f, 15, &x_tile, &y_tile)) {
                    f->actionState = FIGURE_ACTION_181_BALLISTA_FIRING;
                    f->waitTicksMissile = figure_properties_for_type(f->type)->missile_delay;
                }
            }
            break;
        case FIGURE_ACTION_181_BALLISTA_FIRING:
            f->waitTicksMissile++;
            if (f->waitTicksMissile > figure_properties_for_type(f->type)->missile_delay) {
                int x_tile, y_tile;
                if (figure_combat_get_missile_target_for_soldier(f, 15, &x_tile, &y_tile)) {
                    f->direction = calc_missile_shooter_direction(f->x, f->y, x_tile, y_tile);
                    f->waitTicksMissile = 0;
                    figure_create_missile(f->id, f->x, f->y, x_tile, y_tile, FIGURE_BOLT);
                    sound_effect_play(SOUND_EFFECT_BALLISTA_SHOOT);
                } else {
                    f->actionState = FIGURE_ACTION_180_BALLISTA_CREATED;
                }
            }
            break;
    }
    int dir = figure_image_direction(f);
    if (f->actionState == FIGURE_ACTION_181_BALLISTA_FIRING) {
        f->graphicId = image_group(GROUP_FIGURE_BALLISTA) + dir +
            8 * BALLISTA_FIRING_OFFSETS[f->waitTicksMissile / 4];
    } else {
        f->graphicId = image_group(GROUP_FIGURE_BALLISTA) + dir;
    }
}

static void tower_sentry_pick_target(figure *f)
{
    if (enemy_army_total_enemy_formations() <= 0) {
        return;
    }
    if (f->actionState == FIGURE_ACTION_150_ATTACK ||
        f->actionState == FIGURE_ACTION_149_CORPSE) {
        return;
    }
    if (f->inBuildingWaitTicks) {
        return;
    }
    f->waitTicksNextTarget++;
    if (f->waitTicksNextTarget >= 40) {
        f->waitTicksNextTarget = 0;
        int x_tile, y_tile;
        if (figure_combat_get_missile_target_for_soldier(f, 10, &x_tile, &y_tile)) {
            f->actionState = FIGURE_ACTION_172_TOWER_SENTRY_FIRING;
            f->destinationX = f->x;
            f->destinationY = f->y;
        }
    }
}

static int tower_sentry_init_patrol(building *b, int *x_tile, int *y_tile)
{
    int dir = b->figureRoamDirection;
    int x = b->x;
    int y = b->y;
    switch (dir) {
        case DIR_0_TOP: y -= 8; break;
        case DIR_2_RIGHT: x += 8; break;
        case DIR_4_BOTTOM: y += 8; break;
        case DIR_6_LEFT: x -= 8; break;
    }
    map_grid_bound(&x, &y);

    if (Terrain_getWallTileWithinRadius(x, y, 6, x_tile, y_tile)) {
        b->figureRoamDirection += 2;
        if (b->figureRoamDirection > 6) b->figureRoamDirection = 0;
        return 1;
    }
    for (int i = 0; i < 4; i++) {
        dir = b->figureRoamDirection;
        b->figureRoamDirection += 2;
        if (b->figureRoamDirection > 6) b->figureRoamDirection = 0;
        x = b->x;
        y = b->y;
        switch (dir) {
            case DIR_0_TOP: y -= 3; break;
            case DIR_2_RIGHT: x += 3; break;
            case DIR_4_BOTTOM: y += 3; break;
            case DIR_6_LEFT: x -= 3; break;
        }
        map_grid_bound(&x, &y);
        if (Terrain_getWallTileWithinRadius(x, y, 6, x_tile, y_tile)) {
            return 1;
        }
    }
    return 0;
}

void figure_tower_sentry_action(figure *f)
{
    building *b = building_get(f->buildingId);
    f->terrainUsage = FigureTerrainUsage_Walls;
    f->useCrossCountry = 0;
    f->isGhost = 1;
    f->heightAdjustedTicks = 10;
    f->maxRoamLength = 800;
    if (!BuildingIsInUse(b) || b->figureId != f->id) {
        f->state = FigureState_Dead;
    }
    figure_image_increase_offset(f, 12);
    
    tower_sentry_pick_target(f);
    switch (f->actionState) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_170_TOWER_SENTRY_AT_REST:
            f->graphicOffset = 0;
            f->waitTicks++;
            if (f->waitTicks > 40) {
                f->waitTicks = 0;
                int x_tile, y_tile;
                if (tower_sentry_init_patrol(b, &x_tile, &y_tile)) {
                    f->actionState = FIGURE_ACTION_171_TOWER_SENTRY_PATROLLING;
                    f->destinationX = x_tile;
                    f->destinationY = y_tile;
                    figure_route_remove(f);
                }
            }
            break;
        case FIGURE_ACTION_171_TOWER_SENTRY_PATROLLING:
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->actionState = FIGURE_ACTION_173_TOWER_SENTRY_RETURNING;
                f->destinationX = f->sourceX;
                f->destinationY = f->sourceY;
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->actionState = FIGURE_ACTION_170_TOWER_SENTRY_AT_REST;
            }
            break;
        case FIGURE_ACTION_172_TOWER_SENTRY_FIRING:
            figure_movement_move_ticks_tower_sentry(f, 1);
            f->waitTicksMissile++;
            if (f->waitTicksMissile > figure_properties_for_type(f->type)->missile_delay) {
                int x_tile, y_tile;
                if (figure_combat_get_missile_target_for_soldier(f, 10, &x_tile, &y_tile)) {
                    f->direction = calc_missile_shooter_direction(f->x, f->y, x_tile, y_tile);
                    f->waitTicksMissile = 0;
                    figure_create_missile(f->id, f->x, f->y, x_tile, y_tile, FIGURE_JAVELIN);
                } else {
                    f->actionState = FIGURE_ACTION_173_TOWER_SENTRY_RETURNING;
                    f->destinationX = f->sourceX;
                    f->destinationY = f->sourceY;
                    figure_route_remove(f);
                }
            }
            break;
        case FIGURE_ACTION_173_TOWER_SENTRY_RETURNING:
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->actionState = FIGURE_ACTION_170_TOWER_SENTRY_AT_REST;
            } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
            }
            break;
        case FIGURE_ACTION_174_TOWER_SENTRY_GOING_TO_TOWER:
            f->terrainUsage = FigureTerrainUsage_Roads;
            f->isGhost = 0;
            f->heightAdjustedTicks = 0;
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                map_figure_delete(f);
                f->sourceX = f->x = b->x;
                f->sourceY = f->y = b->y;
                f->gridOffset = map_grid_offset(f->x, f->y);
                map_figure_add(f);
                f->actionState = FIGURE_ACTION_170_TOWER_SENTRY_AT_REST;
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
            }
            break;
    }
    if (map_terrain_is(f->gridOffset, TERRAIN_WALL)) {
        f->currentHeight = 18;
    } else if (map_terrain_is(f->gridOffset, TERRAIN_GATEHOUSE)) {
        f->inBuildingWaitTicks = 24;
    } else if (f->actionState != FIGURE_ACTION_174_TOWER_SENTRY_GOING_TO_TOWER) {
        f->state = FigureState_Dead;
    }
    if (f->inBuildingWaitTicks) {
        f->inBuildingWaitTicks--;
        f->heightAdjustedTicks = 0;
    }
    int dir = figure_image_direction(f);
    if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = image_group(GROUP_FIGURE_TOWER_SENTRY) +
            136 + figure_image_corpse_offset(f);
    } else if (f->actionState == FIGURE_ACTION_172_TOWER_SENTRY_FIRING) {
        f->graphicId = image_group(GROUP_FIGURE_TOWER_SENTRY) +
            dir + 96 + 8 * TOWER_SENTRY_FIRING_OFFSETS[f->waitTicksMissile / 2];
    } else {
        f->graphicId = image_group(GROUP_FIGURE_TOWER_SENTRY) +
            dir + 8 * f->graphicOffset;
    }
}

void figure_tower_sentry_reroute()
{
    for (int i = 1; i < MAX_FIGURES; i++) {
        figure *f = figure_get(i);
        if (f->type != FIGURE_TOWER_SENTRY || map_routing_is_wall_passable(f->gridOffset)) {
            continue;
        }
        // tower sentry got off wall due to rotation
        int x_tile, y_tile;
        if (Terrain_getWallTileWithinRadius(f->x, f->y, 2, &x_tile, &y_tile)) {
            figure_route_remove(f);
            f->progressOnTile = 0;
            map_figure_delete(f);
            f->previousTileX = f->x = x_tile;
            f->previousTileY = f->y = y_tile;
            f->crossCountryX = 15 * x_tile;
            f->crossCountryY = 15 * y_tile;
            f->gridOffset = map_grid_offset(x_tile, y_tile);
            map_figure_add(f);
            f->actionState = FIGURE_ACTION_173_TOWER_SENTRY_RETURNING;
            f->destinationX = f->sourceX;
            f->destinationY = f->sourceY;
        } else {
            // Teleport back to tower
            map_figure_delete(f);
            building *b = building_get(f->buildingId);
            f->sourceX = f->x = b->x;
            f->sourceY = f->y = b->y;
            f->gridOffset = map_grid_offset(f->x, f->y);
            map_figure_add(f);
            f->actionState = FIGURE_ACTION_170_TOWER_SENTRY_AT_REST;
            figure_route_remove(f);
        }
    }
}

void figure_kill_tower_sentries_at(int x, int y)
{
    for (int i = 0; i < MAX_FIGURES; i++) {
        figure *f = figure_get(i);
        if (!figure_is_dead(f) && f->type == FIGURE_TOWER_SENTRY) {
            if (calc_maximum_distance(f->x, f->y, x, y) <= 1) {
                f->state = FigureState_Dead;
            }
        }
    }
}
