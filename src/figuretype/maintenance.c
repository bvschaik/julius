#include "maintenance.h"

#include "building/building.h"
#include "building/list.h"
#include "building/maintenance.h"
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

#include "Data/CityInfo.h"

void figure_engineer_action(figure *f)
{
    building *b = building_get(f->buildingId);
    
    f->terrainUsage = FigureTerrainUsage_Roads;
    f->useCrossCountry = 0;
    f->maxRoamLength = 640;
    if (b->state != BUILDING_STATE_IN_USE || b->figureId != f->id) {
        f->state = FigureState_Dead;
    }
    figure_image_increase_offset(f, 12);
    
    switch (f->actionState) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_60_ENGINEER_CREATED:
            f->isGhost = 1;
            f->graphicOffset = 0;
            f->waitTicks--;
            if (f->waitTicks <= 0) {
                int x_road, y_road;
                if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                    f->actionState = FIGURE_ACTION_61_ENGINEER_ENTERING_EXITING;
                    figure_movement_set_cross_country_destination(f, x_road, y_road);
                    f->roamLength = 0;
                } else {
                    f->state = FigureState_Dead;
                }
            }
            break;
        case FIGURE_ACTION_61_ENGINEER_ENTERING_EXITING:
            f->useCrossCountry = 1;
            f->isGhost = 1;
            if (figure_movement_move_ticks_cross_country(f, 1) == 1) {
                if (map_building_at(f->gridOffset) == f->buildingId) {
                    // returned to own building
                    f->state = FigureState_Dead;
                } else {
                    f->actionState = FIGURE_ACTION_62_ENGINEER_ROAMING;
                    figure_movement_init_roaming(f);
                    f->roamLength = 0;
                }
            }
            break;
        case FIGURE_ACTION_62_ENGINEER_ROAMING:
            f->isGhost = 0;
            f->roamLength++;
            if (f->roamLength >= f->maxRoamLength) {
                int x_road, y_road;
                if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                    f->actionState = FIGURE_ACTION_63_ENGINEER_RETURNING;
                    f->destinationX = x_road;
                    f->destinationY = y_road;
                } else {
                    f->state = FigureState_Dead;
                }
            }
            figure_movement_roam_ticks(f, 1);
            break;
        case FIGURE_ACTION_63_ENGINEER_RETURNING:
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->actionState = FIGURE_ACTION_61_ENGINEER_ENTERING_EXITING;
                figure_movement_set_cross_country_destination(f, b->x, b->y);
                f->roamLength = 0;
            } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
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
        if (f->state != FigureState_Alive || f->targetedByFigureId) {
            continue;
        }
        int dist;
        if (f->type == FIGURE_RIOTER || f->type == FIGURE_ENEMY54_GLADIATOR) {
            dist = calc_maximum_distance(x, y, f->x, f->y);
        } else if (f->type == FIGURE_INDIGENOUS_NATIVE && f->actionState == FIGURE_ACTION_159_NATIVE_ATTACKING) {
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
    if (Data_CityInfo.riotersOrAttackingNativesInCity <= 0 && enemy_army_total_enemy_formations() <= 0) {
        return 0;
    }
    switch (f->actionState) {
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
    f->waitTicksNextTarget++;
    if (f->waitTicksNextTarget < 10) {
        return 0;
    }
    f->waitTicksNextTarget = 0;
    int distance;
    int enemy_id = get_nearest_enemy(f->x, f->y, &distance);
    if (enemy_id > 0 && distance <= 30) {
        figure *enemy = figure_get(enemy_id);
        f->waitTicksNextTarget = 0;
        f->actionState = FIGURE_ACTION_76_PREFECT_GOING_TO_ENEMY;
        f->destinationX = enemy->x;
        f->destinationY = enemy->y;
        f->targetFigureId = enemy_id;
        enemy->targetedByFigureId = f->id;
        f->targetFigureCreatedSequence = enemy->createdSequence;
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
    switch (f->actionState) {
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
    f->waitTicksMissile++;
    if (f->waitTicksMissile < 20) {
        return 0;
    }
    int distance;
    int ruin_id = building_maintenance_get_closest_burning_ruin(f->x, f->y, &distance);
    if (ruin_id > 0 && distance <= 25) {
        building *ruin = building_get(ruin_id);
        f->waitTicksMissile = 0;
        f->actionState = FIGURE_ACTION_74_PREFECT_GOING_TO_FIRE;
        f->destinationX = ruin->roadAccessX;
        f->destinationY = ruin->roadAccessY;
        f->destinationBuildingId = ruin_id;
        figure_route_remove(f);
        ruin->figureId4 = f->id;
        return 1;
    }
    return 0;
}

static void extinguish_fire(figure *f)
{
    building *burn = building_get(f->destinationBuildingId);
    int distance = calc_maximum_distance(f->x, f->y, burn->x, burn->y);
    if (burn->state == BUILDING_STATE_IN_USE && burn->type == BUILDING_BURNING_RUIN && distance < 2) {
        burn->fireDuration = 32;
        sound_effect_play(SOUND_EFFECT_FIRE_SPLASH);
    } else {
        f->waitTicks = 1;
    }
    f->attackDirection = calc_general_direction(f->x, f->y, burn->x, burn->y);
    if (f->attackDirection >= 8) {
        f->attackDirection = 0;
    }
    f->waitTicks--;
    if (f->waitTicks <= 0) {
        f->waitTicksMissile = 20;
        if (!fight_fire(f)) {
            building *b = building_get(f->buildingId);
            int x_road, y_road;
            if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                f->actionState = FIGURE_ACTION_73_PREFECT_RETURNING;
                f->destinationX = x_road;
                f->destinationY = y_road;
                figure_route_remove(f);
            } else {
                f->state = FigureState_Dead;
            }
        }
    }
}

static int target_is_alive(figure *f)
{
    if (f->targetFigureId <= 0) {
        return 0;
    }
    figure *target = figure_get(f->targetFigureId);
    if (!figure_is_dead(target) && target->createdSequence == f->targetFigureCreatedSequence) {
        return 1;
    }
    return 0;
}

void figure_prefect_action(figure *f)
{
    building *b = building_get(f->buildingId);
    
    f->terrainUsage = FigureTerrainUsage_Roads;
    f->useCrossCountry = 0;
    f->maxRoamLength = 640;
    if (b->state != BUILDING_STATE_IN_USE || b->figureId != f->id) {
        f->state = FigureState_Dead;
    }
    figure_image_increase_offset(f, 12);
    
    // special actions
    if (!fight_enemy(f)) {
        fight_fire(f);
    }
    switch (f->actionState) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_70_PREFECT_CREATED:
            f->isGhost = 1;
            f->graphicOffset = 0;
            f->waitTicks--;
            if (f->waitTicks <= 0) {
                int x_road, y_road;
                if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                    f->actionState = FIGURE_ACTION_71_PREFECT_ENTERING_EXITING;
                    figure_movement_set_cross_country_destination(f, x_road, y_road);
                    f->roamLength = 0;
                } else {
                    f->state = FigureState_Dead;
                }
            }
            break;
        case FIGURE_ACTION_71_PREFECT_ENTERING_EXITING:
            f->useCrossCountry = 1;
            f->isGhost = 1;
            if (figure_movement_move_ticks_cross_country(f, 1) == 1) {
                if (map_building_at(f->gridOffset) == f->buildingId) {
                    // returned to own building
                    f->state = FigureState_Dead;
                } else {
                    f->actionState = FIGURE_ACTION_72_PREFECT_ROAMING;
                    figure_movement_init_roaming(f);
                    f->roamLength = 0;
                }
            }
            break;
        case FIGURE_ACTION_72_PREFECT_ROAMING:
            f->isGhost = 0;
            f->roamLength++;
            if (f->roamLength >= f->maxRoamLength) {
                int x_road, y_road;
                if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                    f->actionState = FIGURE_ACTION_73_PREFECT_RETURNING;
                    f->destinationX = x_road;
                    f->destinationY = y_road;
                    figure_route_remove(f);
                } else {
                    f->state = FigureState_Dead;
                }
            }
            figure_movement_roam_ticks(f, 1);
            break;
        case FIGURE_ACTION_73_PREFECT_RETURNING:
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->actionState = FIGURE_ACTION_71_PREFECT_ENTERING_EXITING;
                figure_movement_set_cross_country_destination(f, b->x, b->y);
                f->roamLength = 0;
            } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
            }
            break;
        case FIGURE_ACTION_74_PREFECT_GOING_TO_FIRE:
            f->terrainUsage = FigureTerrainUsage_Any;
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->actionState = FIGURE_ACTION_75_PREFECT_AT_FIRE;
                figure_route_remove(f);
                f->roamLength = 0;
                f->waitTicks = 50;
            } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
            }
            break;
        case FIGURE_ACTION_75_PREFECT_AT_FIRE:
            extinguish_fire(f);
            break;
        case FIGURE_ACTION_76_PREFECT_GOING_TO_ENEMY:
            f->terrainUsage = FigureTerrainUsage_Any;
            if (!target_is_alive(f)) {
                int x_road, y_road;
                if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                    f->actionState = FIGURE_ACTION_73_PREFECT_RETURNING;
                    f->destinationX = x_road;
                    f->destinationY = y_road;
                    figure_route_remove(f);
                    f->roamLength = 0;
                } else {
                    f->state = FigureState_Dead;
                }
            }
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                figure *target = figure_get(f->targetFigureId);
                f->destinationX = target->x;
                f->destinationY = target->y;
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
            }
            break;
    }
    // graphic id
    int dir;
    if (f->actionState == FIGURE_ACTION_75_PREFECT_AT_FIRE ||
        f->actionState == FIGURE_ACTION_150_ATTACK) {
        dir = f->attackDirection;
    } else if (f->direction < 8) {
        dir = f->direction;
    } else {
        dir = f->previousTileDirection;
    }
    dir = figure_image_normalize_direction(dir);
    switch (f->actionState) {
        case FIGURE_ACTION_74_PREFECT_GOING_TO_FIRE:
            f->graphicId = image_group(GROUP_FIGURE_PREFECT_WITH_BUCKET) +
                dir + 8 * f->graphicOffset;
            break;
        case FIGURE_ACTION_75_PREFECT_AT_FIRE:
            f->graphicId = image_group(GROUP_FIGURE_PREFECT_WITH_BUCKET) +
                dir + 96 + 8 * (f->graphicOffset / 2);
            break;
        case FIGURE_ACTION_150_ATTACK:
            if (f->attackGraphicOffset >= 12) {
                f->graphicId = image_group(GROUP_FIGURE_PREFECT) +
                    104 + dir + 8 * ((f->attackGraphicOffset - 12) / 2);
            } else {
                f->graphicId = image_group(GROUP_FIGURE_PREFECT) + 104 + dir;
            }
            break;
        case FIGURE_ACTION_149_CORPSE:
            f->graphicId = image_group(GROUP_FIGURE_PREFECT) +
                96 + figure_image_corpse_offset(f);
            break;
        default:
            f->graphicId = image_group(GROUP_FIGURE_PREFECT) +
                dir + 8 * f->graphicOffset;
            break;
    }
}

void figure_worker_action(figure *f)
{
    f->terrainUsage = FigureTerrainUsage_Roads;
    f->useCrossCountry = 0;
    f->maxRoamLength = 384;
    building *b = building_get(f->buildingId);
    if (b->state != BUILDING_STATE_IN_USE || b->figureId != f->id) {
        f->state = FigureState_Dead;
    }
}
