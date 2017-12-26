#include "entertainer.h"

#include "building/building.h"
#include "building/list.h"
#include "core/calc.h"
#include "figure/image.h"
#include "figure/route.h"
#include "graphics/image.h"
#include "map/grid.h"
#include "map/road_access.h"
#include "map/road_network.h"
#include "scenario/gladiator_revolt.h"

#include "FigureAction.h"
#include "FigureMovement.h"

static int determine_destination(int x, int y, building_type type1, building_type type2)
{
    int road_network = map_road_network_get(map_grid_offset(x,y));

    building_list_small_clear();
    
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (!BuildingIsInUse(b)) {
            continue;
        }
        if (b->type != type1 && b->type != type2) {
            continue;
        }
        if (b->distanceFromEntry && b->roadNetworkId == road_network) {
            if (b->type == BUILDING_HIPPODROME && b->prevPartBuildingId) {
                continue;
            }
            building_list_small_add(i);
        }
    }
    int total_venues = building_list_small_size();
    if (total_venues <= 0) {
        return 0;
    }
    const int *venues = building_list_small_items();
    int min_building_id = 0;
    int min_distance = 10000;
    for (int i = 0; i < total_venues; i++) {
        building *b = building_get(venues[i]);
        int days_left;
        if (b->type == type1) {
            days_left = b->data.entertainment.days1;
        } else if (b->type == type2) {
            days_left = b->data.entertainment.days2;
        } else {
            days_left = 0;
        }
        int dist = days_left + calc_maximum_distance(x, y, b->x, b->y);
        if (dist < min_distance) {
            min_distance = dist;
            min_building_id = venues[i];
        }
    }
    return min_building_id;
}

static void update_shows(figure *f)
{
    building *b = building_get(f->destinationBuildingId);
    switch (f->type) {
        case FIGURE_ACTOR:
            b->data.entertainment.play++;
            if (b->data.entertainment.play >= 5) {
                b->data.entertainment.play = 0;
            }
            if (b->type == BUILDING_THEATER) {
                b->data.entertainment.days1 = 32;
            } else {
                b->data.entertainment.days2 = 32;
            }
            break;
        case FIGURE_GLADIATOR:
            if (b->type == BUILDING_AMPHITHEATER) {
                b->data.entertainment.days1 = 32;
            } else {
                b->data.entertainment.days2 = 32;
            }
            break;
        case FIGURE_LION_TAMER:
        case FIGURE_CHARIOTEER:
            b->data.entertainment.days1 = 32;
            break;
    }
}

static void update_image(figure *f)
{
    int dir = figure_image_normalize_direction(f->direction < 8 ? f->direction : f->previousTileDirection);

    if (f->type == FIGURE_CHARIOTEER) {
        f->cartGraphicId = 0;
        if (f->actionState == FIGURE_ACTION_150_ATTACK ||
            f->actionState == FIGURE_ACTION_149_CORPSE) {
            f->graphicId = image_group(GROUP_FIGURE_CHARIOTEER) + dir;
        } else {
            f->graphicId = image_group(GROUP_FIGURE_CHARIOTEER) +
                dir + 8 * f->graphicOffset;
        }
        return;
    }
    int image_id;
    if (f->type == FIGURE_ACTOR) {
        image_id = image_group(GROUP_FIGURE_ACTOR);
    } else if (f->type == FIGURE_GLADIATOR) {
        image_id = image_group(GROUP_FIGURE_GLADIATOR);
    } else if (f->type == FIGURE_LION_TAMER) {
        image_id = image_group(GROUP_FIGURE_LION_TAMER);
        if (f->waitTicksMissile >= 96) {
            image_id = image_group(GROUP_FIGURE_LION_TAMER_WHIP);
        }
        f->cartGraphicId = image_group(GROUP_FIGURE_LION);
    } else {
        return;
    }
    if (f->actionState == FIGURE_ACTION_150_ATTACK) {
        if (f->type == FIGURE_GLADIATOR) {
            f->graphicId = image_id + 104 + dir + 8 * (f->graphicOffset / 2);
        } else {
            f->graphicId = image_id + dir;
        }
    } else if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = image_id + 96 + figure_image_corpse_offset(f);
        f->cartGraphicId = 0;
    } else {
        f->graphicId = image_id + dir + 8 * f->graphicOffset;
    }
    if (f->cartGraphicId) {
        f->cartGraphicId += dir + 8 * f->graphicOffset;
        figure_image_set_cart_offset(f, dir);
    }
}

void figure_entertainer_action(figure *f)
{
    building *b = building_get(f->buildingId);
    f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART);
    f->terrainUsage = FigureTerrainUsage_Roads;
    f->useCrossCountry = 0;
    f->maxRoamLength = 512;
    figure_image_increase_offset(f, 12);
    f->waitTicksMissile++;
    if (f->waitTicksMissile >= 120) {
        f->waitTicksMissile = 0;
    }
    if (scenario_gladiator_revolt_is_in_progress() && f->type == FIGURE_GLADIATOR) {
        if (f->actionState == FIGURE_ACTION_92_ENTERTAINER_GOING_TO_VENUE ||
            f->actionState == FIGURE_ACTION_94_ENTERTAINER_ROAMING ||
            f->actionState == FIGURE_ACTION_95_ENTERTAINER_RETURNING) {
            f->type = FIGURE_ENEMY54_GLADIATOR;
            figure_route_remove(f);
            f->roamLength = 0;
            f->actionState = FIGURE_ACTION_158_NATIVE_CREATED;
            return;
        }
    }
    int speed_factor = f->type == FIGURE_CHARIOTEER ? 2 : 1;
    switch (f->actionState) {
        case FIGURE_ACTION_150_ATTACK:
            FigureAction_Common_handleAttack(f);
            figure_image_increase_offset(f, 32);
            break;
        case FIGURE_ACTION_149_CORPSE:
            FigureAction_Common_handleCorpse(f);
            break;
        case FIGURE_ACTION_90_ENTERTAINER_AT_SCHOOL_CREATED:
            f->isGhost = 1;
            f->graphicOffset = 0;
            f->waitTicksMissile = 0;
            f->waitTicks--;
            if (f->waitTicks <= 0) {
                int x_road, y_road;
                if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                    f->actionState = FIGURE_ACTION_91_ENTERTAINER_EXITING_SCHOOL;
                    FigureAction_Common_setCrossCountryDestination(f, x_road, y_road);
                    f->roamLength = 0;
                } else {
                    f->state = FigureState_Dead;
                }
            }
            break;
        case FIGURE_ACTION_91_ENTERTAINER_EXITING_SCHOOL:
            f->useCrossCountry = 1;
            f->isGhost = 1;
            if (FigureMovement_crossCountryWalkTicks(f, 1) == 1) {
                int dst_building_id = 0;
                switch (f->type) {
                    case FIGURE_ACTOR:
                        dst_building_id = determine_destination(f->x, f->y, BUILDING_THEATER, BUILDING_AMPHITHEATER);
                        break;
                    case FIGURE_GLADIATOR:
                        dst_building_id = determine_destination(f->x, f->y, BUILDING_AMPHITHEATER, BUILDING_COLOSSEUM);
                        break;
                    case FIGURE_LION_TAMER:
                        dst_building_id = determine_destination(f->x, f->y, BUILDING_COLOSSEUM, 0);
                        break;
                    case FIGURE_CHARIOTEER:
                        dst_building_id = determine_destination(f->x, f->y, BUILDING_HIPPODROME, 0);
                        break;
                }
                if (dst_building_id) {
                    building *b_dst = building_get(dst_building_id);
                    int x_road, y_road;
                    if (map_closest_road_within_radius(b_dst->x, b_dst->y, b_dst->size, 2, &x_road, &y_road)) {
                        f->destinationBuildingId = dst_building_id;
                        f->actionState = FIGURE_ACTION_92_ENTERTAINER_GOING_TO_VENUE;
                        f->destinationX = x_road;
                        f->destinationY = y_road;
                        f->roamLength = 0;
                    } else {
                        f->state = FigureState_Dead;
                    }
                } else {
                    f->state = FigureState_Dead;
                }
            }
            f->isGhost = 1;
            break;
        case FIGURE_ACTION_92_ENTERTAINER_GOING_TO_VENUE:
            f->isGhost = 0;
            f->roamLength++;
            if (f->roamLength >= 3200) {
                f->state = FigureState_Dead;
            }
            FigureMovement_walkTicks(f, speed_factor);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                update_shows(f);
                f->state = FigureState_Dead;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
            }
            break;
        case FIGURE_ACTION_94_ENTERTAINER_ROAMING:
            f->isGhost = 0;
            f->roamLength++;
            if (f->roamLength >= f->maxRoamLength) {
                int x_road, y_road;
                if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                    f->actionState = FIGURE_ACTION_95_ENTERTAINER_RETURNING;
                    f->destinationX = x_road;
                    f->destinationY = y_road;
                } else {
                    f->state = FigureState_Dead;
                }
            }
            FigureMovement_roamTicks(f, speed_factor);
            break;
        case FIGURE_ACTION_95_ENTERTAINER_RETURNING:
            FigureMovement_walkTicks(f, speed_factor);
            if (f->direction == DIR_FIGURE_AT_DESTINATION ||
                f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
            }
            break;
    }
    update_image(f);
}
