#include "migrant.h"

#include "building/building.h"
#include "building/house.h"
#include "building/model.h"
#include "figure/image.h"
#include "figure/route.h"
#include "graphics/image.h"
#include "map/road_access.h"

#include "Data/CityInfo.h"
#include "CityInfo.h"
#include "FigureAction.h"
#include "FigureMovement.h"
#include "HousePopulation.h"

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

void figure_immigrant_action(figure *f)
{
    building *b = building_get(f->immigrantBuildingId);
    
    f->terrainUsage = FigureTerrainUsage_Any;
    f->cartGraphicId = 0;
    if (!BuildingIsInUse(b) || b->immigrantFigureId != f->id || !b->houseSize) {
        f->state = FigureState_Dead;
        return;
    }
    
    figure_image_increase_offset(f, 12);
    
    switch (f->actionState) {
        case FIGURE_ACTION_150_ATTACK:
            FigureAction_Common_handleAttack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            FigureAction_Common_handleCorpse(f);
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
                    f->state = FigureState_Dead;
                }
            }
            break;
        case FIGURE_ACTION_2_IMMIGRANT_ARRIVING:
            f->isGhost = 0;
            FigureMovement_walkTicks(f, 1);
            switch (f->direction) {
                case DIR_FIGURE_AT_DESTINATION:
                    f->actionState = FIGURE_ACTION_3_IMMIGRANT_ENTERING_HOUSE;
                    FigureAction_Common_setCrossCountryDestination(f, b->x, b->y);
                    f->roamLength = 0;
                    break;
                case DIR_FIGURE_REROUTE:
                    figure_route_remove(f);
                    break;
                case DIR_FIGURE_LOST:
                    b->immigrantFigureId = 0;
                    b->distanceFromEntry = 0;
                    f->state = FigureState_Dead;
                    break;
            }
            break;
        case FIGURE_ACTION_3_IMMIGRANT_ENTERING_HOUSE:
            f->useCrossCountry = 1;
            f->isGhost = 1;
            if (FigureMovement_crossCountryWalkTicks(f, 1) == 1) {
                f->state = FigureState_Dead;
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
                CityInfo_Population_addPeople(f->migrantNumPeople);
                b->immigrantFigureId = 0;
            }
            f->isGhost = f->inBuildingWaitTicks ? 1 : 0;
            break;
    }
    
    update_direction_and_image(f);
}

void figure_emigrant_action(figure *f)
{
    f->terrainUsage = FigureTerrainUsage_Any;
    f->cartGraphicId = 0;
    
    figure_image_increase_offset(f, 12);
    
    switch (f->actionState) {
        case FIGURE_ACTION_150_ATTACK:
            FigureAction_Common_handleAttack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            FigureAction_Common_handleCorpse(f);
            break;
        case FIGURE_ACTION_4_EMIGRANT_CREATED:
            f->isGhost = 1;
            f->graphicOffset = 0;
            f->waitTicks++;
            if (f->waitTicks >= 5) {
                int xRoad, yRoad;
                if (!map_closest_road_within_radius(f->x, f->y, 1, 5, &xRoad, &yRoad)) {
                    f->state = FigureState_Dead;
                }
                f->actionState = FIGURE_ACTION_5_EMIGRANT_EXITING_HOUSE;
                FigureAction_Common_setCrossCountryDestination(f, xRoad, yRoad);
                f->roamLength = 0;
            }
            break;
        case FIGURE_ACTION_5_EMIGRANT_EXITING_HOUSE:
            f->useCrossCountry = 1;
            f->isGhost = 1;
            if (FigureMovement_crossCountryWalkTicks(f, 1) == 1) {
                f->actionState = FIGURE_ACTION_6_EMIGRANT_LEAVING;
                f->destinationX = Data_CityInfo.entryPointX;
                f->destinationY = Data_CityInfo.entryPointY;
                f->roamLength = 0;
                f->progressOnTile = 15;
            }
            f->isGhost = f->inBuildingWaitTicks ? 1 : 0;
            break;
        case FIGURE_ACTION_6_EMIGRANT_LEAVING:
            f->useCrossCountry = 0;
            f->isGhost = 0;
            FigureMovement_walkTicks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION ||
                f->direction == DIR_FIGURE_REROUTE ||
                f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
            }
            break;
    }
    update_direction_and_image(f);
}

void figure_homeless_action(figure *f)
{
    figure_image_increase_offset(f, 12);
    f->terrainUsage = FigureTerrainUsage_PreferRoads;
    
    switch (f->actionState) {
        case FIGURE_ACTION_150_ATTACK:
            FigureAction_Common_handleAttack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            FigureAction_Common_handleCorpse(f);
            break;
        case FIGURE_ACTION_7_HOMELESS_CREATED:
            f->graphicOffset = 0;
            f->waitTicks++;
            if (f->waitTicks > 51) {
                int buildingId = HousePopulation_getClosestHouseWithRoom(f->x, f->y);
                if (buildingId) {
                    building *b = building_get(buildingId);
                    int xRoad, yRoad;
                    if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
                        b->immigrantFigureId = f->id;
                        f->immigrantBuildingId = buildingId;
                        f->actionState = FIGURE_ACTION_8_HOMELESS_GOING_TO_HOUSE;
                        f->destinationX = xRoad;
                        f->destinationY = yRoad;
                        f->roamLength = 0;
                    } else {
                        f->state = FigureState_Dead;
                    }
                } else {
                    f->actionState = FIGURE_ACTION_10_HOMELESS_LEAVING;
                    f->destinationX = Data_CityInfo.exitPointX;
                    f->destinationY = Data_CityInfo.exitPointY;
                    f->roamLength = 0;
                    f->waitTicks = 0;
                }
            }
            break;
        case FIGURE_ACTION_8_HOMELESS_GOING_TO_HOUSE:
            f->isGhost = 0;
            FigureMovement_walkTicks(f, 1);
            if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                building_get(f->immigrantBuildingId)->immigrantFigureId = 0;
                f->state = FigureState_Dead;
            } else if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                building *b = building_get(f->immigrantBuildingId);
                f->actionState = FIGURE_ACTION_9_HOMELESS_ENTERING_HOUSE;
                FigureAction_Common_setCrossCountryDestination(f, b->x, b->y);
                f->roamLength = 0;
            }
            break;
        case FIGURE_ACTION_9_HOMELESS_ENTERING_HOUSE:
            f->useCrossCountry = 1;
            f->isGhost = 1;
            if (FigureMovement_crossCountryWalkTicks(f, 1) == 1) {
                f->state = FigureState_Dead;
                building *b = building_get(f->immigrantBuildingId);
                if (f->immigrantBuildingId && BuildingIsHouse(b->type)) {
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
                    CityInfo_Population_addPeopleHomeless(f->migrantNumPeople);
                    b->immigrantFigureId = 0;
                }
            }
            break;
        case FIGURE_ACTION_10_HOMELESS_LEAVING:
            FigureMovement_walkTicks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION || f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            }
            f->waitTicks++;
            if (f->waitTicks > 30) {
                f->waitTicks = 0;
                int buildingId = HousePopulation_getClosestHouseWithRoom(f->x, f->y);
                if (buildingId > 0) {
                    building *b = building_get(buildingId);
                    int xRoad, yRoad;
                    if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
                        b->immigrantFigureId = f->id;
                        f->immigrantBuildingId = buildingId;
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
