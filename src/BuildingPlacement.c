#include "BuildingPlacement.h"

#include "Building.h"
#include "CityInfo.h"
#include "Figure.h"
#include "Formation.h"
#include "HousePopulation.h"
#include "Resource.h"
#include "SidebarMenu.h"
#include "Terrain.h"
#include "TerrainGraphics.h"
#include "Undo.h"

#include "UI/PopupDialog.h"
#include "UI/Window.h"

#include "Data/CityInfo.h"
#include "Data/State.h"

#include "building/building.h"
#include "building/count.h"
#include "building/model.h"
#include "building/placement_warning.h"
#include "building/properties.h"
#include "building/storage.h"
#include "city/finance.h"
#include "city/warning.h"
#include "core/calc.h"
#include "core/direction.h"
#include "core/random.h"
#include "figure/formation.h"
#include "graphics/image.h"
#include "map/aqueduct.h"
#include "map/bridge.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/property.h"
#include "map/routing.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"

#define BOUND_REGION() \
	if (xStart < xEnd) {\
		xMin = xStart;\
		xMax = xEnd;\
	} else {\
		xMin = xEnd;\
		xMax = xStart;\
	}\
	if (yStart < yEnd) {\
		yMin = yStart;\
		yMax = yEnd;\
	} else {\
		yMin = yEnd;\
		yMax = yStart;\
	}\
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);

struct ReservoirInfo {
	int cost;
	int placeReservoirAtStart;
	int placeReservoirAtEnd;
};

enum {
	PlaceReservoir_Blocked = -1,
	PlaceReservoir_No = 0,
	PlaceReservoir_Yes = 1,
	PlaceReservoir_Exists = 2
};

static struct {
	int xStart;
	int yStart;
	int xEnd;
	int yEnd;
	int bridgeConfirmed;
	int fortConfirmed;
} confirm;

static int itemsPlaced;

static void addToTerrainFort(int type, int buildingId, int x, int y, int size)
{
    building *b = building_get(buildingId);
	b->prevPartBuildingId = 0;
	Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_FORT), TERRAIN_BUILDING);
	int formationId = Formation_createLegion(b);
	b->formationId = formationId;
	if (type == BUILDING_FORT_LEGIONARIES) {
		b->subtype.fortFigureType = FIGURE_FORT_LEGIONARY;
        formation_set_figure_type(formationId, FIGURE_FORT_LEGIONARY);
	}
	if (type == BUILDING_FORT_JAVELIN) {
		b->subtype.fortFigureType = FIGURE_FORT_JAVELIN;
        formation_set_figure_type(formationId, FIGURE_FORT_JAVELIN);
	}
	if (type == BUILDING_FORT_MOUNTED) {
		b->subtype.fortFigureType = FIGURE_FORT_MOUNTED;
        formation_set_figure_type(formationId, FIGURE_FORT_MOUNTED);
	}
	// create parade ground
	building *ground = building_create(BUILDING_FORT_GROUND, x + 3, y - 1);
	Undo_addBuildingToList(ground->id);
	ground->formationId = formationId;
	ground->prevPartBuildingId = buildingId;
	b->nextPartBuildingId = ground->id;
	ground->nextPartBuildingId = 0;
	Terrain_addBuildingToGrids(ground->id, x + 3, y - 1, 4,
		image_group(GROUP_BUILDING_FORT) + 1, TERRAIN_BUILDING);
}

static void addToTerrainHippodrome(int type, int buildingId, int x, int y, int size)
{
	int graphicId1 = image_group(GROUP_BUILDING_HIPPODROME_1);
	int graphicId2 = image_group(GROUP_BUILDING_HIPPODROME_2);
	Data_CityInfo.buildingHippodromePlaced = 1;

	building *part1 = building_get(buildingId);
	if (Data_State.map.orientation == DIR_0_TOP || Data_State.map.orientation == DIR_4_BOTTOM) {
		part1->subtype.orientation = 0;
	} else {
		part1->subtype.orientation = 3;
	}
	part1->prevPartBuildingId = 0;
	int graphicId;
	switch (Data_State.map.orientation) {
		case DIR_0_TOP:    graphicId = graphicId2; break;
		case DIR_2_RIGHT:  graphicId = graphicId1 + 4; break;
		case DIR_4_BOTTOM: graphicId = graphicId2 + 4; break;
		case DIR_6_LEFT:   graphicId = graphicId1; break;
		default: return;
	}
	Terrain_addBuildingToGrids(buildingId, x, y, size, graphicId, TERRAIN_BUILDING);

	building *part2 = building_create(BUILDING_HIPPODROME, x + 5, y);
	Undo_addBuildingToList(part2->id);
	if (Data_State.map.orientation == DIR_0_TOP || Data_State.map.orientation == DIR_4_BOTTOM) {
		part2->subtype.orientation = 1;
	} else {
		part2->subtype.orientation = 4;
	}
	part2->prevPartBuildingId = buildingId;
	part1->nextPartBuildingId = part2->id;
	part2->nextPartBuildingId = 0;
	switch (Data_State.map.orientation) {
		case DIR_0_TOP: case DIR_4_BOTTOM: graphicId = graphicId2 + 2; break;
		case DIR_2_RIGHT: case DIR_6_LEFT: graphicId = graphicId1 + 2; break;
	}
	Terrain_addBuildingToGrids(part2->id, x + 5, y, size, graphicId, TERRAIN_BUILDING);

	building *part3 = building_create(BUILDING_HIPPODROME, x + 10, y);
	Undo_addBuildingToList(part3->id);
	if (Data_State.map.orientation == DIR_0_TOP || Data_State.map.orientation == DIR_4_BOTTOM) {
		part3->subtype.orientation = 2;
	} else {
		part3->subtype.orientation = 5;
	}
	part3->prevPartBuildingId = part2->id;
	part2->nextPartBuildingId = part3->id;
	part3->nextPartBuildingId = 0;
	switch (Data_State.map.orientation) {
		case DIR_0_TOP: graphicId = graphicId2 + 4; break;
		case DIR_2_RIGHT: graphicId = graphicId1; break;
		case DIR_4_BOTTOM: graphicId = graphicId2; break;
		case DIR_6_LEFT: graphicId = graphicId1 + 4; break;
	}
	Terrain_addBuildingToGrids(part3->id, x + 10, y, size, graphicId, TERRAIN_BUILDING);
}

static int addToTerrainWarehouseSpace(int x, int y, int prevId)
{
	building *b = building_create(BUILDING_WAREHOUSE_SPACE, x, y);
	Undo_addBuildingToList(b->id);
	b->prevPartBuildingId = prevId;
	building_get(prevId)->nextPartBuildingId = b->id;
	Terrain_addBuildingToGrids(b->id, x, y, 1,
		image_group(GROUP_BUILDING_WAREHOUSE_STORAGE_EMPTY), TERRAIN_BUILDING);
	return b->id;
}

static void addToTerrainWarehouse(int type, int buildingId, int x, int y)
{
    building *b = building_get(buildingId);
	b->storage_id = building_storage_create();
	b->prevPartBuildingId = 0;
	Terrain_addBuildingToGrids(buildingId, x, y, 1, image_group(GROUP_BUILDING_WAREHOUSE), TERRAIN_BUILDING);

	int prev = buildingId;
	prev = addToTerrainWarehouseSpace(x + 1, y, prev);
	prev = addToTerrainWarehouseSpace(x + 2, y, prev);
	prev = addToTerrainWarehouseSpace(x, y + 1, prev);
	prev = addToTerrainWarehouseSpace(x + 1, y + 1, prev);
	prev = addToTerrainWarehouseSpace(x + 2, y + 1, prev);
	prev = addToTerrainWarehouseSpace(x, y + 2, prev);
	prev = addToTerrainWarehouseSpace(x + 1, y + 2, prev);
	prev = addToTerrainWarehouseSpace(x + 2, y + 2, prev);
	building_get(prev)->nextPartBuildingId = 0;
}

static void addToTerrain(int type, int buildingId, int x, int y, int size,
	int orientation, int watersideOrientationAbs, int watersideOrientationRel)
{
	switch (type) {
		// houses
		case BUILDING_HOUSE_LARGE_TENT:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_TENT) + 2, TERRAIN_BUILDING);
			break;
		case BUILDING_HOUSE_SMALL_SHACK:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_SHACK), TERRAIN_BUILDING);
			break;
		case BUILDING_HOUSE_LARGE_SHACK:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_SHACK) + 2, TERRAIN_BUILDING);
			break;
		case BUILDING_HOUSE_SMALL_HOVEL:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_HOVEL), TERRAIN_BUILDING);
			break;
		case BUILDING_HOUSE_LARGE_HOVEL:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_HOVEL) + 2, TERRAIN_BUILDING);
			break;
		case BUILDING_HOUSE_SMALL_CASA:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_CASA), TERRAIN_BUILDING);
			break;
		case BUILDING_HOUSE_LARGE_CASA:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_CASA) + 2, TERRAIN_BUILDING);
			break;
		case BUILDING_HOUSE_SMALL_INSULA:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_INSULA_1), TERRAIN_BUILDING);
			break;
		case BUILDING_HOUSE_MEDIUM_INSULA:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_INSULA_1) + 2, TERRAIN_BUILDING);
			break;
		case BUILDING_HOUSE_LARGE_INSULA:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_INSULA_2), TERRAIN_BUILDING);
			break;
		case BUILDING_HOUSE_GRAND_INSULA:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_INSULA_2) + 2, TERRAIN_BUILDING);
			break;
		case BUILDING_HOUSE_SMALL_VILLA:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_VILLA_1), TERRAIN_BUILDING);
			break;
		case BUILDING_HOUSE_MEDIUM_VILLA:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_VILLA_1) + 2, TERRAIN_BUILDING);
			break;
		case BUILDING_HOUSE_LARGE_VILLA:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_VILLA_2), TERRAIN_BUILDING);
			break;
		case BUILDING_HOUSE_GRAND_VILLA:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_VILLA_2) + 1, TERRAIN_BUILDING);
			break;
		case BUILDING_HOUSE_SMALL_PALACE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_PALACE_1), TERRAIN_BUILDING);
			break;
		case BUILDING_HOUSE_MEDIUM_PALACE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_PALACE_1) + 1, TERRAIN_BUILDING);
			break;
		case BUILDING_HOUSE_LARGE_PALACE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_PALACE_2), TERRAIN_BUILDING);
			break;
		case BUILDING_HOUSE_LUXURY_PALACE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_PALACE_2) + 1, TERRAIN_BUILDING);
			break;
		// entertainment
		case BUILDING_AMPHITHEATER:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_AMPHITHEATER), TERRAIN_BUILDING);
			break;
		case BUILDING_THEATER:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_THEATER), TERRAIN_BUILDING);
			break;
		case BUILDING_COLOSSEUM:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_COLOSSEUM), TERRAIN_BUILDING);
			break;
		case BUILDING_GLADIATOR_SCHOOL:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_GLADIATOR_SCHOOL), TERRAIN_BUILDING);
			break;
		case BUILDING_LION_HOUSE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_LION_HOUSE), TERRAIN_BUILDING);
			break;
		case BUILDING_ACTOR_COLONY:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_ACTOR_COLONY), TERRAIN_BUILDING);
			break;
		case BUILDING_CHARIOT_MAKER:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_CHARIOT_MAKER), TERRAIN_BUILDING);
			break;
		// statues
		case BUILDING_SMALL_STATUE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_STATUE), TERRAIN_BUILDING);
			break;
		case BUILDING_MEDIUM_STATUE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_STATUE) + 1, TERRAIN_BUILDING);
			break;
		case BUILDING_LARGE_STATUE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_STATUE) + 2, TERRAIN_BUILDING);
			break;
		// health
		case BUILDING_DOCTOR:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_DOCTOR), TERRAIN_BUILDING);
			break;
		case BUILDING_HOSPITAL:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOSPITAL), TERRAIN_BUILDING);
			break;
		case BUILDING_BATHHOUSE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_BATHHOUSE_NO_WATER), TERRAIN_BUILDING);
			break;
		case BUILDING_BARBER:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_BARBER), TERRAIN_BUILDING);
			break;
		// education
		case BUILDING_SCHOOL:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_SCHOOL), TERRAIN_BUILDING);
			break;
		case BUILDING_ACADEMY:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_ACADEMY), TERRAIN_BUILDING);
			break;
		case BUILDING_LIBRARY:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_LIBRARY), TERRAIN_BUILDING);
			break;
		// security
		case BUILDING_PREFECTURE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_PREFECTURE), TERRAIN_BUILDING);
			break;
		// farms
		case BUILDING_WHEAT_FARM:
			TerrainGraphics_setBuildingFarm(buildingId, x, y, image_group(GROUP_BUILDING_FARM_CROPS), 0);
			break;
		case BUILDING_VEGETABLE_FARM:
			TerrainGraphics_setBuildingFarm(buildingId, x, y, image_group(GROUP_BUILDING_FARM_CROPS) + 5, 0);
			break;
		case BUILDING_FRUIT_FARM:
			TerrainGraphics_setBuildingFarm(buildingId, x, y, image_group(GROUP_BUILDING_FARM_CROPS) + 10, 0);
			break;
		case BUILDING_OLIVE_FARM:
			TerrainGraphics_setBuildingFarm(buildingId, x, y, image_group(GROUP_BUILDING_FARM_CROPS) + 15, 0);
			break;
		case BUILDING_VINES_FARM:
			TerrainGraphics_setBuildingFarm(buildingId, x, y, image_group(GROUP_BUILDING_FARM_CROPS) + 20, 0);
			break;
		case BUILDING_PIG_FARM:
			TerrainGraphics_setBuildingFarm(buildingId, x, y, image_group(GROUP_BUILDING_FARM_CROPS) + 25, 0);
			break;
		// industry
		case BUILDING_MARBLE_QUARRY:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_MARBLE_QUARRY), TERRAIN_BUILDING);
			break;
		case BUILDING_IRON_MINE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_IRON_MINE), TERRAIN_BUILDING);
			break;
		case BUILDING_TIMBER_YARD:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_TIMBER_YARD), TERRAIN_BUILDING);
			break;
		case BUILDING_CLAY_PIT:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_CLAY_PIT), TERRAIN_BUILDING);
			break;
		// workshops
		case BUILDING_WINE_WORKSHOP:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_WINE_WORKSHOP), TERRAIN_BUILDING);
			break;
		case BUILDING_OIL_WORKSHOP:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_OIL_WORKSHOP), TERRAIN_BUILDING);
			break;
		case BUILDING_WEAPONS_WORKSHOP:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_WEAPONS_WORKSHOP), TERRAIN_BUILDING);
			break;
		case BUILDING_FURNITURE_WORKSHOP:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_FURNITURE_WORKSHOP), TERRAIN_BUILDING);
			break;
		case BUILDING_POTTERY_WORKSHOP:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_POTTERY_WORKSHOP), TERRAIN_BUILDING);
			break;
		// distribution
		case BUILDING_GRANARY:
			building_get(buildingId)->storage_id = building_storage_create();
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_GRANARY), TERRAIN_BUILDING);
			break;
		case BUILDING_MARKET:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_MARKET), TERRAIN_BUILDING);
			break;
		// government
		case BUILDING_GOVERNORS_HOUSE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_GOVERNORS_HOUSE), TERRAIN_BUILDING);
			break;
		case BUILDING_GOVERNORS_VILLA:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_GOVERNORS_VILLA), TERRAIN_BUILDING);
			break;
		case BUILDING_GOVERNORS_PALACE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_GOVERNORS_PALACE), TERRAIN_BUILDING);
			break;
		case BUILDING_MISSION_POST:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_MISSION_POST), TERRAIN_BUILDING);
			break;
		case BUILDING_ENGINEERS_POST:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_ENGINEERS_POST), TERRAIN_BUILDING);
			break;
		case BUILDING_FORUM:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_FORUM), TERRAIN_BUILDING);
			break;
		// water
		case BUILDING_FOUNTAIN:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_FOUNTAIN_1), TERRAIN_BUILDING);
			break;
		case BUILDING_WELL:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_WELL), TERRAIN_BUILDING);
			break;
		// military
		case BUILDING_MILITARY_ACADEMY:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_MILITARY_ACADEMY), TERRAIN_BUILDING);
			break;
		// religion
		case BUILDING_SMALL_TEMPLE_CERES:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_TEMPLE_CERES), TERRAIN_BUILDING);
			break;
		case BUILDING_SMALL_TEMPLE_NEPTUNE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_TEMPLE_NEPTUNE), TERRAIN_BUILDING);
			break;
		case BUILDING_SMALL_TEMPLE_MERCURY:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_TEMPLE_MERCURY), TERRAIN_BUILDING);
			break;
		case BUILDING_SMALL_TEMPLE_MARS:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_TEMPLE_MARS), TERRAIN_BUILDING);
			break;
		case BUILDING_SMALL_TEMPLE_VENUS:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_TEMPLE_VENUS), TERRAIN_BUILDING);
			break;
		case BUILDING_LARGE_TEMPLE_CERES:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_TEMPLE_CERES) + 1, TERRAIN_BUILDING);
			break;
		case BUILDING_LARGE_TEMPLE_NEPTUNE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_TEMPLE_NEPTUNE) + 1, TERRAIN_BUILDING);
			break;
		case BUILDING_LARGE_TEMPLE_MERCURY:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_TEMPLE_MERCURY) + 1, TERRAIN_BUILDING);
			break;
		case BUILDING_LARGE_TEMPLE_MARS:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_TEMPLE_MARS) + 1, TERRAIN_BUILDING);
			break;
		case BUILDING_LARGE_TEMPLE_VENUS:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_TEMPLE_VENUS) + 1, TERRAIN_BUILDING);
			break;
		case BUILDING_ORACLE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_ORACLE), TERRAIN_BUILDING);
			break;
		// ships
		case BUILDING_SHIPYARD:
			building_get(buildingId)->data.other.dockOrientation = watersideOrientationAbs;
			Terrain_addWatersideBuildingToGrids(buildingId, x, y, 2,
				image_group(GROUP_BUILDING_SHIPYARD) + watersideOrientationRel);
			break;
		case BUILDING_WHARF:
			building_get(buildingId)->data.other.dockOrientation = watersideOrientationAbs;
			Terrain_addWatersideBuildingToGrids(buildingId, x, y, 2,
				image_group(GROUP_BUILDING_WHARF) + watersideOrientationRel);
			break;
		case BUILDING_DOCK:
			Data_CityInfo.numWorkingDocks++;
			building_get(buildingId)->data.other.dockOrientation = watersideOrientationAbs;
			{
				int graphicId;
				switch (watersideOrientationRel) {
					case 0: graphicId = image_group(GROUP_BUILDING_DOCK_1); break;
					case 1: graphicId = image_group(GROUP_BUILDING_DOCK_2); break;
					case 2: graphicId = image_group(GROUP_BUILDING_DOCK_3); break;
					default:graphicId = image_group(GROUP_BUILDING_DOCK_4); break;
				}
				Terrain_addWatersideBuildingToGrids(buildingId, x, y, size, graphicId);
			}
			break;
		// defense
		case BUILDING_TOWER:
			Terrain_clearWithRadius(x, y, 2, 0, TERRAIN_WALL);
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_TOWER),
				TERRAIN_BUILDING | TERRAIN_GATEHOUSE);
			TerrainGraphics_updateAreaWalls(x, y, 5);
			break;
		case BUILDING_GATEHOUSE:
			Terrain_addBuildingToGrids(buildingId, x, y, size,
				image_group(GROUP_BUILDING_TOWER) + orientation, TERRAIN_BUILDING | TERRAIN_GATEHOUSE);
			building_get(buildingId)->subtype.orientation = orientation;
			Building_determineGraphicIdsForOrientedBuildings();
			Terrain_addRoadsForGatehouse(x, y, orientation);
			TerrainGraphics_updateAreaRoads(x, y, 5);
			TerrainGraphics_updateRegionPlazas(0, 0, Data_State.map.width - 1, Data_State.map.height - 1);
			TerrainGraphics_updateAreaWalls(x, y, 5);
			break;
		case BUILDING_TRIUMPHAL_ARCH:
			Terrain_addBuildingToGrids(buildingId, x, y, size,
				image_group(GROUP_BUILDING_TRIUMPHAL_ARCH) + orientation - 1, TERRAIN_BUILDING);
			building_get(buildingId)->subtype.orientation = orientation;
			Building_determineGraphicIdsForOrientedBuildings();
			Terrain_addRoadsForTriumphalArch(x, y, orientation);
			TerrainGraphics_updateAreaRoads(x, y, 5);
			TerrainGraphics_updateRegionPlazas(0, 0, Data_State.map.width - 1, Data_State.map.height - 1);
			Data_CityInfo.triumphalArchesPlaced++;
			SidebarMenu_enableBuildingMenuItems();
			Data_State.selectedBuilding.type = 0;
			break;
		case BUILDING_SENATE_UPGRADED:
			Data_CityInfo.buildingSenatePlaced = 1;
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_SENATE), TERRAIN_BUILDING);
			if (!Data_CityInfo.buildingSenateGridOffset) {
				Data_CityInfo.buildingSenateBuildingId = buildingId;
				Data_CityInfo.buildingSenateX = x;
				Data_CityInfo.buildingSenateY = y;
				Data_CityInfo.buildingSenateGridOffset = map_grid_offset(x, y);
			}
			break;
		case BUILDING_BARRACKS:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_BARRACKS), TERRAIN_BUILDING);
			if (!Data_CityInfo.buildingBarracksGridOffset) {
				Data_CityInfo.buildingBarracksBuildingId = buildingId;
				Data_CityInfo.buildingBarracksX = x;
				Data_CityInfo.buildingBarracksY = y;
				Data_CityInfo.buildingBarracksGridOffset = map_grid_offset(x, y);
			}
			break;
		case BUILDING_WAREHOUSE:
			addToTerrainWarehouse(type, buildingId, x, y);
			break;
		case BUILDING_HIPPODROME:
			addToTerrainHippodrome(type, buildingId, x, y, size);
			break;
		case BUILDING_FORT_LEGIONARIES:
		case BUILDING_FORT_JAVELIN:
		case BUILDING_FORT_MOUNTED:
			addToTerrainFort(type, buildingId, x, y, size);
			break;
		// native buildings (unused, I think)
		case BUILDING_NATIVE_HUT:
			Terrain_addBuildingToGrids(buildingId, x, y, size,
				image_group(GROUP_BUILDING_NATIVE) + (random_byte() & 1), TERRAIN_BUILDING);
			break;
		case BUILDING_NATIVE_MEETING:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_NATIVE) + 2, TERRAIN_BUILDING);
			break;
		case BUILDING_NATIVE_CROPS:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_FARM_CROPS), TERRAIN_BUILDING);
			break;
		// distribution center (also unused)
		case BUILDING_DISTRIBUTION_CENTER_UNUSED:
			Data_CityInfo.buildingDistributionCenterPlaced = 1;
			if (!Data_CityInfo.buildingDistributionCenterGridOffset) {
				Data_CityInfo.buildingDistributionCenterBuildingId = buildingId;
				Data_CityInfo.buildingDistributionCenterX = x;
				Data_CityInfo.buildingDistributionCenterY = y;
				Data_CityInfo.buildingDistributionCenterGridOffset = map_grid_offset(x, y);
			}
			break;
	}
	map_routing_update_land();
	map_routing_update_walls();
}

static int placeBuilding(int type, int x, int y)
{
	unsigned short terrainMask = TERRAIN_ALL;
	if (type == BUILDING_GATEHOUSE || type == BUILDING_TRIUMPHAL_ARCH) {
		terrainMask = ~TERRAIN_ROAD;
	} else if (type == BUILDING_TOWER) {
		terrainMask = ~TERRAIN_WALL;
	}
	int size = building_properties_for_type(type)->size;
	if (type == BUILDING_WAREHOUSE) {
		size = 3;
	}
	int buildingOrientation = 0;
	if (type == BUILDING_GATEHOUSE) {
		buildingOrientation = Terrain_getOrientationGatehouse(x, y);
	} else if (type == BUILDING_TRIUMPHAL_ARCH) {
		buildingOrientation = Terrain_getOrientationTriumphalArch(x, y);
	}
	switch (Data_State.map.orientation) {
		case DIR_2_RIGHT: x = x - size + 1; break;
		case DIR_4_BOTTOM: x = x - size + 1; y = y - size + 1; break;
		case DIR_6_LEFT: y = y - size + 1; break;
	}
	// extra checks
	if (type == BUILDING_GATEHOUSE) {
		if (!Terrain_isClear(x, y, size, terrainMask, 0)) {
			city_warning_show(WARNING_CLEAR_LAND_NEEDED);
			return 0;
		}
		if (!buildingOrientation) {
			if (Data_State.selectedBuilding.roadRequired == 1) {
				buildingOrientation = 1;
			} else {
				buildingOrientation = 2;
			}
		}
	}
	if (type == BUILDING_TRIUMPHAL_ARCH) {
		if (!Terrain_isClear(x, y, size, terrainMask, 0)) {
			city_warning_show(WARNING_CLEAR_LAND_NEEDED);
			return 0;
		}
		if (!buildingOrientation) {
			if (Data_State.selectedBuilding.roadRequired == 1) {
				buildingOrientation = 1;
			} else {
				buildingOrientation = 3;
			}
		}
	}
	int watersideOrientationAbs, watersideOrientationRel;
	if (type == BUILDING_SHIPYARD || type == BUILDING_WHARF) {
		if (Terrain_determineOrientationWatersideSize2(
				x, y, 0, &watersideOrientationAbs, &watersideOrientationRel)) {
			city_warning_show(WARNING_SHORE_NEEDED);
			return 0;
		}
	} else if (type == BUILDING_DOCK) {
		if (Terrain_determineOrientationWatersideSize3(
				x, y, 0, &watersideOrientationAbs, &watersideOrientationRel)) {
			city_warning_show(WARNING_SHORE_NEEDED);
			return 0;
		}
		if (!Building_Dock_isConnectedToOpenWater(x, y)) {
			city_warning_show(WARNING_DOCK_OPEN_WATER_NEEDED);
			return 0;
		}
	} else {
		if (!Terrain_isClear(x, y, size, terrainMask, 0)) {
			city_warning_show(WARNING_CLEAR_LAND_NEEDED);
			return 0;
		}
		if (Data_State.selectedBuilding.meadowRequired) {
			if (!Terrain_existsTileWithinRadiusWithType(x, y, 3, 1, TERRAIN_MEADOW)) {
				city_warning_show(WARNING_MEADOW_NEEDED);
				return 0;
			}
		} else if (Data_State.selectedBuilding.rockRequired) {
			if (!Terrain_existsTileWithinRadiusWithType(x, y, 2, 1, TERRAIN_ROCK)) {
				city_warning_show(WARNING_ROCK_NEEDED);
				return 0;
			}
		} else if (Data_State.selectedBuilding.treesRequired) {
			if (!Terrain_existsTileWithinRadiusWithType(x, y, 2, 1, TERRAIN_SCRUB | TERRAIN_TREE)) {
				city_warning_show(WARNING_TREE_NEEDED);
				return 0;
			}
		} else if (Data_State.selectedBuilding.waterRequired) {
			if (!Terrain_existsTileWithinRadiusWithType(x, y, 2, 3, TERRAIN_WATER)) {
				city_warning_show(WARNING_WATER_NEEDED);
				return 0;
			}
		} else if (Data_State.selectedBuilding.wallRequired) {
			if (!Terrain_allTilesWithinRadiusHaveType(x, y, 2, 0, TERRAIN_WALL)) {
				city_warning_show(WARNING_WALL_NEEDED);
				return 0;
			}
		}
	}
	if (type == BUILDING_FORT_LEGIONARIES || type == BUILDING_FORT_JAVELIN || type == BUILDING_FORT_MOUNTED) {
		if (!Terrain_isClear(x + 3, y - 1, 4, terrainMask, 0)) {
			city_warning_show(WARNING_CLEAR_LAND_NEEDED);
			return 0;
		}
		if (formation_totals_get_num_legions() >= 6) {
			city_warning_show(WARNING_MAX_LEGIONS_REACHED);
			return 0;
		}
	}
	if (type == BUILDING_HIPPODROME) {
		if (Data_CityInfo.buildingHippodromePlaced) {
			city_warning_show(WARNING_ONE_BUILDING_OF_TYPE);
			return 0;
		}
		if (!Terrain_isClear(x + 5, y, 5, terrainMask, 0) ||
			!Terrain_isClear(x + 10, y, 5, terrainMask, 0)) {
			city_warning_show(WARNING_CLEAR_LAND_NEEDED);
			return 0;
		}
	}
	if (type == BUILDING_SENATE_UPGRADED && Data_CityInfo.buildingSenatePlaced) {
		city_warning_show(WARNING_ONE_BUILDING_OF_TYPE);
		return 0;
	}
	if (type == BUILDING_BARRACKS && building_count_total(BUILDING_BARRACKS) > 0) {
		city_warning_show(WARNING_ONE_BUILDING_OF_TYPE);
		return 0;
	}
	building_placement_warning_check_all(type, x, y, size);

	// phew, checks done!
	building *b;
	if (type == BUILDING_FORT_LEGIONARIES || type == BUILDING_FORT_JAVELIN || type == BUILDING_FORT_MOUNTED) {
		b = building_create(BUILDING_FORT, x, y);
	} else {
		b = building_create(type, x, y);
	}
	Undo_addBuildingToList(b->id);
	if (b->id <= 0) {
		return 0;
	}
	addToTerrain(type, b->id, x, y, size, buildingOrientation,
		watersideOrientationAbs, watersideOrientationRel);
	return 1;
}

static void placeHouses(int measureOnly, int xStart, int yStart, int xEnd, int yEnd)
{
	int xMin, xMax, yMin, yMax;
	BOUND_REGION();

	int needsRoadWarning = 0;
	itemsPlaced = 0;
	Undo_restoreBuildings();
	for (int y = yMin; y <= yMax; y++) {
		for (int x = xMin; x <= xMax; x++) {
			int gridOffset = map_grid_offset(x,y);
			if (map_terrain_is(gridOffset, TERRAIN_NOT_CLEAR)) {
				continue;
			}
			if (measureOnly) {
				map_property_mark_constructing(gridOffset);
				itemsPlaced++;
			} else {
				building *b = building_create(BUILDING_HOUSE_VACANT_LOT, x, y);
				Undo_addBuildingToList(b->id);
				if (b->id > 0) {
					itemsPlaced++;
					Terrain_addBuildingToGrids(b->id, x, y, 1,
						image_group(GROUP_BUILDING_HOUSE_VACANT_LOT), TERRAIN_BUILDING);
					if (!Terrain_existsTileWithinRadiusWithType(x, y, 1, 2, TERRAIN_ROAD)) {
						needsRoadWarning = 1;
					}
				}
			}
		}
	}
	if (!measureOnly) {
		building_placement_warning_check_food_stocks(BUILDING_HOUSE_VACANT_LOT);
		if (needsRoadWarning) {
			city_warning_show(WARNING_HOUSE_TOO_FAR_FROM_ROAD);
		}
		map_routing_update_land();
		UI_Window_requestRefresh();
	}
}

static void clearRegionConfirmed(int measureOnly, int xStart, int yStart, int xEnd, int yEnd)
{
	itemsPlaced = 0;
	Undo_restoreBuildings();
	map_terrain_restore();
	map_aqueduct_restore();
	Undo_restoreTerrainGraphics();

	int xMin, xMax, yMin, yMax;
	BOUND_REGION();

	for (int y = yMin; y <= yMax; y++) {
		for (int x = xMin; x <= xMax; x++) {
			int gridOffset = map_grid_offset(x,y);
			if (map_terrain_is(gridOffset, TERRAIN_ROCK | TERRAIN_ELEVATION)) {
				continue;
			}
			if (map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
				int buildingId = map_building_at(gridOffset);
				if (!buildingId) {
					continue;
				}
				building *b = building_get(buildingId);
				if (b->type == BUILDING_BURNING_RUIN || b->type == BUILDING_NATIVE_CROPS ||
					b->type == BUILDING_NATIVE_HUT || b->type == BUILDING_NATIVE_MEETING) {
					continue;
				}
				if (b->state == BuildingState_DeletedByPlayer) {
					continue;
				}
				if (b->type == BUILDING_FORT_GROUND || b->type == BUILDING_FORT) {
					if (!measureOnly && confirm.fortConfirmed != 1) {
						continue;
					}
					if (!measureOnly && confirm.fortConfirmed == 1) {
						Data_State.undoAvailable = 0;
					}
				}
				if (b->houseSize && b->housePopulation && !measureOnly) {
					HousePopulation_createHomeless(b->x, b->y, b->housePopulation);
					b->housePopulation = 0;
				}
				if (b->state != BuildingState_DeletedByPlayer) {
					itemsPlaced++;
					Undo_addBuildingToList(buildingId);
				}
				b->state = BuildingState_DeletedByPlayer;
				b->isDeleted = 1;
				building *space = building_get(buildingId);
				for (int i = 0; i < 9; i++) {
					if (space->prevPartBuildingId <= 0) {
						break;
					}
					int spaceId = space->prevPartBuildingId;
					space = building_get(spaceId);
					Undo_addBuildingToList(spaceId);
					space->state = BuildingState_DeletedByPlayer;
				}
				space = building_get(buildingId);
				for (int i = 0; i < 9; i++) {
					space = building_next(space);
					if (space->id <= 0) {
						break;
					}
					Undo_addBuildingToList(space->id);
					space->state = BuildingState_DeletedByPlayer;
				}
			} else if (map_terrain_is(gridOffset, TERRAIN_AQUEDUCT)) {
				map_terrain_remove(gridOffset, TERRAIN_CLEARABLE);
				itemsPlaced++;
				map_aqueduct_remove(gridOffset);
			} else if (map_terrain_is(gridOffset, TERRAIN_WATER)) {
				if (!measureOnly && map_bridge_count_figures(gridOffset) > 0) {
					city_warning_show(WARNING_PEOPLE_ON_BRIDGE);
				} else if (confirm.bridgeConfirmed == 1) {
					map_bridge_remove(gridOffset, measureOnly);
					itemsPlaced++;
				}
			} else if (map_terrain_is(gridOffset, TERRAIN_NOT_CLEAR)) {
				map_terrain_remove(gridOffset, TERRAIN_CLEARABLE);
				itemsPlaced++;
			}
		}
	}
	int radius;
	if (xMax - xMin <= yMax - yMin) {
		radius = yMax - yMin + 3;
	} else {
		radius = xMax - xMin + 3;
	}
	TerrainGraphics_updateRegionEmptyLand(xMin, yMin, xMax, yMax);
	TerrainGraphics_updateRegionMeadow(xMin, yMin, xMax, yMax);
	TerrainGraphics_updateRegionRubble(xMin, yMin, xMax, yMax);
	TerrainGraphics_updateAllGardens();
	TerrainGraphics_updateAreaRoads(xMin, yMin, radius);
	TerrainGraphics_updateRegionPlazas(0, 0, Data_State.map.width - 1, Data_State.map.height - 1);
	TerrainGraphics_updateAreaWalls(xMin, yMin, radius);
	if (!measureOnly) {
		map_routing_update_land();
		map_routing_update_walls();
		map_routing_update_water();
		UI_Window_requestRefresh();
	}
}

static void confirmDeleteFort(int accepted)
{
	if (accepted == 1) {
		confirm.fortConfirmed = 1;
	} else {
		confirm.fortConfirmed = -1;
	}
	clearRegionConfirmed(0, confirm.xStart, confirm.yStart, confirm.xEnd, confirm.yEnd);
}

static void confirmDeleteBridge(int accepted)
{
	if (accepted == 1) {
		confirm.bridgeConfirmed = 1;
	} else {
		confirm.bridgeConfirmed = -1;
	}
	clearRegionConfirmed(0, confirm.xStart, confirm.yStart, confirm.xEnd, confirm.yEnd);
}

static void clearRegion(int measureOnly, int xStart, int yStart, int xEnd, int yEnd)
{
	confirm.fortConfirmed = 0;
	confirm.bridgeConfirmed = 0;
	if (measureOnly) {
		return clearRegionConfirmed(measureOnly, xStart, yStart, xEnd, yEnd);
	}

	int xMin, xMax, yMin, yMax;
	BOUND_REGION();
	int askConfirmBridge = 0;
	int askConfirmFort = 0;
	for (int y = yMin; y <= yMax; y++) {
		for (int x = xMin; x <= xMax; x++) {
			int gridOffset = map_grid_offset(x,y);
			int buildingId = map_building_at(gridOffset);
			if (buildingId) {
                building *b = building_get(buildingId);
				if (b->type == BUILDING_FORT || b->type == BUILDING_FORT_GROUND) {
					askConfirmFort = 1;
				}
			}
			if (map_is_bridge(gridOffset)) {
				askConfirmBridge = 1;
			}
		}
	}
	confirm.xStart = xStart;
	confirm.yStart = yStart;
	confirm.xEnd = xEnd;
	confirm.yEnd = yEnd;
	if (askConfirmFort) {
		UI_PopupDialog_show(PopupDialog_DeleteFort, confirmDeleteFort, 2);
	} else if (askConfirmBridge) {
		UI_PopupDialog_show(PopupDialog_DeleteBridge, confirmDeleteBridge, 2);
	} else {
		clearRegionConfirmed(measureOnly, xStart, yStart, xEnd, yEnd);
	}
}

static int placeRoutedBuilding(int xSrc, int ySrc, int xDst, int yDst, routed_building_type type, int *items)
{
    static const int directionIndices[8][4] = {
        {0, 2, 6, 4},
        {0, 2, 6, 4},
        {2, 4, 0, 6},
        {2, 4, 0, 6},
        {4, 6, 2, 0},
        {4, 6, 2, 0},
        {6, 0, 4, 2},
        {6, 0, 4, 2}
    };
    *items = 0;
    int gridOffset = map_grid_offset(xDst, yDst);
    int guard = 0;
    // reverse routing
    while (1) {
        if (++guard >= 400) {
            return 0;
        }
        int distance = map_routing_distance(gridOffset);
        if (distance <= 0) {
            return 0;
        }
        switch (type) {
            default:
            case ROUTED_BUILDING_ROAD:
                *items += TerrainGraphics_setTileRoad(xDst, yDst);
                break;
            case ROUTED_BUILDING_WALL:
                *items += TerrainGraphics_setTileWall(xDst, yDst);
                break;
            case ROUTED_BUILDING_AQUEDUCT:
                *items += TerrainGraphics_setTileAqueductTerrain(xDst, yDst);
                break;
            case ROUTED_BUILDING_AQUEDUCT_WITHOUT_GRAPHIC:
                *items += 1;
                break;
        }
        int direction = calc_general_direction(xDst, yDst, xSrc, ySrc);
        if (direction == DIR_8_NONE) {
            return 1; // destination reached
        }
        int routed = 0;
        for (int i = 0; i < 4; i++) {
            int index = directionIndices[direction][i];
            int newGridOffset = gridOffset + map_grid_direction_delta(index);
            int newDist = map_routing_distance(newGridOffset);
            if (newDist > 0 && newDist < distance) {
                gridOffset = newGridOffset;
                xDst = map_grid_offset_to_x(gridOffset);
                yDst = map_grid_offset_to_y(gridOffset);
                routed = 1;
                break;
            }
        }
        if (!routed) {
            return 0;
        }
    }
}

static void placeRoad(int measureOnly, int xStart, int yStart, int xEnd, int yEnd)
{
	map_terrain_restore();
	map_aqueduct_restore();
	Undo_restoreTerrainGraphics();

	itemsPlaced = 0;
	int startOffset = map_grid_offset(xStart, yStart);
	int endOffset = map_grid_offset(xEnd, yEnd);
	int forbiddenTerrainMask =
        TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER |
        TERRAIN_SCRUB | TERRAIN_GARDEN | TERRAIN_ELEVATION |
        TERRAIN_RUBBLE | TERRAIN_BUILDING | TERRAIN_WALL;
	if (map_terrain_is(startOffset, forbiddenTerrainMask)) {
		return;
	}
	if (map_terrain_is(endOffset, forbiddenTerrainMask)) {
		return;
	}
	
	if (map_routing_calculate_distances_for_building(ROUTED_BUILDING_ROAD, xStart, yStart) &&
		    placeRoutedBuilding(xStart, yStart, xEnd, yEnd, ROUTED_BUILDING_ROAD, &itemsPlaced)) {
		if (!measureOnly) {
			map_routing_update_land();
			UI_Window_requestRefresh();
		}
	}
}

static void placeWall(int measureOnly, int xStart, int yStart, int xEnd, int yEnd)
{
	map_terrain_restore();
	map_aqueduct_restore();
	Undo_restoreTerrainGraphics();

	itemsPlaced = 0;
	int startOffset = map_grid_offset(xStart, yStart);
	int endOffset = map_grid_offset(xEnd, yEnd);
	int forbiddenTerrainMask =
        TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_SCRUB |
        TERRAIN_ROAD | TERRAIN_GARDEN | TERRAIN_ELEVATION |
        TERRAIN_RUBBLE | TERRAIN_AQUEDUCT | TERRAIN_ACCESS_RAMP;
	if (map_terrain_is(startOffset, forbiddenTerrainMask)) {
		return;
	}
	if (map_terrain_is(endOffset, forbiddenTerrainMask)) {
		return;
	}
	map_routing_calculate_distances_for_building(ROUTED_BUILDING_WALL, xStart, yStart);
	if (placeRoutedBuilding(xStart, yStart, xEnd, yEnd, ROUTED_BUILDING_WALL, &itemsPlaced)) {
		if (!measureOnly) {
			map_routing_update_land();
			map_routing_update_walls();
			UI_Window_requestRefresh();
		}
	}
}

static void placePlaza(int measureOnly, int xStart, int yStart, int xEnd, int yEnd)
{
	int xMin, yMin, xMax, yMax;
	BOUND_REGION();
	map_terrain_restore();
	map_aqueduct_restore();
	map_property_restore();
	Undo_restoreTerrainGraphics();
	
	itemsPlaced = 0;
	for (int y = yMin; y <= yMax; y++) {
		for (int x = xMin; x <= xMax; x++) {
			int gridOffset = map_grid_offset(x, y);
			if (map_terrain_is(gridOffset, TERRAIN_ROAD) &&
				!map_terrain_is(gridOffset, TERRAIN_WATER | TERRAIN_BUILDING | TERRAIN_AQUEDUCT)) {
				if (!map_property_is_plaza_or_earthquake(gridOffset)) {
					itemsPlaced++;
				}
				map_image_set(gridOffset, 0);
				map_property_mark_plaza_or_earthquake(gridOffset);
				map_property_set_multi_tile_size(gridOffset, 1);
				map_property_mark_draw_tile(gridOffset);
			}
		}
	}
	TerrainGraphics_updateRegionPlazas(0, 0,
		Data_State.map.width - 1, Data_State.map.height - 1);
}

static void placeGarden(int xStart, int yStart, int xEnd, int yEnd)
{
	int xMin, yMin, xMax, yMax;
	BOUND_REGION();
	
	map_terrain_restore();
	map_aqueduct_restore();
	map_property_restore();
	Undo_restoreTerrainGraphics();

	itemsPlaced = 0;
	for (int y = yMin; y <= yMax; y++) {
		for (int x = xMin; x <= xMax; x++) {
			int gridOffset = map_grid_offset(x,y);
			if (!map_terrain_is(gridOffset, TERRAIN_NOT_CLEAR)) {
				itemsPlaced++;
				map_terrain_add(gridOffset, TERRAIN_GARDEN);
			}
		}
	}
	TerrainGraphics_updateAllGardens();
}

static int placeAqueduct(int measureOnly, int xStart, int yStart, int xEnd, int yEnd, int *cost)
{
	map_terrain_restore();
	map_aqueduct_restore();
	Undo_restoreTerrainGraphics();
	int itemCost = model_get_building(BUILDING_AQUEDUCT)->cost;
	*cost = 0;
	int blocked = 0;
	int gridOffset = map_grid_offset(xStart, yStart);
	if (map_terrain_is(gridOffset, TERRAIN_ROAD)) {
		if (map_property_is_plaza_or_earthquake(gridOffset)) {
			blocked = 1;
		}
	} else if (map_terrain_is(gridOffset, TERRAIN_NOT_CLEAR)) {
		blocked = 1;
	}
	gridOffset = map_grid_offset(xEnd, yEnd);
	if (map_terrain_is(gridOffset, TERRAIN_ROAD)) {
		if (map_property_is_plaza_or_earthquake(gridOffset)) {
			blocked = 1;
		}
	} else if (map_terrain_is(gridOffset, TERRAIN_NOT_CLEAR)) {
		blocked = 1;
	}
	if (blocked) {
		return 0;
	}
	if (!map_routing_calculate_distances_for_building(ROUTED_BUILDING_AQUEDUCT, xStart, yStart)) {
		return 0;
	}
	int numItems;
	placeRoutedBuilding(xStart, yStart, xEnd, yEnd, ROUTED_BUILDING_AQUEDUCT, &numItems);
	*cost = itemCost * numItems;
	return 1;
}

static int placeReservoirAndAqueducts(int measureOnly, int xStart, int yStart, int xEnd, int yEnd, struct ReservoirInfo *info)
{
	info->cost = 0;
	info->placeReservoirAtStart = PlaceReservoir_No;
	info->placeReservoirAtEnd = PlaceReservoir_No;

	map_terrain_restore();
	map_aqueduct_restore();
	Undo_restoreTerrainGraphics();

	int distance = calc_maximum_distance(xStart, yStart, xEnd, yEnd);
	if (measureOnly && !Data_State.selectedBuilding.placementInProgress) {
		distance = 0;
	}
	if (distance > 0) {
		if (map_building_is_reservoir(xStart - 1, yStart - 1)) {
			info->placeReservoirAtStart = PlaceReservoir_Exists;
		} else if (Terrain_isClear(xStart - 1, yStart - 1, 3, TERRAIN_ALL, 0)) {
			info->placeReservoirAtStart = PlaceReservoir_Yes;
		} else {
			info->placeReservoirAtStart = PlaceReservoir_Blocked;
		}
	}
	if (map_building_is_reservoir(xEnd - 1, yEnd - 1)) {
		info->placeReservoirAtEnd = PlaceReservoir_Exists;
	} else if (Terrain_isClear(xEnd - 1, yEnd - 1, 3, TERRAIN_ALL, 0)) {
		info->placeReservoirAtEnd = PlaceReservoir_Yes;
	} else {
		info->placeReservoirAtEnd = PlaceReservoir_Blocked;
	}
	if (info->placeReservoirAtStart == PlaceReservoir_Blocked || info->placeReservoirAtEnd == PlaceReservoir_Blocked) {
		return 0;
	}
	if (info->placeReservoirAtStart == PlaceReservoir_Yes && info->placeReservoirAtEnd == PlaceReservoir_Yes && distance < 3) {
		return 0;
	}
	if (!distance) {
		info->cost = model_get_building(BUILDING_RESERVOIR)->cost;
		return 1;
	}
	if (!map_routing_calculate_distances_for_building(ROUTED_BUILDING_AQUEDUCT, xStart, yStart)) {
		return 0;
	}
	if (info->placeReservoirAtStart != PlaceReservoir_No) {
		map_routing_block(xStart - 1, yStart - 1, 3);
		Terrain_updateToPlaceBuildingToOverlay(3, xStart - 1, yStart - 1, TERRAIN_ALL, 1);
	}
	if (info->placeReservoirAtEnd != PlaceReservoir_No) {
		map_routing_block(xEnd - 1, yEnd - 1, 3);
		Terrain_updateToPlaceBuildingToOverlay(3, xEnd - 1, yEnd - 1, TERRAIN_ALL, 1);
	}
	const int aqueductOffsetsX[] = {0, 2, 0, -2};
	const int aqueductOffsetsY[] = {-2, 0, 2, 0};
	int minDist = 10000;
	int minDirStart, minDirEnd;
	for (int dirStart = 0; dirStart < 4; dirStart++) {
		int dxStart = aqueductOffsetsX[dirStart];
		int dyStart = aqueductOffsetsY[dirStart];
		for (int dirEnd = 0; dirEnd < 4; dirEnd++) {
			int dxEnd = aqueductOffsetsX[dirEnd];
			int dyEnd = aqueductOffsetsY[dirEnd];
			int dist;
			if (placeRoutedBuilding(
					xStart + dxStart, yStart + dyStart, xEnd + dxEnd, yEnd + dyEnd,
					ROUTED_BUILDING_AQUEDUCT_WITHOUT_GRAPHIC, &dist)) {
				if (dist && dist < minDist) {
					minDist = dist;
					minDirStart = dirStart;
					minDirEnd = dirEnd;
				}
			}
		}
	}
	if (minDist == 10000) {
		return 0;
	}
	int xAqStart = aqueductOffsetsX[minDirStart];
	int yAqStart = aqueductOffsetsY[minDirStart];
	int xAqEnd = aqueductOffsetsX[minDirEnd];
	int yAqEnd = aqueductOffsetsY[minDirEnd];
	int aqItems;
	placeRoutedBuilding(xStart + xAqStart, yStart + yAqStart,
		xEnd + xAqEnd, yEnd + yAqEnd, ROUTED_BUILDING_AQUEDUCT, &aqItems);
	if (info->placeReservoirAtStart == PlaceReservoir_Yes) {
		info->cost += model_get_building(BUILDING_RESERVOIR)->cost;
	}
	if (info->placeReservoirAtEnd == PlaceReservoir_Yes) {
		info->cost += model_get_building(BUILDING_RESERVOIR)->cost;
	}
	if (aqItems) {
		info->cost += aqItems * model_get_building(BUILDING_AQUEDUCT)->cost;
	}
	return 1;
}

void BuildingPlacement_update(int xStart, int yStart, int xEnd, int yEnd, int type)
{
	if (!type || city_finance_out_of_money()) {
		Data_State.selectedBuilding.cost = 0;
		return;
	}
	map_property_clear_constructing_and_deleted();
	int currentCost = model_get_building(type)->cost;

	if (type == BUILDING_CLEAR_LAND) {
		clearRegion(1, xStart, yStart, xEnd, yEnd);
		if (itemsPlaced >= 0) currentCost *= itemsPlaced;
	} else if (type == BUILDING_WALL) {
		placeWall(1, xStart, yStart, xEnd, yEnd);
		if (itemsPlaced >= 0) currentCost *= itemsPlaced;
	} else if (type == BUILDING_ROAD) {
		placeRoad(1, xStart, yStart, xEnd, yEnd);
		if (itemsPlaced >= 0) currentCost *= itemsPlaced;
	} else if (type == BUILDING_PLAZA) {
		placePlaza(1, xStart, yStart, xEnd, yEnd);
		if (itemsPlaced >= 0) currentCost *= itemsPlaced;
	} else if (type == BUILDING_GARDENS) {
		placeGarden(xStart, yStart, xEnd, yEnd);
		if (itemsPlaced >= 0) currentCost *= itemsPlaced;
	} else if (type == BUILDING_LOW_BRIDGE || type == BUILDING_SHIP_BRIDGE) {
		int length = map_bridge_building_length();
		if (length > 1) currentCost *= length;
	} else if (type == BUILDING_AQUEDUCT) {
		placeAqueduct(1, xStart, yStart, xEnd, yEnd, &currentCost);
		TerrainGraphics_updateRegionAqueduct(0, 0, Data_State.map.width - 1, Data_State.map.height - 1, 0);
	} else if (type == BUILDING_DRAGGABLE_RESERVOIR) {
		struct ReservoirInfo info;
		placeReservoirAndAqueducts(1, xStart, yStart, xEnd, yEnd, &info);
		currentCost = info.cost;
		TerrainGraphics_updateRegionAqueduct(0, 0, Data_State.map.width - 1, Data_State.map.height - 1, 1);
		Data_State.selectedBuilding.drawAsConstructing = 0;
	} else if (type == BUILDING_HOUSE_VACANT_LOT) {
		placeHouses(1, xStart, yStart, xEnd, yEnd);
		if (itemsPlaced >= 0) currentCost *= itemsPlaced;
	} else if (type == BUILDING_GATEHOUSE) {
		Terrain_updateToPlaceBuildingToOverlay(2, xEnd, yEnd, ~TERRAIN_ROAD, 0);
	} else if (type == BUILDING_TRIUMPHAL_ARCH) {
		Terrain_updateToPlaceBuildingToOverlay(3, xEnd, yEnd, ~TERRAIN_ROAD, 0);
	} else if (type == BUILDING_WAREHOUSE) {
		Terrain_updateToPlaceBuildingToOverlay(3, xEnd, yEnd, TERRAIN_ALL, 0);
	} else if (type == BUILDING_FORT_LEGIONARIES || type == BUILDING_FORT_JAVELIN || type == BUILDING_FORT_MOUNTED) {
		if (formation_totals_get_num_legions() < 6) {
			const int offsetsX[] = {3, 4, 4, 3};
			const int offsetsY[] = {-1, -1, 0, 0};
			int orientIndex = Data_State.map.orientation / 2;
			int xOffset = offsetsX[orientIndex];
			int yOffset = offsetsY[orientIndex];
			if (Terrain_isClearToBuild(3, xEnd, yEnd, TERRAIN_ALL) &&
				Terrain_isClearToBuild(4, xEnd + xOffset, yEnd + yOffset, TERRAIN_ALL)) {
				Terrain_updateToPlaceBuildingToOverlay(3, xEnd, yEnd, TERRAIN_ALL, 0);
			}
		}
	} else if (type == BUILDING_HIPPODROME) {
		if (Terrain_isClearToBuild(5, xEnd, yEnd, TERRAIN_ALL) &&
			Terrain_isClearToBuild(5, xEnd + 5, yEnd, TERRAIN_ALL) &&
			Terrain_isClearToBuild(5, xEnd + 10, yEnd, TERRAIN_ALL)) {
			Terrain_updateToPlaceBuildingToOverlay(5, xEnd, yEnd, TERRAIN_ALL, 0);
		}
	} else if (type == BUILDING_SHIPYARD || type == BUILDING_WHARF) {
		if (!Terrain_determineOrientationWatersideSize2(xEnd, yEnd, 1, 0, 0)) {
			Data_State.selectedBuilding.drawAsConstructing = 1;
		}
	} else if (type == BUILDING_DOCK) {
		if (!Terrain_determineOrientationWatersideSize3(xEnd, yEnd, 1, 0, 0)) {
			Data_State.selectedBuilding.drawAsConstructing = 1;
		}
	} else if (Data_State.selectedBuilding.meadowRequired) {
		Terrain_existsTileWithinRadiusWithType(xEnd, yEnd, 3, 1, TERRAIN_MEADOW);
	} else if (Data_State.selectedBuilding.rockRequired) {
		Terrain_existsTileWithinRadiusWithType(xEnd, yEnd, 2, 1, TERRAIN_ROCK);
	} else if (Data_State.selectedBuilding.treesRequired) {
		Terrain_existsTileWithinRadiusWithType(xEnd, yEnd, 2, 1, TERRAIN_TREE | TERRAIN_SCRUB);
	} else if (Data_State.selectedBuilding.waterRequired) {
		Terrain_existsTileWithinRadiusWithType(xEnd, yEnd, 2, 3, TERRAIN_WATER);
	} else if (Data_State.selectedBuilding.wallRequired) {
		Terrain_allTilesWithinRadiusHaveType(xEnd, yEnd, 2, 0, TERRAIN_WALL);
	} else {
		if (!(type == BUILDING_SENATE_UPGRADED && Data_CityInfo.buildingSenatePlaced) &&
			!(type == BUILDING_BARRACKS && building_count_total(BUILDING_BARRACKS) > 0) &&
			!(type == BUILDING_DISTRIBUTION_CENTER_UNUSED && Data_CityInfo.buildingDistributionCenterPlaced)) {
			int size = building_properties_for_type(type)->size;
			Terrain_updateToPlaceBuildingToOverlay(size, xEnd, yEnd, TERRAIN_ALL, 0);
		}
	}
	Data_State.selectedBuilding.cost = currentCost;
}

void BuildingPlacement_place(int orientation, int xStart, int yStart, int xEnd, int yEnd, int type)
{
	building_placement_warning_reset();
	if (!type) {
		return;
	}
	if (city_finance_out_of_money()) {
		map_property_clear_constructing_and_deleted();
		city_warning_show(WARNING_OUT_OF_MONEY);
		return;
	}
	if (type >= BUILDING_LARGE_TEMPLE_CERES && type <= BUILDING_LARGE_TEMPLE_VENUS && Data_CityInfo.resourceStored[RESOURCE_MARBLE] < 2) {
		map_property_clear_constructing_and_deleted();
		city_warning_show(WARNING_MARBLE_NEEDED_LARGE_TEMPLE);
		return;
	}
	if (type == BUILDING_ORACLE && Data_CityInfo.resourceStored[RESOURCE_MARBLE] < 2) {
		map_property_clear_constructing_and_deleted();
		city_warning_show(WARNING_MARBLE_NEEDED_ORACLE);
		return;
	}
	if (type != BUILDING_CLEAR_LAND && Figure_hasNearbyEnemy(xStart, yStart, xEnd, yEnd)) {
		if (type == BUILDING_WALL || type == BUILDING_ROAD || type == BUILDING_AQUEDUCT) {
			map_terrain_restore();
			map_aqueduct_restore();
			Undo_restoreTerrainGraphics();
		} else if (type == BUILDING_PLAZA || type == BUILDING_GARDENS) {
			map_terrain_restore();
			map_aqueduct_restore();
			map_property_restore();
			Undo_restoreTerrainGraphics();
		} else if (type == BUILDING_LOW_BRIDGE || type == BUILDING_SHIP_BRIDGE) {
			map_bridge_reset_building_length();
		} else {
			map_property_clear_constructing_and_deleted();
		}
		city_warning_show(WARNING_ENEMY_NEARBY);
		return;
	}

	int placementCost = model_get_building(type)->cost;
	if (type == BUILDING_CLEAR_LAND) {
		clearRegion(0, xStart, yStart, xEnd, yEnd);
		placementCost *= itemsPlaced;
	} else if (type == BUILDING_WALL) {
		placeWall(0, xStart, yStart, xEnd, yEnd);
		placementCost *= itemsPlaced;
	} else if (type == BUILDING_ROAD) {
		placeRoad(0, xStart, yStart, xEnd, yEnd);
		placementCost *= itemsPlaced;
	} else if (type == BUILDING_PLAZA) {
		placePlaza(0, xStart, yStart, xEnd, yEnd);
		placementCost *= itemsPlaced;
	} else if (type == BUILDING_GARDENS) {
		placeGarden(xStart, yStart, xEnd, yEnd);
		placementCost *= itemsPlaced;
		map_routing_update_land();
	} else if (type == BUILDING_LOW_BRIDGE) {
		int length = map_bridge_add(xEnd, yEnd, 0);
		if (length <= 1) {
			city_warning_show(WARNING_SHORE_NEEDED);
			return;
		}
		placementCost *= length;
	} else if (type == BUILDING_SHIP_BRIDGE) {
		int length = map_bridge_add(xEnd, yEnd, 1);
		if (length <= 1) {
			city_warning_show(WARNING_SHORE_NEEDED);
			return;
		}
		placementCost *= length;
	} else if (type == BUILDING_AQUEDUCT) {
		int cost;
		if (!placeAqueduct(0, xStart, yStart, xEnd, yEnd, &cost)) {
			city_warning_show(WARNING_CLEAR_LAND_NEEDED);
			return;
		}
		placementCost = cost;
		TerrainGraphics_updateRegionAqueduct(0, 0, Data_State.map.width - 1, Data_State.map.height - 1, 0);
		map_routing_update_land();
	} else if (type == BUILDING_DRAGGABLE_RESERVOIR) {
		struct ReservoirInfo info;
		if (!placeReservoirAndAqueducts(0, xStart, yStart, xEnd, yEnd, &info)) {
			city_warning_show(WARNING_CLEAR_LAND_NEEDED);
			return;
		}
		if (info.placeReservoirAtStart == PlaceReservoir_Yes) {
			building *reservoir = building_create(BUILDING_RESERVOIR, xStart - 1, yStart - 1);
			Undo_addBuildingToList(reservoir->id);
			Terrain_addBuildingToGrids(reservoir->id, xStart-1, yStart-1, 3, image_group(GROUP_BUILDING_RESERVOIR), TERRAIN_BUILDING);
			map_aqueduct_set(map_grid_offset(xStart-1, yStart-1), 0);
		}
		if (info.placeReservoirAtEnd == PlaceReservoir_Yes) {
			building *reservoir = building_create(BUILDING_RESERVOIR, xEnd - 1, yEnd - 1);
			Undo_addBuildingToList(reservoir->id);
			Terrain_addBuildingToGrids(reservoir->id, xEnd-1, yEnd-1, 3, image_group(GROUP_BUILDING_RESERVOIR), TERRAIN_BUILDING);
			map_aqueduct_set(map_grid_offset(xEnd-1, yEnd-1), 0);
			if (!Terrain_existsTileWithinAreaWithType(xStart - 2, yStart - 2, 5, TERRAIN_WATER) && info.placeReservoirAtStart == PlaceReservoir_No) {
				building_placement_warning_check_reservoir(BUILDING_RESERVOIR);
			}
		}
		placementCost = info.cost;
		TerrainGraphics_updateRegionAqueduct(0, 0, Data_State.map.width - 1, Data_State.map.height - 1, 0);
		map_routing_update_land();
	} else if (type == BUILDING_HOUSE_VACANT_LOT) {
		placeHouses(0, xStart, yStart, xEnd, yEnd);
		placementCost *= itemsPlaced;
	} else {
		if (!placeBuilding(type, xEnd, yEnd)) {
			return;
		}
	}
	if ((type >= BUILDING_LARGE_TEMPLE_CERES && type <= BUILDING_LARGE_TEMPLE_VENUS) || type == BUILDING_ORACLE) {
		Resource_removeFromCityWarehouses(RESOURCE_MARBLE, 2);
	}
	formation_move_herds_away(xEnd, yEnd);
	city_finance_process_construction(placementCost);
	if (type != BUILDING_TRIUMPHAL_ARCH) {
		Undo_recordBuild(placementCost);
	}
}

