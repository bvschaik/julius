#include "migrant.h"

#include "building/house.h"
#include "building/model.h"
#include "city/map.h"
#include "city/population.h"
#include "core/calc.h"
#include "core/image.h"
#include "figure/combat.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "map/road_access.h"

void figure_create_immigrant(building *house, int num_people)
{
    const map_tile *entry = city_map_entry_point();
    figure *f = figure_create(FIGURE_IMMIGRANT, entry->x, entry->y, DIR_0_TOP);
    f->actionState = FIGURE_ACTION_1_IMMIGRANT_CREATED;
    f->immigrantBuildingId = house->id;
    house->immigrantFigureId = f->id;
    f->waitTicks = 10 + (house->houseGenerationDelay & 0x7f);
    f->migrantNumPeople = num_people;
}

void figure_create_emigrant(building *house, int num_people)
{
    city_population_remove(num_people);
    if (num_people < house->housePopulation) {
        house->housePopulation -= num_people;
    } else {
        house->housePopulation = 0;
        building_house_change_to_vacant_lot(house);
    }
    figure *f = figure_create(FIGURE_EMIGRANT, house->x, house->y, DIR_0_TOP);
    f->actionState = FIGURE_ACTION_4_EMIGRANT_CREATED;
    f->waitTicks = 0;
    f->migrantNumPeople = num_people;
}

void figure_create_homeless(int x, int y, int num_people)
{
    figure *f = figure_create(FIGURE_HOMELESS, x, y, DIR_0_TOP);
    f->actionState = FIGURE_ACTION_7_HOMELESS_CREATED;
    f->waitTicks = 0;
    f->migrantNumPeople = num_people;
    city_population_remove_homeless(num_people);
}

static void update_direction_and_image(figure *f)
{
    figure_image_update(f, image_group(GROUP_FIGURE_MIGRANT));
    if (f->actionState == FIGURE_ACTION_2_IMMIGRANT_ARRIVING ||
        f->actionState == FIGURE_ACTION_6_EMIGRANT_LEAVING) {
        int dir = figure_image_direction(f);
        f->cartGraphicId = image_group(GROUP_FIGURE_MIGRANT_CART) + dir;
        figure_image_set_cart_offset(f, (dir + 4) % 8);
    }
}

static int closest_house_with_room(int x, int y)
{
    int min_dist = 1000;
    int min_building_id = 0;
    int max_id = building_get_highest_id();
    for (int i = 1; i <= max_id; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->houseSize && b->distanceFromEntry > 0 && b->housePopulationRoom > 0) {
            if (!b->immigrantFigureId) {
                int dist = calc_maximum_distance(x, y, b->x, b->y);
                if (dist < min_dist) {
                    min_dist = dist;
                    min_building_id = i;
                }
            }
        }
    }
    return min_building_id;

}

void figure_immigrant_action(figure *f)
{
    building *b = building_get(f->immigrantBuildingId);
    
    f->terrainUsage = TERRAIN_USAGE_ANY;
    f->cartGraphicId = 0;
    if (b->state != BUILDING_STATE_IN_USE || b->immigrantFigureId != f->id || !b->houseSize) {
        f->state = FIGURE_STATE_DEAD;
        return;
    }
    
    figure_image_increase_offset(f, 12);
    
    switch (f->actionState) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_1_IMMIGRANT_CREATED:
            f->isGhost = 1;
            f->graphicOffset = 0;
            f->waitTicks--;
            if (f->waitTicks <= 0) {
                int xRoad, yRoad;
                if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
                    f->actionState = FIGURE_ACTION_2_IMMIGRANT_ARRIVING;
                    f->destinationX = xRoad;
                    f->destinationY = yRoad;
                    f->roamLength = 0;
                } else {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            break;
        case FIGURE_ACTION_2_IMMIGRANT_ARRIVING:
            f->isGhost = 0;
            figure_movement_move_ticks(f, 1);
            switch (f->direction) {
                case DIR_FIGURE_AT_DESTINATION:
                    f->actionState = FIGURE_ACTION_3_IMMIGRANT_ENTERING_HOUSE;
                    figure_movement_set_cross_country_destination(f, b->x, b->y);
                    f->roamLength = 0;
                    break;
                case DIR_FIGURE_REROUTE:
                    figure_route_remove(f);
                    break;
                case DIR_FIGURE_LOST:
                    b->immigrantFigureId = 0;
                    b->distanceFromEntry = 0;
                    f->state = FIGURE_STATE_DEAD;
                    break;
            }
            break;
        case FIGURE_ACTION_3_IMMIGRANT_ENTERING_HOUSE:
            f->useCrossCountry = 1;
            f->isGhost = 1;
            if (figure_movement_move_ticks_cross_country(f, 1) == 1) {
                f->state = FIGURE_STATE_DEAD;
                int maxPeople = model_get_house(b->subtype.houseLevel)->max_people;
                if (b->houseIsMerged) {
                    maxPeople *= 4;
                }
                int room = maxPeople - b->housePopulation;
                if (room < 0) {
                    room = 0;
                }
                if (room < f->migrantNumPeople) {
                    f->migrantNumPeople = room;
                }
                if (!b->housePopulation) {
                    building_house_change_to(b, BUILDING_HOUSE_SMALL_TENT);
                }
                b->housePopulation += f->migrantNumPeople;
                b->housePopulationRoom = maxPeople - b->housePopulation;
                city_population_add(f->migrantNumPeople);
                b->immigrantFigureId = 0;
            }
            f->isGhost = f->inBuildingWaitTicks ? 1 : 0;
            break;
    }
    
    update_direction_and_image(f);
}

void figure_emigrant_action(figure *f)
{
    f->terrainUsage = TERRAIN_USAGE_ANY;
    f->cartGraphicId = 0;
    
    figure_image_increase_offset(f, 12);
    
    switch (f->actionState) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_4_EMIGRANT_CREATED:
            f->isGhost = 1;
            f->graphicOffset = 0;
            f->waitTicks++;
            if (f->waitTicks >= 5) {
                int xRoad, yRoad;
                if (!map_closest_road_within_radius(f->x, f->y, 1, 5, &xRoad, &yRoad)) {
                    f->state = FIGURE_STATE_DEAD;
                }
                f->actionState = FIGURE_ACTION_5_EMIGRANT_EXITING_HOUSE;
                figure_movement_set_cross_country_destination(f, xRoad, yRoad);
                f->roamLength = 0;
            }
            break;
        case FIGURE_ACTION_5_EMIGRANT_EXITING_HOUSE:
            f->useCrossCountry = 1;
            f->isGhost = 1;
            if (figure_movement_move_ticks_cross_country(f, 1) == 1) {
                const map_tile *entry = city_map_entry_point();
                f->actionState = FIGURE_ACTION_6_EMIGRANT_LEAVING;
                f->destinationX = entry->x;
                f->destinationY = entry->y;
                f->roamLength = 0;
                f->progressOnTile = 15;
            }
            f->isGhost = f->inBuildingWaitTicks ? 1 : 0;
            break;
        case FIGURE_ACTION_6_EMIGRANT_LEAVING:
            f->useCrossCountry = 0;
            f->isGhost = 0;
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION ||
                f->direction == DIR_FIGURE_REROUTE ||
                f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
    }
    update_direction_and_image(f);
}

void figure_homeless_action(figure *f)
{
    figure_image_increase_offset(f, 12);
    f->terrainUsage = TERRAIN_USAGE_PREFER_ROADS;
    
    switch (f->actionState) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_7_HOMELESS_CREATED:
            f->graphicOffset = 0;
            f->waitTicks++;
            if (f->waitTicks > 51) {
                int building_id = closest_house_with_room(f->x, f->y);
                if (building_id) {
                    building *b = building_get(building_id);
                    int xRoad, yRoad;
                    if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
                        b->immigrantFigureId = f->id;
                        f->immigrantBuildingId = building_id;
                        f->actionState = FIGURE_ACTION_8_HOMELESS_GOING_TO_HOUSE;
                        f->destinationX = xRoad;
                        f->destinationY = yRoad;
                        f->roamLength = 0;
                    } else {
                        f->state = FIGURE_STATE_DEAD;
                    }
                } else {
                    const map_tile *exit = city_map_exit_point();
                    f->actionState = FIGURE_ACTION_10_HOMELESS_LEAVING;
                    f->destinationX = exit->x;
                    f->destinationY = exit->y;
                    f->roamLength = 0;
                    f->waitTicks = 0;
                }
            }
            break;
        case FIGURE_ACTION_8_HOMELESS_GOING_TO_HOUSE:
            f->isGhost = 0;
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                building_get(f->immigrantBuildingId)->immigrantFigureId = 0;
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                building *b = building_get(f->immigrantBuildingId);
                f->actionState = FIGURE_ACTION_9_HOMELESS_ENTERING_HOUSE;
                figure_movement_set_cross_country_destination(f, b->x, b->y);
                f->roamLength = 0;
            }
            break;
        case FIGURE_ACTION_9_HOMELESS_ENTERING_HOUSE:
            f->useCrossCountry = 1;
            f->isGhost = 1;
            if (figure_movement_move_ticks_cross_country(f, 1) == 1) {
                f->state = FIGURE_STATE_DEAD;
                building *b = building_get(f->immigrantBuildingId);
                if (f->immigrantBuildingId && building_is_house(b->type)) {
                    int maxPeople = model_get_house(b->subtype.houseLevel)->max_people;
                    if (b->houseIsMerged) {
                        maxPeople *= 4;
                    }
                    int room = maxPeople - b->housePopulation;
                    if (room < 0) {
                        room = 0;
                    }
                    if (room < f->migrantNumPeople) {
                        f->migrantNumPeople = room;
                    }
                    if (!b->housePopulation) {
                        building_house_change_to(b, BUILDING_HOUSE_SMALL_TENT);
                    }
                    b->housePopulation += f->migrantNumPeople;
                    b->housePopulationRoom = maxPeople - b->housePopulation;
                    city_population_add_homeless(f->migrantNumPeople);
                    b->immigrantFigureId = 0;
                }
            }
            break;
        case FIGURE_ACTION_10_HOMELESS_LEAVING:
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION || f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            }
            f->waitTicks++;
            if (f->waitTicks > 30) {
                f->waitTicks = 0;
                int building_id = closest_house_with_room(f->x, f->y);
                if (building_id > 0) {
                    building *b = building_get(building_id);
                    int xRoad, yRoad;
                    if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
                        b->immigrantFigureId = f->id;
                        f->immigrantBuildingId = building_id;
                        f->actionState = FIGURE_ACTION_8_HOMELESS_GOING_TO_HOUSE;
                        f->destinationX = xRoad;
                        f->destinationY = yRoad;
                        f->roamLength = 0;
                        figure_route_remove(f);
                    }
                }
            }
            break;
    }
    figure_image_update(f, image_group(GROUP_FIGURE_HOMELESS));
}
