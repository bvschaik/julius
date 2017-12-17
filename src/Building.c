#include "Building.h"

#include "CityInfo.h"
#include "CityView.h"
#include "Figure.h"
#include "FigureAction.h"
#include "Formation.h"
#include "HousePopulation.h"
#include "Resource.h"
#include "Terrain.h"
#include "TerrainGraphics.h"
#include "Undo.h"

#include "Data/CityInfo.h"
#include "Data/State.h"

#include "building/building.h"
#include "building/properties.h"
#include "building/storage.h"
#include "city/message.h"
#include "city/warning.h"
#include "core/calc.h"
#include "core/direction.h"
#include "figure/figure.h"
#include "graphics/image.h"
#include "map/building.h"
#include "map/desirability.h"
#include "map/elevation.h"
#include "map/grid.h"
#include "map/random.h"
#include "map/road_network.h"
#include "map/routing.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "scenario/map.h"
#include "scenario/property.h"
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
	memset(Data_Buildings, 0, MAX_BUILDINGS * sizeof(struct Data_Building));
	Data_Buildings_Extra.highestBuildingIdEver = 0;
	Data_Buildings_Extra.createdSequence = 0;
}

int Building_create(int type, int x, int y)
{
	int buildingId = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (building_get(i)->state == BuildingState_Unused && !Undo_isBuildingInList(i)) {
			buildingId = i;
			break;
		}
	}
	if (!buildingId) {
		city_warning_show(WARNING_DATA_LIMIT_REACHED);
		return 0;
	}
	
	struct Data_Building *b = building_get(buildingId);
    const building_properties *props = building_properties_for_type(type);
	
	b->state = BuildingState_Created;
	b->ciid = 1;
	b->__unknown_02 = Data_CityInfo.__unknown_00a5; // TODO ??
	b->type = type;
	b->size = props->size;
	b->createdSequence = Data_Buildings_Extra.createdSequence++;
	b->sentiment.houseHappiness = 50;
	b->distanceFromEntry = 0;
	
	// house size
	b->houseSize = 0;
	if (type >= BUILDING_HOUSE_SMALL_TENT && type <= BUILDING_HOUSE_MEDIUM_INSULA) {
		b->houseSize = 1;
	} else if (type >= BUILDING_HOUSE_LARGE_INSULA && type <= BUILDING_HOUSE_MEDIUM_VILLA) {
		b->houseSize = 2;
	} else if (type >= BUILDING_HOUSE_LARGE_VILLA && type <= BUILDING_HOUSE_MEDIUM_PALACE) {
		b->houseSize = 3;
	} else if (type >= BUILDING_HOUSE_LARGE_PALACE && type <= BUILDING_HOUSE_LUXURY_PALACE) {
		b->houseSize = 4;
	}
	
	// subtype
	if (BuildingIsHouse(type)) {
		b->subtype.houseLevel = type - 10;
	} else {
		b->subtype.houseLevel = 0;
	}
	
	// input/output resources
	switch (type) {
		case BUILDING_WHEAT_FARM:
			b->outputResourceId = RESOURCE_WHEAT;
			break;
		case BUILDING_VEGETABLE_FARM:
			b->outputResourceId = RESOURCE_VEGETABLES;
			break;
		case BUILDING_FRUIT_FARM:
			b->outputResourceId = RESOURCE_FRUIT;
			break;
		case BUILDING_OLIVE_FARM:
			b->outputResourceId = RESOURCE_OLIVES;
			break;
		case BUILDING_VINES_FARM:
			b->outputResourceId = RESOURCE_VINES;
			break;
		case BUILDING_PIG_FARM:
			b->outputResourceId = RESOURCE_MEAT;
			break;
		case BUILDING_MARBLE_QUARRY:
			b->outputResourceId = RESOURCE_MARBLE;
			break;
		case BUILDING_IRON_MINE:
			b->outputResourceId = RESOURCE_IRON;
			break;
		case BUILDING_TIMBER_YARD:
			b->outputResourceId = RESOURCE_TIMBER;
			break;
		case BUILDING_CLAY_PIT:
			b->outputResourceId = RESOURCE_CLAY;
			break;
		case BUILDING_WINE_WORKSHOP:
			b->outputResourceId = RESOURCE_WINE;
			b->subtype.workshopType = WORKSHOP_VINES_TO_WINE;
			break;
		case BUILDING_OIL_WORKSHOP:
			b->outputResourceId = RESOURCE_OIL;
			b->subtype.workshopType = WORKSHOP_OLIVES_TO_OIL;
			break;
		case BUILDING_WEAPONS_WORKSHOP:
			b->outputResourceId = RESOURCE_WEAPONS;
			b->subtype.workshopType = WORKSHOP_IRON_TO_WEAPONS;
			break;
		case BUILDING_FURNITURE_WORKSHOP:
			b->outputResourceId = RESOURCE_FURNITURE;
			b->subtype.workshopType = WORKSHOP_TIMBER_TO_FURNITURE;
			break;
		case BUILDING_POTTERY_WORKSHOP:
			b->outputResourceId = RESOURCE_POTTERY;
			b->subtype.workshopType = WORKSHOP_CLAY_TO_POTTERY;
			break;
		default:
			b->outputResourceId = RESOURCE_NONE;
			break;
	}
	
	if (type == BUILDING_GRANARY) {
		b->data.storage.resourceStored[RESOURCE_NONE] = 2400;
	}
	
	b->x = x;
	b->y = y;
	b->gridOffset = map_grid_offset(x, y);
	b->houseGenerationDelay = map_random_get(b->gridOffset) & 0x7f;
	b->figureRoamDirection = b->houseGenerationDelay & 6;
	b->fireProof = props->fire_proof;
	b->isAdjacentToWater = Terrain_isAdjacentToWater(x, y, b->size);

	return buildingId;
}

void Building_delete(int buildingId)
{
	Building_deleteData(buildingId);
	memset(&Data_Buildings[buildingId], 0, sizeof(struct Data_Building));
}

void Building_deleteData(int buildingId)
{
	struct Data_Building *b = building_get(buildingId);
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

void Building_GameTick_updateState()
{
	int landRecalc = 0;
	int wallRecalc = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = building_get(i);
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
				Building_delete(i);
			} else if (b->state == BuildingState_Rubble) {
				if (b->houseSize) {
					CityInfo_Population_removePeopleHomeRemoved(b->housePopulation);
				}
				Building_delete(i);
			} else if (b->state == BuildingState_DeletedByGame) {
				Building_delete(i);
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

int Building_getMainBuildingId(int buildingId)
{
	for (int guard = 0; guard < 9; guard++) {
        struct Data_Building *b = building_get(buildingId);
		if (b->prevPartBuildingId <= 0) {
			return buildingId;
		}
		buildingId = b->prevPartBuildingId;
	}
	return 0;
}

void Building_collapseOnFire(int buildingId, int hasPlague)
{
	struct Data_Building *b = building_get(buildingId);
	Data_State.undoAvailable = 0;
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
	Building_deleteData(buildingId);

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
		int ruinId = Building_create(BUILDING_BURNING_RUIN, x, y);
		struct Data_Building *ruin = building_get(ruinId);
		int graphicId;
		if (wasTent) {
			graphicId = image_group(GROUP_TERRAIN_RUBBLE_TENT);
		} else {
			int random = map_random_get(ruin->gridOffset) & 3;
			graphicId = image_group(GROUP_TERRAIN_RUBBLE_GENERAL) + 9 * random;
		}
		Terrain_addBuildingToGrids(ruinId, ruin->x, ruin->y, 1, graphicId, TERRAIN_BUILDING);
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
	struct Data_Building *space = building_get(buildingId);
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
		if (space->nextPartBuildingId <= 0) {
			break;
		}
		int spaceId = space->nextPartBuildingId;
		space = building_get(spaceId);
		if (onFire) {
			Building_collapseOnFire(spaceId, 0);
		} else {
			TerrainGraphics_setBuildingAreaRubble(spaceId, space->x, space->y, space->size);
			space->state = BuildingState_Rubble;
		}
	}
}

void Building_collapseLastPlaced()
{
	int highestSequence = 0;
	int buildingId = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
        struct Data_Building *b = building_get(i);
		if (b->state == BuildingState_Created || b->state == BuildingState_InUse) {
			if (b->createdSequence > highestSequence) {
				highestSequence = b->createdSequence;
				buildingId = i;
			}
		}
	}
	if (buildingId) {
        struct Data_Building *b = building_get(buildingId);
		city_message_post(1, MESSAGE_ROAD_TO_ROME_BLOCKED, 0, b->gridOffset);
		Data_State.undoAvailable = 0;
		b->state = BuildingState_Rubble;
		TerrainGraphics_setBuildingAreaRubble(buildingId, b->x, b->y, b->size);
		Figure_createDustCloud(b->x, b->y, b->size);
		Building_collapseLinked(buildingId, 0);
		map_routing_update_land();
	}
}

int Building_collapseFirstOfType(int buildingType)
{
	for (int i = 1; i < MAX_BUILDINGS; i++) {
        struct Data_Building *b = building_get(i);
		if (BuildingIsInUse(i) && b->type == buildingType) {
			int gridOffset = b->gridOffset;
			Data_State.undoAvailable = 0;
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
		struct Data_Building *b = building_get(buildingId);
		TerrainGraphics_setBuildingAreaRubble(buildingId, b->x, b->y, b->size);
		if (BuildingIsInUse(buildingId)) {
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
			Figure_createDustCloud(b->x, b->y, b->size);
			Building_collapseLinked(buildingId, 0);
		}
	} else {
		if (map_terrain_is(gridOffset, TERRAIN_WALL)) {
			Figure_killTowerSentriesAt(x, y);
		}
		TerrainGraphics_setBuildingAreaRubble(0, x, y, 1);
	}
	FigureAction_TowerSentry_reroute();
	TerrainGraphics_updateAreaWalls(x, y, 3);
	TerrainGraphics_updateRegionAqueduct(x - 2, y - 2, x + 2, y + 2, 0);
	map_routing_update_land();
	map_routing_update_walls();
}

void Building_setDesirability()
{
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (!BuildingIsInUse(i)) {
			continue;
		}
		struct Data_Building *b = building_get(i);
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
        struct Data_Building *b = building_get(i);
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
        struct Data_Building *b = building_get(i);
        if (BuildingIsInUse(i) && b->houseTaxCoverage) {
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
		struct Data_Building *b = building_get(i);
		if (b->state == BuildingState_Unused) {
			continue;
		}
		int graphicId;
		switch (b->type) {
			case BUILDING_GATEHOUSE:
				if (b->subtype.orientation == 1) {
					if (mapOrientationIsTopOrBottom) {
						graphicId = image_group(GROUP_BUILDING_TOWER) + 1;
					} else {
						graphicId = image_group(GROUP_BUILDING_TOWER) + 2;
					}
				} else {
					if (mapOrientationIsTopOrBottom) {
						graphicId = image_group(GROUP_BUILDING_TOWER) + 2;
					} else {
						graphicId = image_group(GROUP_BUILDING_TOWER) + 1;
					}
				}
				Terrain_addBuildingToGrids(i, b->x, b->y, b->size,
					graphicId, TERRAIN_GATEHOUSE | TERRAIN_BUILDING);
				Terrain_addRoadsForGatehouse(b->x, b->y, b->subtype.orientation);
				break;
			case BUILDING_TRIUMPHAL_ARCH:
				if (b->subtype.orientation == 1) {
					if (mapOrientationIsTopOrBottom) {
						graphicId = image_group(GROUP_BUILDING_TRIUMPHAL_ARCH);
					} else {
						graphicId = image_group(GROUP_BUILDING_TRIUMPHAL_ARCH) + 2;
					}
				} else {
					if (mapOrientationIsTopOrBottom) {
						graphicId = image_group(GROUP_BUILDING_TRIUMPHAL_ARCH) + 2;
					} else {
						graphicId = image_group(GROUP_BUILDING_TRIUMPHAL_ARCH);
					}
				}
				Terrain_addBuildingToGrids(i, b->x, b->y, b->size,
					graphicId, TERRAIN_BUILDING);
				Terrain_addRoadsForTriumphalArch(b->x, b->y, b->subtype.orientation);
				break;
			case BUILDING_HIPPODROME:
				if (mapOrientation == DIR_0_TOP) {
					graphicId = image_group(GROUP_BUILDING_HIPPODROME_2);
					switch (b->subtype.orientation) {
						case 0: case 3: graphicId += 0; break;
						case 1: case 4: graphicId += 2; break;
						case 2: case 5: graphicId += 4; break;
					}
				} else if (mapOrientation == DIR_4_BOTTOM) {
					graphicId = image_group(GROUP_BUILDING_HIPPODROME_2);
					switch (b->subtype.orientation) {
						case 0: case 3: graphicId += 4; break;
						case 1: case 4: graphicId += 2; break;
						case 2: case 5: graphicId += 0; break;
					}
				} else if (mapOrientation == DIR_6_LEFT) {
					graphicId = image_group(GROUP_BUILDING_HIPPODROME_1);
					switch (b->subtype.orientation) {
						case 0: case 3: graphicId += 0; break;
						case 1: case 4: graphicId += 2; break;
						case 2: case 5: graphicId += 4; break;
					}
				} else { // DIR_2_RIGHT
					graphicId = image_group(GROUP_BUILDING_HIPPODROME_1);
					switch (b->subtype.orientation) {
						case 0: case 3: graphicId += 4; break;
						case 1: case 4: graphicId += 2; break;
						case 2: case 5: graphicId += 0; break;
					}
				}
				Terrain_addBuildingToGrids(i, b->x, b->y, b->size,
					graphicId, TERRAIN_BUILDING);
				break;
			case BUILDING_SHIPYARD:
				graphicOffset = (4 + b->data.other.dockOrientation - mapOrientation / 2) % 4;
				graphicId = image_group(GROUP_BUILDING_SHIPYARD) + graphicOffset;
				Terrain_addWatersideBuildingToGrids(i, b->x, b->y, 2, graphicId);
				break;
			case BUILDING_WHARF:
				graphicOffset = (4 + b->data.other.dockOrientation - mapOrientation / 2) % 4;
				graphicId = image_group(GROUP_BUILDING_WHARF) + graphicOffset;
				Terrain_addWatersideBuildingToGrids(i, b->x, b->y, 2, graphicId);
				break;
			case BUILDING_DOCK:
				graphicOffset = (4 + b->data.other.dockOrientation - mapOrientation / 2) % 4;
				switch (graphicOffset) {
					case 0: graphicId = image_group(GROUP_BUILDING_DOCK_1); break;
					case 1: graphicId = image_group(GROUP_BUILDING_DOCK_2); break;
					case 2: graphicId = image_group(GROUP_BUILDING_DOCK_3); break;
					default: graphicId = image_group(GROUP_BUILDING_DOCK_4); break;
				}
				Terrain_addWatersideBuildingToGrids(i, b->x, b->y, 3, graphicId);
				break;
		}
	}
}

void Building_GameTick_checkAccessToRome()
{
	map_routing_calculate_distances(Data_CityInfo.entryPointX, Data_CityInfo.entryPointY);
	int problemGridOffset = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (!BuildingIsInUse(i)) {
			continue;
		}
		struct Data_Building *b = building_get(i);
		int xRoad, yRoad;
		if (b->houseSize) {
			if (!Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
				// no road: eject people
				b->distanceFromEntry = 0;
				b->houseUnreachableTicks++;
				if (b->houseUnreachableTicks > 4) {
					if (b->housePopulation) {
						HousePopulation_createHomeless(b->x, b->y, b->housePopulation);
						b->housePopulation = 0;
						b->houseUnreachableTicks = 0;
					}
					b->state = BuildingState_Undo;
				}
			} else if (map_routing_distance(map_grid_offset(xRoad, yRoad))) {
				// reachable from rome
				b->distanceFromEntry = map_routing_distance(map_grid_offset(xRoad, yRoad));
				b->houseUnreachableTicks = 0;
			} else if (Terrain_getClosestReachableRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
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
			int roadGridOffset = Terrain_getRoadToLargestRoadNetwork(b->x, b->y, 3, &xRoad, &yRoad);
			if (roadGridOffset >= 0) {
				b->roadNetworkId = map_road_network_get(roadGridOffset);
				b->distanceFromEntry = map_routing_distance(roadGridOffset);
				b->roadAccessX = xRoad;
				b->roadAccessY = yRoad;
			}
		} else if (b->type == BUILDING_WAREHOUSE_SPACE) {
			b->distanceFromEntry = 0;
			struct Data_Building *mainBuilding = building_get(Building_getMainBuildingId(i));
			b->roadNetworkId = mainBuilding->roadNetworkId;
			b->distanceFromEntry = mainBuilding->distanceFromEntry;
			b->roadAccessX = mainBuilding->roadAccessX;
			b->roadAccessY = mainBuilding->roadAccessY;
		} else if (b->type == BUILDING_HIPPODROME) {
			b->distanceFromEntry = 0;
			int roadGridOffset = Terrain_getRoadToLargestRoadNetworkHippodrome(b->x, b->y, 5, &xRoad, &yRoad);
			if (roadGridOffset >= 0) {
				b->roadNetworkId = map_road_network_get(roadGridOffset);
				b->distanceFromEntry = map_routing_distance(roadGridOffset);
				b->roadAccessX = xRoad;
				b->roadAccessY = yRoad;
			}
		} else { // other building
			b->distanceFromEntry = 0;
			int roadGridOffset = Terrain_getRoadToLargestRoadNetwork(b->x, b->y, b->size, &xRoad, &yRoad);
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
				Data_State.undoAvailable = 0;
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

void Building_Industry_updateProduction()
{
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = building_get(i);
		if (!BuildingIsInUse(i) || !b->outputResourceId) {
			continue;
		}
		b->data.industry.hasFullResource = 0;
		if (b->housesCovered <= 0 || b->numWorkers <= 0) {
			continue;
		}
		if (b->subtype.workshopType && !b->loadsStored) {
			continue;
		}
		if (b->data.industry.curseDaysLeft) {
			b->data.industry.curseDaysLeft--;
		} else {
			if (b->data.industry.blessingDaysLeft) {
				b->data.industry.blessingDaysLeft--;
			}
			if (b->type == BUILDING_MARBLE_QUARRY) {
				b->data.industry.progress += b->numWorkers / 2;
			} else {
				b->data.industry.progress += b->numWorkers;
			}
			if (b->data.industry.blessingDaysLeft && BuildingIsFarm(b->type)) {
				b->data.industry.progress += b->numWorkers;
			}
			int maxProgress = b->subtype.workshopType ? 400 : 200;
			if (b->data.industry.progress > maxProgress) {
				b->data.industry.progress = maxProgress;
			}
			if (BuildingIsFarm(b->type)) {
				TerrainGraphics_setBuildingFarm(i, b->x, b->y,
					image_group(GROUP_BUILDING_FARM_CROPS) + 5 * (b->outputResourceId - 1),
					b->data.industry.progress);
			}
		}
	}
}

void Building_Industry_updateDoubleWheatProduction()
{
	if (scenario_property_climate() == CLIMATE_NORTHERN) {
		return;
	}
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = building_get(i);
		if (!BuildingIsInUse(i) || !b->outputResourceId) {
			continue;
		}
		if (b->housesCovered <= 0 || b->numWorkers <= 0) {
			continue;
		}
		if (b->type == BUILDING_WHEAT_FARM && !b->data.industry.curseDaysLeft) {
			b->data.industry.progress += b->numWorkers;
			if (b->data.industry.blessingDaysLeft) {
				b->data.industry.progress += b->numWorkers;
			}
			if (b->data.industry.progress > 200) {
				b->data.industry.progress = 200;
			}
			TerrainGraphics_setBuildingFarm(i, b->x, b->y,
				image_group(GROUP_BUILDING_FARM_CROPS) + 5 * (b->outputResourceId - 1),
				b->data.industry.progress);
		}
	}
}

void Building_Industry_witherFarmCropsFromCeres(int bigCurse)
{
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = building_get(i);
		if (BuildingIsInUse(i) && b->outputResourceId && BuildingIsFarm(b->type)) {
			b->data.industry.progress = 0;
			b->data.industry.blessingDaysLeft = 0;
			b->data.industry.curseDaysLeft = bigCurse ? 48 : 4;
			TerrainGraphics_setBuildingFarm(i, b->x, b->y,
				image_group(GROUP_BUILDING_FARM_CROPS) + 5 * (b->outputResourceId - 1),
				b->data.industry.progress);
		}
	}
}

void Building_Industry_blessFarmsFromCeres()
{
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = building_get(i);
		if (BuildingIsInUse(i) && b->outputResourceId && BuildingIsFarm(b->type)) {
			b->data.industry.progress = 200;
			b->data.industry.curseDaysLeft = 0;
			b->data.industry.blessingDaysLeft = 16;
			TerrainGraphics_setBuildingFarm(i, b->x, b->y,
				image_group(GROUP_BUILDING_FARM_CROPS) + 5 * (b->outputResourceId - 1),
				b->data.industry.progress);
		}
	}
}

int Building_Industry_hasProducedResource(int buildingId)
{
    struct Data_Building *b = building_get(buildingId);
	int target = b->subtype.workshopType ? 400 : 200;
	return b->data.industry.progress >= target;
}

void Building_Industry_startNewProduction(int buildingId)
{
	struct Data_Building *b = building_get(buildingId);
	b->data.industry.progress = 0;
	if (b->subtype.workshopType) {
		if (b->loadsStored) {
			if (b->loadsStored > 1) {
				b->data.industry.hasFullResource = 1;
			}
			b->loadsStored--;
		}
	}
	if (BuildingIsFarm(b->type)) {
		TerrainGraphics_setBuildingFarm(buildingId, b->x, b->y,
			image_group(GROUP_BUILDING_FARM_CROPS) + 5 * (b->outputResourceId - 1),
			b->data.industry.progress);
	}
}

int Building_Market_getDestinationGranaryWarehouse(int marketId)
{
	struct {
		int buildingId;
		int distance;
		int numBuildings;
	} resources[INVENTORY_MAX];
	for (int i = 0; i < INVENTORY_MAX; i++) {
		resources[i].buildingId = 0;
		resources[i].numBuildings = 0;
		resources[i].distance = 40;
	}
	struct Data_Building *market = building_get(marketId);
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (!BuildingIsInUse(i)) {
			continue;
		}
		struct Data_Building *b = building_get(i);
		if (b->type != BUILDING_GRANARY && b->type != BUILDING_WAREHOUSE) {
			continue;
		}
		if (!b->hasRoadAccess || b->distanceFromEntry <= 0 ||
			b->roadNetworkId != market->roadNetworkId) {
			continue;
		}
		int distance = calc_maximum_distance(market->x, market->y, b->x, b->y);
		if (distance >= 40) {
			continue;
		}
		if (b->type == BUILDING_GRANARY) {
			if (scenario_property_rome_supplies_wheat()) {
				continue;
			}
			// foods
			if (b->data.storage.resourceStored[RESOURCE_WHEAT]) {
				resources[INVENTORY_WHEAT].numBuildings++;
				if (distance < resources[INVENTORY_WHEAT].distance) {
					resources[INVENTORY_WHEAT].distance = distance;
					resources[INVENTORY_WHEAT].buildingId = i;
				}
			}
			if (b->data.storage.resourceStored[RESOURCE_VEGETABLES]) {
				resources[INVENTORY_VEGETABLES].numBuildings++;
				if (distance < resources[INVENTORY_VEGETABLES].distance) {
					resources[INVENTORY_VEGETABLES].distance = distance;
					resources[INVENTORY_VEGETABLES].buildingId = i;
				}
			}
			if (b->data.storage.resourceStored[RESOURCE_FRUIT]) {
				resources[INVENTORY_FRUIT].numBuildings++;
				if (distance < resources[INVENTORY_FRUIT].distance) {
					resources[INVENTORY_FRUIT].distance = distance;
					resources[INVENTORY_FRUIT].buildingId = i;
				}
			}
			if (b->data.storage.resourceStored[RESOURCE_MEAT]) {
				resources[INVENTORY_MEAT].numBuildings++;
				if (distance < resources[INVENTORY_MEAT].distance) {
					resources[INVENTORY_MEAT].distance = distance;
					resources[INVENTORY_MEAT].buildingId = i;
				}
			}
		} else if (b->type == BUILDING_WAREHOUSE) {
			// goods
			if (!Data_CityInfo.resourceStockpiled[RESOURCE_WINE] &&
				Resource_getAmountStoredInWarehouse(i, RESOURCE_WINE) > 0) {
				resources[INVENTORY_WINE].numBuildings++;
				if (distance < resources[INVENTORY_WINE].distance) {
					resources[INVENTORY_WINE].distance = distance;
					resources[INVENTORY_WINE].buildingId = i;
				}
			}
			if (!Data_CityInfo.resourceStockpiled[RESOURCE_OIL] &&
				Resource_getAmountStoredInWarehouse(i, RESOURCE_OIL) > 0) {
				resources[INVENTORY_OIL].numBuildings++;
				if (distance < resources[INVENTORY_OIL].distance) {
					resources[INVENTORY_OIL].distance = distance;
					resources[INVENTORY_OIL].buildingId = i;
				}
			}
			if (!Data_CityInfo.resourceStockpiled[RESOURCE_POTTERY] &&
				Resource_getAmountStoredInWarehouse(i, RESOURCE_POTTERY) > 0) {
				resources[INVENTORY_POTTERY].numBuildings++;
				if (distance < resources[INVENTORY_POTTERY].distance) {
					resources[INVENTORY_POTTERY].distance = distance;
					resources[INVENTORY_POTTERY].buildingId = i;
				}
			}
			if (!Data_CityInfo.resourceStockpiled[RESOURCE_FURNITURE] &&
				Resource_getAmountStoredInWarehouse(i, RESOURCE_FURNITURE) > 0) {
				resources[INVENTORY_FURNITURE].numBuildings++;
				if (distance < resources[INVENTORY_FURNITURE].distance) {
					resources[INVENTORY_FURNITURE].distance = distance;
					resources[INVENTORY_FURNITURE].buildingId = i;
				}
			}
		}
	}

	// update demands
	if (market->data.market.potteryDemand) {
		market->data.market.potteryDemand--;
	} else {
		resources[INVENTORY_POTTERY].numBuildings = 0;
	}
	if (market->data.market.furnitureDemand) {
		market->data.market.furnitureDemand--;
	} else {
		resources[INVENTORY_FURNITURE].numBuildings = 0;
	}
	if (market->data.market.oilDemand) {
		market->data.market.oilDemand--;
	} else {
		resources[INVENTORY_OIL].numBuildings = 0;
	}
	if (market->data.market.wineDemand) {
		market->data.market.wineDemand--;
	} else {
		resources[INVENTORY_WINE].numBuildings = 0;
	}

	int canGo = 0;
	for (int i = 0; i < INVENTORY_MAX; i++) {
		if (resources[i].numBuildings) {
			canGo = 1;
			break;
		}
	}
	if (!canGo) {
		return 0;
	}
	// prefer food if we don't have it
	if (!market->data.market.inventory[INVENTORY_WHEAT] && resources[INVENTORY_WHEAT].numBuildings) {
		market->data.market.fetchInventoryId = INVENTORY_WHEAT;
		return resources[INVENTORY_WHEAT].buildingId;
	} else if (!market->data.market.inventory[INVENTORY_VEGETABLES] && resources[INVENTORY_VEGETABLES].numBuildings) {
		market->data.market.fetchInventoryId = INVENTORY_VEGETABLES;
		return resources[INVENTORY_VEGETABLES].buildingId;
	} else if (!market->data.market.inventory[INVENTORY_FRUIT] && resources[INVENTORY_FRUIT].numBuildings) {
		market->data.market.fetchInventoryId = INVENTORY_FRUIT;
		return resources[INVENTORY_FRUIT].buildingId;
	} else if (!market->data.market.inventory[INVENTORY_MEAT] && resources[INVENTORY_MEAT].numBuildings) {
		market->data.market.fetchInventoryId = INVENTORY_MEAT;
		return resources[INVENTORY_MEAT].buildingId;
	}
	// then prefer resource if we don't have it
	if (!market->data.market.inventory[INVENTORY_POTTERY] && resources[INVENTORY_POTTERY].numBuildings) {
		market->data.market.fetchInventoryId = INVENTORY_POTTERY;
		return resources[INVENTORY_POTTERY].buildingId;
	} else if (!market->data.market.inventory[INVENTORY_FURNITURE] && resources[INVENTORY_FURNITURE].numBuildings) {
		market->data.market.fetchInventoryId = INVENTORY_FURNITURE;
		return resources[INVENTORY_FURNITURE].buildingId;
	} else if (!market->data.market.inventory[INVENTORY_OIL] && resources[INVENTORY_OIL].numBuildings) {
		market->data.market.fetchInventoryId = INVENTORY_OIL;
		return resources[INVENTORY_OIL].buildingId;
	} else if (!market->data.market.inventory[INVENTORY_WINE] && resources[INVENTORY_WINE].numBuildings) {
		market->data.market.fetchInventoryId = INVENTORY_WINE;
		return resources[INVENTORY_WINE].buildingId;
	}
	// then prefer smallest stock below 50
	int minStock = 50;
	int fetchInventoryId = -1;
	if (resources[INVENTORY_WHEAT].numBuildings &&
		market->data.market.inventory[INVENTORY_WHEAT] < minStock) {
		minStock = market->data.market.inventory[INVENTORY_WHEAT];
		fetchInventoryId = INVENTORY_WHEAT;
	}
	if (resources[INVENTORY_VEGETABLES].numBuildings &&
		market->data.market.inventory[INVENTORY_VEGETABLES] < minStock) {
		minStock = market->data.market.inventory[INVENTORY_VEGETABLES];
		fetchInventoryId = INVENTORY_VEGETABLES;
	}
	if (resources[INVENTORY_FRUIT].numBuildings &&
		market->data.market.inventory[INVENTORY_FRUIT] < minStock) {
		minStock = market->data.market.inventory[INVENTORY_FRUIT];
		fetchInventoryId = INVENTORY_FRUIT;
	}
	if (resources[INVENTORY_MEAT].numBuildings &&
		market->data.market.inventory[INVENTORY_MEAT] < minStock) {
		minStock = market->data.market.inventory[INVENTORY_MEAT];
		fetchInventoryId = INVENTORY_MEAT;
	}
	if (resources[INVENTORY_POTTERY].numBuildings &&
		market->data.market.inventory[INVENTORY_POTTERY] < minStock) {
		minStock = market->data.market.inventory[INVENTORY_POTTERY];
		fetchInventoryId = INVENTORY_POTTERY;
	}
	if (resources[INVENTORY_FURNITURE].numBuildings &&
		market->data.market.inventory[INVENTORY_FURNITURE] < minStock) {
		minStock = market->data.market.inventory[INVENTORY_FURNITURE];
		fetchInventoryId = INVENTORY_FURNITURE;
	}
	if (resources[INVENTORY_OIL].numBuildings &&
		market->data.market.inventory[INVENTORY_OIL] < minStock) {
		minStock = market->data.market.inventory[INVENTORY_OIL];
		fetchInventoryId = INVENTORY_OIL;
	}
	if (resources[INVENTORY_WINE].numBuildings &&
		market->data.market.inventory[INVENTORY_WINE] < minStock) {
		minStock = market->data.market.inventory[INVENTORY_WINE];
		fetchInventoryId = INVENTORY_WINE;
	}

	if (fetchInventoryId == -1) {
		// all items well stocked: pick food below threshold
		if (resources[INVENTORY_WHEAT].numBuildings &&
			market->data.market.inventory[INVENTORY_WHEAT] < 600) {
			fetchInventoryId = INVENTORY_WHEAT;
		}
		if (resources[INVENTORY_VEGETABLES].numBuildings &&
			market->data.market.inventory[INVENTORY_VEGETABLES] < 400) {
			fetchInventoryId = INVENTORY_VEGETABLES;
		}
		if (resources[INVENTORY_FRUIT].numBuildings &&
			market->data.market.inventory[INVENTORY_FRUIT] < 400) {
			fetchInventoryId = INVENTORY_FRUIT;
		}
		if (resources[INVENTORY_MEAT].numBuildings &&
			market->data.market.inventory[INVENTORY_MEAT] < 400) {
			fetchInventoryId = INVENTORY_MEAT;
		}
	}
	if (fetchInventoryId < 0 || fetchInventoryId >= INVENTORY_MAX) {
		return 0;
	}
	market->data.market.fetchInventoryId = fetchInventoryId;
	return resources[fetchInventoryId].buildingId;
}

int Building_Market_getMaxFoodStock(int buildingId)
{
    struct Data_Building *b = building_get(buildingId);
	int maxStock = 0;
	if (buildingId > 0 && b->type == BUILDING_MARKET) {
		for (int i = INVENTORY_MIN_FOOD; i < INVENTORY_MAX_FOOD; i++) {
			int stock = b->data.market.inventory[i];
			if (stock > maxStock) {
				maxStock = stock;
			}
		}
	}
	return maxStock;
}

int Building_Market_getMaxGoodsStock(int buildingId)
{
    struct Data_Building *b = building_get(buildingId);
	int maxStock = 0;
	if (buildingId > 0 && b->type == BUILDING_MARKET) {
		for (int i = INVENTORY_MIN_GOOD; i < INVENTORY_MAX_GOOD; i++) {
			int stock = b->data.market.inventory[i];
			if (stock > maxStock) {
				maxStock = stock;
			}
		}
	}
	return maxStock;
}

int Building_Dock_getNumIdleDockers(int buildingId)
{
	struct Data_Building *b = building_get(buildingId);
	int numIdle = 0;
	for (int i = 0; i < 3; i++) {
		if (b->data.other.dockFigureIds[i]) {
			figure *f = figure_get(b->data.other.dockFigureIds[i]);
			if (f->actionState == FigureActionState_132_DockerIdling ||
				f->actionState == FigureActionState_133_DockerImportQueue) {
				numIdle++;
			}
		}
	}
	return numIdle;
}

void Building_Dock_updateOpenWaterAccess()
{
    map_point river_entry = scenario_map_river_entry();
	map_routing_calculate_distances_water_boat(river_entry.x, river_entry.y);
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = building_get(i);
		if (BuildingIsInUse(i) && !b->houseSize && b->type == BUILDING_DOCK) {
			if (Terrain_isAdjacentToOpenWater(b->x, b->y, 3)) {
				b->hasWaterAccess = 1;
			} else {
				b->hasWaterAccess = 0;
			}
		}
	}
}

int Building_Dock_isConnectedToOpenWater(int x, int y)
{
    map_point river_entry = scenario_map_river_entry();
	map_routing_calculate_distances_water_boat(river_entry.x, river_entry.y);
	if (Terrain_isAdjacentToOpenWater(x, y, 3)) {
		return 1;
	} else {
		return 0;
	}
}

void Building_Mercury_removeResources(int bigCurse)
{
	int maxStored = 0;
	int maxBuildingId = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = building_get(i);
		if (!BuildingIsInUse(i)) {
			continue;
		}
		int totalStored = 0;
		if (b->type == BUILDING_WAREHOUSE) {
			for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
				totalStored += Resource_getAmountStoredInWarehouse(i, r);
			}
		} else if (b->type == BUILDING_GRANARY) {
			for (int r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
				totalStored += Resource_getAmountStoredInGranary(i, r);
			}
			totalStored /= 100;
		} else {
			continue;
		}
		if (totalStored > maxStored) {
			maxStored = totalStored;
			maxBuildingId = i;
		}
	}
	if (!maxBuildingId) {
		return;
	}
	struct Data_Building *b = building_get(maxBuildingId);
	if (bigCurse == 1) {
		city_message_disable_sound_for_next_message();
		city_message_post(0, MESSAGE_FIRE, b->type, b->gridOffset);
		Building_collapseOnFire(maxBuildingId, 0);
		Building_collapseLinked(maxBuildingId, 1);
		sound_effect_play(SOUND_EFFECT_EXPLOSION);
		map_routing_update_land();
	} else {
		if (b->type == BUILDING_WAREHOUSE) {
			Resource_removeFromWarehouseForMercury(maxBuildingId, 16);
		} else if (b->type == BUILDING_GRANARY) {
			int amount = Resource_removeFromGranary(maxBuildingId, RESOURCE_WHEAT, 1600);
			amount = Resource_removeFromGranary(maxBuildingId, RESOURCE_VEGETABLES, amount);
			amount = Resource_removeFromGranary(maxBuildingId, RESOURCE_FRUIT, amount);
			Resource_removeFromGranary(maxBuildingId, RESOURCE_MEAT, amount);
		}
	}
}

void Building_Mercury_fillGranary()
{
	int minStored = 10000;
	int minBuildingId = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = building_get(i);
		if (!BuildingIsInUse(i) || b->type != BUILDING_GRANARY) {
			continue;
		}
		int totalStored = 0;
		for (int r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
			totalStored += Resource_getAmountStoredInGranary(i, r);
		}
		if (totalStored < minStored) {
			minStored = totalStored;
			minBuildingId = i;
		}
	}
	if (minBuildingId) {
		for (int n = 0; n < 6; n++) {
			Resource_addToGranary(minBuildingId, RESOURCE_WHEAT, 0);
		}
		for (int n = 0; n < 6; n++) {
			Resource_addToGranary(minBuildingId, RESOURCE_VEGETABLES, 0);
		}
		for (int n = 0; n < 6; n++) {
			Resource_addToGranary(minBuildingId, RESOURCE_FRUIT, 0);
		}
		for (int n = 0; n < 6; n++) {
			Resource_addToGranary(minBuildingId, RESOURCE_MEAT, 0);
		}
	}
}
