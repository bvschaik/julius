#include "Building.h"

#include "CityInfo.h"
#include "CityView.h"
#include "Formation.h"
#include "Terrain.h"
#include "TerrainGraphics.h"

#include "Data/CityInfo.h"
#include "Data/State.h"

#include "building/storage.h"
#include "city/message.h"
#include "city/warning.h"
#include "figuretype/migrant.h"
#include "figuretype/missile.h"
#include "figuretype/wall.h"
#include "game/undo.h"
#include "graphics/image.h"
#include "map/building.h"
#include "map/desirability.h"
#include "map/elevation.h"
#include "map/grid.h"
#include "map/random.h"
#include "map/road_access.h"
#include "map/road_network.h"
#include "map/routing.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/water.h"
#include "sound/effect.h"

#include <string.h>

void Building_updateHighestIds()
{
	Data_Buildings_Extra.highestBuildingIdInUse = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (building_get(i)->state != BuildingState_Unused) {
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

static void Building_deleteData(building *b)
{
	if (b->storage_id) {
		building_storage_delete(b->storage_id);
	}
	if (b->type == BUILDING_SENATE_UPGRADED && b->gridOffset == Data_CityInfo.buildingSenateGridOffset) {
		Data_CityInfo.buildingSenateGridOffset = 0;
		Data_CityInfo.buildingSenateX = 0;
		Data_CityInfo.buildingSenateY = 0;
		Data_CityInfo.buildingSenatePlaced = 0;
	}
	if (b->type == BUILDING_DOCK) {
		--Data_CityInfo.numWorkingDocks;
	}
	if (b->type == BUILDING_BARRACKS && b->gridOffset == Data_CityInfo.buildingBarracksGridOffset) {
		Data_CityInfo.buildingBarracksGridOffset = 0;
		Data_CityInfo.buildingBarracksX = 0;
		Data_CityInfo.buildingBarracksY = 0;
		Data_CityInfo.buildingBarracksPlaced = 0;
	}
	if (b->type == BUILDING_DISTRIBUTION_CENTER_UNUSED && b->gridOffset == Data_CityInfo.buildingDistributionCenterGridOffset) {
		Data_CityInfo.buildingDistributionCenterGridOffset = 0;
		Data_CityInfo.buildingDistributionCenterX = 0;
		Data_CityInfo.buildingDistributionCenterY = 0;
		Data_CityInfo.buildingDistributionCenterPlaced = 0;
	}
	if (b->type == BUILDING_FORT) {
		Formation_deleteFortAndBanner(b->formationId);
	}
	if (b->type == BUILDING_HIPPODROME) {
		Data_CityInfo.buildingHippodromePlaced = 0;
	}
}

static void Building_delete(building *b)
{
    Building_deleteData(b);
    building_delete(b);
}

void Building_GameTick_updateState()
{
	int landRecalc = 0;
	int wallRecalc = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		building *b = building_get(i);
		if (b->state == BuildingState_Created) {
			b->state = BuildingState_InUse;
		}
		if (b->state != BuildingState_InUse || !b->houseSize) {
			if (b->state == BuildingState_Undo || b->state == BuildingState_DeletedByPlayer) {
				if (b->type == BUILDING_TOWER || b->type == BUILDING_GATEHOUSE) {
					wallRecalc = 1;
				}
				Terrain_removeBuildingFromGrids(i, b->x, b->y);
				landRecalc = 1;
				Building_delete(b);
			} else if (b->state == BuildingState_Rubble) {
				if (b->houseSize) {
					CityInfo_Population_removePeopleHomeRemoved(b->housePopulation);
				}
				Building_delete(b);
			} else if (b->state == BuildingState_DeletedByGame) {
				Building_delete(b);
			}
		}
	}
	if (wallRecalc) {
		TerrainGraphics_updateAllWalls();
	}
	if (landRecalc) {
		map_routing_update_land();
	}
}

void Building_collapseOnFire(int buildingId, int hasPlague)
{
	building *b = building_get(buildingId);
	game_undo_disable();
	b->fireRisk = 0;
	b->damageRisk = 0;
	if (b->houseSize && b->housePopulation) {
		CityInfo_Population_removePeopleHomeRemoved(b->housePopulation);
	}
	// FIXED wasTent was always false because houseSize was reset above it
	int wasTent = b->houseSize && b->subtype.houseLevel <= HOUSE_LARGE_TENT;
	b->housePopulation = 0;
	b->houseSize = 0;
	b->outputResourceId = 0;
	b->distanceFromEntry = 0;
	Building_deleteData(b);

	int watersideBuilding = 0;
	if (b->type == BUILDING_DOCK || b->type == BUILDING_WHARF || b->type == BUILDING_SHIPYARD) {
		watersideBuilding = 1;
	}
	int numTiles;
	if (b->size >= 2 && b->size <= 5) {
		numTiles = b->size * b->size;
	} else {
		numTiles = 0;
	}
	Terrain_removeBuildingFromGrids(buildingId, b->x, b->y);
	if (map_terrain_is(b->gridOffset, TERRAIN_WATER)) {
		b->state = BuildingState_DeletedByGame;
	} else {
		b->type = BUILDING_BURNING_RUIN;
		b->figureId4 = 0;
		b->taxIncomeOrStorage = 0;
		b->fireDuration = (b->houseGenerationDelay & 7) + 1;
		b->fireProof = 1;
		b->size = 1;
		b->ruinHasPlague = hasPlague;
		int graphicId;
		if (wasTent) {
			graphicId = image_group(GROUP_TERRAIN_RUBBLE_TENT);
		} else {
			int random = map_random_get(b->gridOffset) & 3;
			graphicId = image_group(GROUP_TERRAIN_RUBBLE_GENERAL) + 9 * random;
		}
		Terrain_addBuildingToGrids(buildingId, b->x, b->y, 1, graphicId, TERRAIN_BUILDING);
	}
	static const int xTiles[] = {0, 1, 1, 0, 2, 2, 2, 1, 0, 3, 3, 3, 3, 2, 1, 0, 4, 4, 4, 4, 4, 3, 2, 1, 0, 5, 5, 5, 5, 5, 5, 4, 3, 2, 1, 0};
	static const int yTiles[] = {0, 0, 1, 1, 0, 1, 2, 2, 2, 0, 1, 2, 3, 3, 3, 3, 0, 1, 2, 3, 4, 4, 4, 4, 4, 0, 1, 2, 3, 4, 5, 5, 5, 5, 5, 5};
	for (int tile = 1; tile < numTiles; tile++) {
		int x = xTiles[tile] + b->x;
		int y = yTiles[tile] + b->y;
		if (map_terrain_is(map_grid_offset(x, y), TERRAIN_WATER)) {
			continue;
		}
		building *ruin = building_create(BUILDING_BURNING_RUIN, x, y);
		int graphicId;
		if (wasTent) {
			graphicId = image_group(GROUP_TERRAIN_RUBBLE_TENT);
		} else {
			int random = map_random_get(ruin->gridOffset) & 3;
			graphicId = image_group(GROUP_TERRAIN_RUBBLE_GENERAL) + 9 * random;
		}
		Terrain_addBuildingToGrids(ruin->id, ruin->x, ruin->y, 1, graphicId, TERRAIN_BUILDING);
		ruin->fireDuration = (ruin->houseGenerationDelay & 7) + 1;
		ruin->figureId4 = 0;
		ruin->fireProof = 1;
		ruin->ruinHasPlague = hasPlague;
	}
	if (watersideBuilding) {
		map_routing_update_water();
	}
}

void Building_collapseLinked(int buildingId, int onFire)
{
	building *space = building_get(buildingId);
	for (int i = 0; i < 9; i++) {
		if (space->prevPartBuildingId <= 0) {
			break;
		}
		int spaceId = space->prevPartBuildingId;
		space = building_get(spaceId);
		if (onFire) {
			Building_collapseOnFire(spaceId, 0);
		} else {
			TerrainGraphics_setBuildingAreaRubble(spaceId, space->x, space->y, space->size);
			space->state = BuildingState_Rubble;
		}
	}

	space = building_get(buildingId);
	for (int i = 0; i < 9; i++) {
        space = building_next(space);
		if (space->id <= 0) {
			break;
		}
		if (onFire) {
			Building_collapseOnFire(space->id, 0);
		} else {
			TerrainGraphics_setBuildingAreaRubble(space->id, space->x, space->y, space->size);
			space->state = BuildingState_Rubble;
		}
	}
}

void Building_collapseLastPlaced()
{
	int highestSequence = 0;
	int buildingId = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
		if (b->state == BuildingState_Created || b->state == BuildingState_InUse) {
			if (b->createdSequence > highestSequence) {
				highestSequence = b->createdSequence;
				buildingId = i;
			}
		}
	}
	if (buildingId) {
        building *b = building_get(buildingId);
		city_message_post(1, MESSAGE_ROAD_TO_ROME_BLOCKED, 0, b->gridOffset);
		game_undo_disable();
		b->state = BuildingState_Rubble;
		TerrainGraphics_setBuildingAreaRubble(buildingId, b->x, b->y, b->size);
		figure_create_explosion_cloud(b->x, b->y, b->size);
		Building_collapseLinked(buildingId, 0);
		map_routing_update_land();
	}
}

int Building_collapseFirstOfType(int buildingType)
{
	for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
		if (BuildingIsInUse(b) && b->type == buildingType) {
			int gridOffset = b->gridOffset;
			game_undo_disable();
			b->state = BuildingState_Rubble;
			
			TerrainGraphics_setBuildingAreaRubble(i, b->x, b->y, b->size);
			sound_effect_play(SOUND_EFFECT_EXPLOSION);
			map_routing_update_land();
			return gridOffset;
		}
	}
	return 0;
}

void Building_increaseDamageByEnemy(int gridOffset, int maxDamage)
{
	if (map_building_damage_increase(gridOffset) > maxDamage) {
		Building_destroyByEnemy(map_grid_offset_to_x(gridOffset),
			map_grid_offset_to_y(gridOffset), gridOffset);
	}
}

void Building_destroyByEnemy(int x, int y, int gridOffset)
{
	int buildingId = map_building_at(gridOffset);
	if (buildingId > 0) {
		building *b = building_get(buildingId);
		TerrainGraphics_setBuildingAreaRubble(buildingId, b->x, b->y, b->size);
		if (BuildingIsInUse(b)) {
			switch (b->type) {
				case BUILDING_HOUSE_SMALL_TENT:
				case BUILDING_HOUSE_LARGE_TENT:
				case BUILDING_PREFECTURE:
				case BUILDING_ENGINEERS_POST:
				case BUILDING_WELL:
				case BUILDING_FORT:
				case BUILDING_FORT_GROUND:
				case BUILDING_GATEHOUSE:
				case BUILDING_TOWER:
					break;
				default:
					Data_CityInfo.ratingPeaceNumDestroyedBuildingsThisYear++;
					break;
			}
			if (Data_CityInfo.ratingPeaceNumDestroyedBuildingsThisYear >= 12) {
				Data_CityInfo.ratingPeaceNumDestroyedBuildingsThisYear = 12;
			}
			b->state = BuildingState_Rubble;
			figure_create_explosion_cloud(b->x, b->y, b->size);
			Building_collapseLinked(buildingId, 0);
		}
	} else {
		if (map_terrain_is(gridOffset, TERRAIN_WALL)) {
			figure_kill_tower_sentries_at(x, y);
		}
		TerrainGraphics_setBuildingAreaRubble(0, x, y, 1);
	}
	figure_tower_sentry_reroute();
	TerrainGraphics_updateAreaWalls(x, y, 3);
	TerrainGraphics_updateRegionAqueduct(x - 2, y - 2, x + 2, y + 2, 0);
	map_routing_update_land();
	map_routing_update_walls();
}

void Building_setDesirability()
{
	for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
		if (!BuildingIsInUse(b)) {
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
		if (b->state != BuildingState_Unused && b->type != BUILDING_TOWER) {
			if (b->housesCovered <= 1) {
				b->housesCovered = 0;
			} else {
				b->housesCovered--;
			}
		}
	}
}

void Building_decayTaxCollectorAccess()
{
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (BuildingIsInUse(b) && b->houseTaxCoverage) {
            b->houseTaxCoverage--;
        }
    }
}

void Building_determineGraphicIdsForOrientedBuildings()
{
	int mapOrientation = Data_State.map.orientation;
	int mapOrientationIsTopOrBottom = mapOrientation == DIR_0_TOP || mapOrientation == DIR_4_BOTTOM;
	int graphicOffset;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		building *b = building_get(i);
		if (b->state == BuildingState_Unused) {
			continue;
		}
		int image_id;
		switch (b->type) {
			case BUILDING_GATEHOUSE:
				if (b->subtype.orientation == 1) {
					if (mapOrientationIsTopOrBottom) {
						image_id = image_group(GROUP_BUILDING_TOWER) + 1;
					} else {
						image_id = image_group(GROUP_BUILDING_TOWER) + 2;
					}
				} else {
					if (mapOrientationIsTopOrBottom) {
						image_id = image_group(GROUP_BUILDING_TOWER) + 2;
					} else {
						image_id = image_group(GROUP_BUILDING_TOWER) + 1;
					}
				}
				Terrain_addBuildingToGrids(i, b->x, b->y, b->size,
					                   image_id, TERRAIN_GATEHOUSE | TERRAIN_BUILDING);
				Terrain_addRoadsForGatehouse(b->x, b->y, b->subtype.orientation);
				break;
			case BUILDING_TRIUMPHAL_ARCH:
				if (b->subtype.orientation == 1) {
					if (mapOrientationIsTopOrBottom) {
						image_id = image_group(GROUP_BUILDING_TRIUMPHAL_ARCH);
					} else {
						image_id = image_group(GROUP_BUILDING_TRIUMPHAL_ARCH) + 2;
					}
				} else {
					if (mapOrientationIsTopOrBottom) {
						image_id = image_group(GROUP_BUILDING_TRIUMPHAL_ARCH) + 2;
					} else {
						image_id = image_group(GROUP_BUILDING_TRIUMPHAL_ARCH);
					}
				}
				Terrain_addBuildingToGrids(i, b->x, b->y, b->size,
					                   image_id, TERRAIN_BUILDING);
				Terrain_addRoadsForTriumphalArch(b->x, b->y, b->subtype.orientation);
				break;
			case BUILDING_HIPPODROME:
				if (mapOrientation == DIR_0_TOP) {
					image_id = image_group(GROUP_BUILDING_HIPPODROME_2);
					switch (b->subtype.orientation) {
						case 0: case 3: 
                    image_id += 0; break;
						case 1: case 4: 
                    image_id += 2; break;
						case 2: case 5: 
                    image_id += 4; break;
					}
				} else if (mapOrientation == DIR_4_BOTTOM) {
					image_id = image_group(GROUP_BUILDING_HIPPODROME_2);
					switch (b->subtype.orientation) {
						case 0: case 3: 
                    image_id += 4; break;
						case 1: case 4: 
                    image_id += 2; break;
						case 2: case 5: 
                    image_id += 0; break;
					}
				} else if (mapOrientation == DIR_6_LEFT) {
					image_id = image_group(GROUP_BUILDING_HIPPODROME_1);
					switch (b->subtype.orientation) {
						case 0: case 3: 
                    image_id += 0; break;
						case 1: case 4: 
                    image_id += 2; break;
						case 2: case 5: 
                    image_id += 4; break;
					}
				} else { // DIR_2_RIGHT
					image_id = image_group(GROUP_BUILDING_HIPPODROME_1);
					switch (b->subtype.orientation) {
						case 0: case 3: 
                    image_id += 4; break;
						case 1: case 4: 
                    image_id += 2; break;
						case 2: case 5: 
                    image_id += 0; break;
					}
				}
				Terrain_addBuildingToGrids(i, b->x, b->y, b->size,
					                   image_id, TERRAIN_BUILDING);
				break;
			case BUILDING_SHIPYARD:
				graphicOffset = (4 + b->data.other.dockOrientation - mapOrientation / 2) % 4;
				image_id = image_group(GROUP_BUILDING_SHIPYARD) + graphicOffset;
				map_water_add_building(i, b->x, b->y, 2, image_id);
				break;
			case BUILDING_WHARF:
				graphicOffset = (4 + b->data.other.dockOrientation - mapOrientation / 2) % 4;
				image_id = image_group(GROUP_BUILDING_WHARF) + graphicOffset;
				map_water_add_building(i, b->x, b->y, 2, image_id);
				break;
			case BUILDING_DOCK:
				graphicOffset = (4 + b->data.other.dockOrientation - mapOrientation / 2) % 4;
				switch (graphicOffset) {
					case 0: 
                image_id = image_group(GROUP_BUILDING_DOCK_1); break;
					case 1: 
                image_id = image_group(GROUP_BUILDING_DOCK_2); break;
					case 2: 
                image_id = image_group(GROUP_BUILDING_DOCK_3); break;
					default: 
                image_id = image_group(GROUP_BUILDING_DOCK_4); break;
				}
				map_water_add_building(i, b->x, b->y, 3, image_id);
				break;
		}
	}
}

void Building_GameTick_checkAccessToRome()
{
	map_routing_calculate_distances(Data_CityInfo.entryPointX, Data_CityInfo.entryPointY);
	int problemGridOffset = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
		if (!BuildingIsInUse(b)) {
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
					b->state = BuildingState_Undo;
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
					b->state = BuildingState_Undo;
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

			TerrainGraphics_updateAllWalls();
			TerrainGraphics_updateRegionAqueduct(0, 0,
				Data_State.map.width - 1, Data_State.map.height - 1, 0);
			TerrainGraphics_updateRegionEmptyLand(0, 0,
				Data_State.map.width - 1, Data_State.map.height - 1);
			TerrainGraphics_updateRegionMeadow(0, 0,
				Data_State.map.width - 1, Data_State.map.height - 1);
			
			map_routing_update_land();
			map_routing_update_walls();
			
			if (map_routing_distance(Data_CityInfo.exitPointGridOffset)) {
				city_message_post(1, MESSAGE_ROAD_TO_ROME_OBSTRUCTED, 0, 0);
				game_undo_disable();
				return;
			}
		}
		Building_collapseLastPlaced();
	} else if (problemGridOffset) {
		// parts of city disconnected
		city_warning_show(WARNING_CITY_BOXED_IN);
		city_warning_show(WARNING_CITY_BOXED_IN_PEOPLE_WILL_PERISH);
		CityView_goToGridOffset(problemGridOffset);
	}
}
