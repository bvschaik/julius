#include "Building.h"

#include "CityView.h"

#include "Data/CityInfo.h"
#include "Data/State.h"

#include "building/destruction.h"
#include "building/storage.h"
#include "city/message.h"
#include "city/population.h"
#include "city/warning.h"
#include "figure/formation_legion.h"
#include "figuretype/migrant.h"
#include "figuretype/missile.h"
#include "figuretype/wall.h"
#include "game/undo.h"
#include "graphics/image.h"
#include "map/building.h"
#include "map/building_tiles.h"
#include "map/desirability.h"
#include "map/elevation.h"
#include "map/grid.h"
#include "map/random.h"
#include "map/road_access.h"
#include "map/road_network.h"
#include "map/routing.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "map/water.h"
#include "sound/effect.h"

void Building_updateHighestIds()
{
	Data_Buildings_Extra.highestBuildingIdInUse = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (building_get(i)->state != BUILDING_STATE_UNUSED) {
			Data_Buildings_Extra.highestBuildingIdInUse = i;
		}
	}
	if (Data_Buildings_Extra.highestBuildingIdInUse > Data_Buildings_Extra.highestBuildingIdEver) {
		Data_Buildings_Extra.highestBuildingIdEver = Data_Buildings_Extra.highestBuildingIdInUse;
	}
}

void Building_clearList()
{
    building_clear_all();
	Data_Buildings_Extra.highestBuildingIdEver = 0;
	Data_Buildings_Extra.createdSequence = 0;
}

void Building_GameTick_updateState()
{
	int landRecalc = 0;
	int wallRecalc = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		building *b = building_get(i);
		if (b->state == BUILDING_STATE_CREATED) {
			b->state = BUILDING_STATE_IN_USE;
		}
		if (b->state != BUILDING_STATE_IN_USE || !b->houseSize) {
			if (b->state == BUILDING_STATE_UNDO || b->state == BUILDING_STATE_DELETED_BY_PLAYER) {
				if (b->type == BUILDING_TOWER || b->type == BUILDING_GATEHOUSE) {
					wallRecalc = 1;
				}
				map_building_tiles_remove(i, b->x, b->y);
				landRecalc = 1;
				building_delete(b);
			} else if (b->state == BUILDING_STATE_RUBBLE) {
				if (b->houseSize) {
					city_population_remove_home_removed(b->housePopulation);
				}
				building_delete(b);
			} else if (b->state == BUILDING_STATE_DELETED_BY_GAME) {
				building_delete(b);
			}
		}
	}
	if (wallRecalc) {
		map_tiles_update_all_walls();
	}
	if (landRecalc) {
		map_routing_update_land();
	}
}

void Building_setDesirability()
{
	for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
		if (b->state != BUILDING_STATE_IN_USE) {
			continue;
		}
		b->desirability = map_desirability_get_max(b->x, b->y, b->size);
		if (b->isAdjacentToWater) {
			b->desirability += 10;
		}
		switch (map_elevation_at(b->gridOffset)) {
			case 0: break;
			case 1: b->desirability += 10; break;
			case 2: b->desirability += 12; break;
			case 3: b->desirability += 14; break;
			case 4: b->desirability += 16; break;
			default: b->desirability += 18; break;
		}
	}
}

void Building_decayHousesCovered()
{
	for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
		if (b->state != BUILDING_STATE_UNUSED && b->type != BUILDING_TOWER) {
			if (b->housesCovered <= 1) {
				b->housesCovered = 0;
			} else {
				b->housesCovered--;
			}
		}
	}
}

void Building_GameTick_checkAccessToRome()
{
	map_routing_calculate_distances(Data_CityInfo.entryPointX, Data_CityInfo.entryPointY);
	int problemGridOffset = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
		if (b->state != BUILDING_STATE_IN_USE) {
			continue;
		}
		int xRoad, yRoad;
		if (b->houseSize) {
			if (!map_closest_road_within_radius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
				// no road: eject people
				b->distanceFromEntry = 0;
				b->houseUnreachableTicks++;
				if (b->houseUnreachableTicks > 4) {
					if (b->housePopulation) {
						figure_create_homeless(b->x, b->y, b->housePopulation);
						b->housePopulation = 0;
						b->houseUnreachableTicks = 0;
					}
					b->state = BUILDING_STATE_UNDO;
				}
			} else if (map_routing_distance(map_grid_offset(xRoad, yRoad))) {
				// reachable from rome
				b->distanceFromEntry = map_routing_distance(map_grid_offset(xRoad, yRoad));
				b->houseUnreachableTicks = 0;
			} else if (map_closest_reachable_road_within_radius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
				b->distanceFromEntry = map_routing_distance(map_grid_offset(xRoad, yRoad));
				b->houseUnreachableTicks = 0;
			} else {
				// no reachable road in radius
				if (!b->houseUnreachableTicks) {
					problemGridOffset = b->gridOffset;
				}
				b->houseUnreachableTicks++;
				if (b->houseUnreachableTicks > 8) {
					b->distanceFromEntry = 0;
					b->houseUnreachableTicks = 0;
					b->state = BUILDING_STATE_UNDO;
				}
			}
		} else if (b->type == BUILDING_WAREHOUSE) {
			if (!Data_CityInfo.buildingTradeCenterBuildingId) {
				Data_CityInfo.buildingTradeCenterBuildingId = i;
			}
			b->distanceFromEntry = 0;
			int roadGridOffset = map_road_to_largest_network(b->x, b->y, 3, &xRoad, &yRoad);
			if (roadGridOffset >= 0) {
				b->roadNetworkId = map_road_network_get(roadGridOffset);
				b->distanceFromEntry = map_routing_distance(roadGridOffset);
				b->roadAccessX = xRoad;
				b->roadAccessY = yRoad;
			}
		} else if (b->type == BUILDING_WAREHOUSE_SPACE) {
			b->distanceFromEntry = 0;
			building *mainBuilding = building_main(b);
			b->roadNetworkId = mainBuilding->roadNetworkId;
			b->distanceFromEntry = mainBuilding->distanceFromEntry;
			b->roadAccessX = mainBuilding->roadAccessX;
			b->roadAccessY = mainBuilding->roadAccessY;
		} else if (b->type == BUILDING_HIPPODROME) {
			b->distanceFromEntry = 0;
			int roadGridOffset = map_road_to_largest_network_hippodrome(b->x, b->y, &xRoad, &yRoad);
			if (roadGridOffset >= 0) {
				b->roadNetworkId = map_road_network_get(roadGridOffset);
				b->distanceFromEntry = map_routing_distance(roadGridOffset);
				b->roadAccessX = xRoad;
				b->roadAccessY = yRoad;
			}
		} else { // other building
			b->distanceFromEntry = 0;
			int roadGridOffset = map_road_to_largest_network(b->x, b->y, b->size, &xRoad, &yRoad);
			if (roadGridOffset >= 0) {
				b->roadNetworkId = map_road_network_get(roadGridOffset);
				b->distanceFromEntry = map_routing_distance(roadGridOffset);
				b->roadAccessX = xRoad;
				b->roadAccessY = yRoad;
			}
		}
	}
	if (!map_routing_distance(Data_CityInfo.exitPointGridOffset)) {
		// no route through city
		if (Data_CityInfo.population <= 0) {
			return;
		}
		for (int i = 0; i < 15; i++) {
			map_routing_delete_first_wall_or_aqueduct(
				Data_CityInfo.entryPointX, Data_CityInfo.entryPointY);
			map_routing_delete_first_wall_or_aqueduct(
				Data_CityInfo.exitPointX, Data_CityInfo.exitPointY);
			map_routing_calculate_distances(Data_CityInfo.entryPointX, Data_CityInfo.entryPointY);

			map_tiles_update_all_walls();
			map_tiles_update_all_aqueducts(0);
			map_tiles_update_all_empty_land();
			map_tiles_update_all_meadow();
			
			map_routing_update_land();
			map_routing_update_walls();
			
			if (map_routing_distance(Data_CityInfo.exitPointGridOffset)) {
				city_message_post(1, MESSAGE_ROAD_TO_ROME_OBSTRUCTED, 0, 0);
				game_undo_disable();
				return;
			}
		}
		building_destroy_last_placed();
	} else if (problemGridOffset) {
		// parts of city disconnected
		city_warning_show(WARNING_CITY_BOXED_IN);
		city_warning_show(WARNING_CITY_BOXED_IN_PEOPLE_WILL_PERISH);
		CityView_goToGridOffset(problemGridOffset);
	}
}
