#include "BuildingPlacement.h"

#include "Building.h"
#include "core/calc.h"
#include "CityInfo.h"
#include "Figure.h"
#include "Formation.h"
#include "Grid.h"
#include "HousePopulation.h"
#include "Resource.h"
#include "Routing.h"
#include "SidebarMenu.h"
#include "Terrain.h"
#include "TerrainBridge.h"
#include "TerrainGraphics.h"
#include "Undo.h"

#include "UI/PopupDialog.h"
#include "UI/Warning.h"
#include "UI/Window.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Grid.h"
#include "Data/State.h"
#include "Data/Figure.h"

#include "building/count.h"
#include "building/model.h"
#include "building/properties.h"
#include "building/storage.h"
#include "core/random.h"
#include "figure/formation.h"
#include "graphics/image.h"

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
	Bound2ToMap(xMin, yMin, xMax, yMax);

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
	Data_Buildings[buildingId].prevPartBuildingId = 0;
	Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_FORT), Terrain_Building);
	int formationId = Formation_createLegion(buildingId);
	Data_Buildings[buildingId].formationId = formationId;
	if (type == BUILDING_FORT_LEGIONARIES) {
		Data_Buildings[buildingId].subtype.fortFigureType = FIGURE_FORT_LEGIONARY;
        formation_set_figure_type(formationId, FIGURE_FORT_LEGIONARY);
	}
	if (type == BUILDING_FORT_JAVELIN) {
		Data_Buildings[buildingId].subtype.fortFigureType = FIGURE_FORT_JAVELIN;
        formation_set_figure_type(formationId, FIGURE_FORT_JAVELIN);
	}
	if (type == BUILDING_FORT_MOUNTED) {
		Data_Buildings[buildingId].subtype.fortFigureType = FIGURE_FORT_MOUNTED;
        formation_set_figure_type(formationId, FIGURE_FORT_MOUNTED);
	}
	// create parade ground
	int groundId = Building_create(BUILDING_FORT_GROUND, x + 3, y - 1);
	Undo_addBuildingToList(groundId);
	Data_Buildings[groundId].formationId = formationId;
	Data_Buildings[groundId].prevPartBuildingId = buildingId;
	Data_Buildings[buildingId].nextPartBuildingId = groundId;
	Data_Buildings[groundId].nextPartBuildingId = 0;
	Terrain_addBuildingToGrids(groundId, x + 3, y - 1, 4,
		image_group(GROUP_BUILDING_FORT) + 1, Terrain_Building);
}

static void addToTerrainHippodrome(int type, int buildingId, int x, int y, int size)
{
	int graphicId1 = image_group(GROUP_BUILDING_HIPPODROME_1);
	int graphicId2 = image_group(GROUP_BUILDING_HIPPODROME_2);
	Data_CityInfo.buildingHippodromePlaced = 1;

	struct Data_Building *part1 = &Data_Buildings[buildingId];
	if (Data_State.map.orientation == Dir_0_Top || Data_State.map.orientation == Dir_4_Bottom) {
		part1->subtype.orientation = 0;
	} else {
		part1->subtype.orientation = 3;
	}
	part1->prevPartBuildingId = 0;
	int graphicId;
	switch (Data_State.map.orientation) {
		case Dir_0_Top:    graphicId = graphicId2; break;
		case Dir_2_Right:  graphicId = graphicId1 + 4; break;
		case Dir_4_Bottom: graphicId = graphicId2 + 4; break;
		case Dir_6_Left:   graphicId = graphicId1; break;
		default: return;
	}
	Terrain_addBuildingToGrids(buildingId, x, y, size, graphicId, Terrain_Building);

	int part2Id = Building_create(BUILDING_HIPPODROME, x + 5, y);
	struct Data_Building *part2 = &Data_Buildings[part2Id];
	Undo_addBuildingToList(part2Id);
	if (Data_State.map.orientation == Dir_0_Top || Data_State.map.orientation == Dir_4_Bottom) {
		part2->subtype.orientation = 1;
	} else {
		part2->subtype.orientation = 4;
	}
	part2->prevPartBuildingId = buildingId;
	part1->nextPartBuildingId = part2Id;
	part2->nextPartBuildingId = 0;
	switch (Data_State.map.orientation) {
		case Dir_0_Top: case Dir_4_Bottom: graphicId = graphicId2 + 2; break;
		case Dir_2_Right: case Dir_6_Left: graphicId = graphicId1 + 2; break;
	}
	Terrain_addBuildingToGrids(part2Id, x + 5, y, size, graphicId, Terrain_Building);

	int part3Id = Building_create(BUILDING_HIPPODROME, x + 10, y);
	struct Data_Building *part3 = &Data_Buildings[part3Id];
	Undo_addBuildingToList(part3Id);
	if (Data_State.map.orientation == Dir_0_Top || Data_State.map.orientation == Dir_4_Bottom) {
		part3->subtype.orientation = 2;
	} else {
		part3->subtype.orientation = 5;
	}
	part3->prevPartBuildingId = part2Id;
	part2->nextPartBuildingId = part3Id;
	part3->nextPartBuildingId = 0;
	switch (Data_State.map.orientation) {
		case Dir_0_Top: graphicId = graphicId2 + 4; break;
		case Dir_2_Right: graphicId = graphicId1; break;
		case Dir_4_Bottom: graphicId = graphicId2; break;
		case Dir_6_Left: graphicId = graphicId1 + 4; break;
	}
	Terrain_addBuildingToGrids(part3Id, x + 10, y, size, graphicId, Terrain_Building);
}

static int addToTerrainWarehouseSpace(int x, int y, int prevId)
{
	int buildingId = Building_create(BUILDING_WAREHOUSE_SPACE, x, y);
	Undo_addBuildingToList(buildingId);
	Data_Buildings[buildingId].prevPartBuildingId = prevId;
	Data_Buildings[prevId].nextPartBuildingId = buildingId;
	Terrain_addBuildingToGrids(buildingId, x, y, 1,
		image_group(GROUP_BUILDING_WAREHOUSE_STORAGE_EMPTY), Terrain_Building);
	return buildingId;
}

static void addToTerrainWarehouse(int type, int buildingId, int x, int y)
{
	Data_Buildings[buildingId].storage_id = building_storage_create();
	Data_Buildings[buildingId].prevPartBuildingId = 0;
	Terrain_addBuildingToGrids(buildingId, x, y, 1, image_group(GROUP_BUILDING_WAREHOUSE), Terrain_Building);

	int prev = buildingId;
	prev = addToTerrainWarehouseSpace(x + 1, y, prev);
	prev = addToTerrainWarehouseSpace(x + 2, y, prev);
	prev = addToTerrainWarehouseSpace(x, y + 1, prev);
	prev = addToTerrainWarehouseSpace(x + 1, y + 1, prev);
	prev = addToTerrainWarehouseSpace(x + 2, y + 1, prev);
	prev = addToTerrainWarehouseSpace(x, y + 2, prev);
	prev = addToTerrainWarehouseSpace(x + 1, y + 2, prev);
	prev = addToTerrainWarehouseSpace(x + 2, y + 2, prev);
	Data_Buildings[prev].nextPartBuildingId = 0;
}

static void addToTerrain(int type, int buildingId, int x, int y, int size,
	int orientation, int watersideOrientationAbs, int watersideOrientationRel)
{
	switch (type) {
		// houses
		case BUILDING_HOUSE_LARGE_TENT:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_TENT) + 2, Terrain_Building);
			break;
		case BUILDING_HOUSE_SMALL_SHACK:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_SHACK), Terrain_Building);
			break;
		case BUILDING_HOUSE_LARGE_SHACK:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_SHACK) + 2, Terrain_Building);
			break;
		case BUILDING_HOUSE_SMALL_HOVEL:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_HOVEL), Terrain_Building);
			break;
		case BUILDING_HOUSE_LARGE_HOVEL:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_HOVEL) + 2, Terrain_Building);
			break;
		case BUILDING_HOUSE_SMALL_CASA:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_CASA), Terrain_Building);
			break;
		case BUILDING_HOUSE_LARGE_CASA:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_CASA) + 2, Terrain_Building);
			break;
		case BUILDING_HOUSE_SMALL_INSULA:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_INSULA_1), Terrain_Building);
			break;
		case BUILDING_HOUSE_MEDIUM_INSULA:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_INSULA_1) + 2, Terrain_Building);
			break;
		case BUILDING_HOUSE_LARGE_INSULA:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_INSULA_2), Terrain_Building);
			break;
		case BUILDING_HOUSE_GRAND_INSULA:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_INSULA_2) + 2, Terrain_Building);
			break;
		case BUILDING_HOUSE_SMALL_VILLA:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_VILLA_1), Terrain_Building);
			break;
		case BUILDING_HOUSE_MEDIUM_VILLA:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_VILLA_1) + 2, Terrain_Building);
			break;
		case BUILDING_HOUSE_LARGE_VILLA:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_VILLA_2), Terrain_Building);
			break;
		case BUILDING_HOUSE_GRAND_VILLA:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_VILLA_2) + 1, Terrain_Building);
			break;
		case BUILDING_HOUSE_SMALL_PALACE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_PALACE_1), Terrain_Building);
			break;
		case BUILDING_HOUSE_MEDIUM_PALACE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_PALACE_1) + 1, Terrain_Building);
			break;
		case BUILDING_HOUSE_LARGE_PALACE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_PALACE_2), Terrain_Building);
			break;
		case BUILDING_HOUSE_LUXURY_PALACE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOUSE_PALACE_2) + 1, Terrain_Building);
			break;
		// entertainment
		case BUILDING_AMPHITHEATER:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_AMPHITHEATER), Terrain_Building);
			break;
		case BUILDING_THEATER:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_THEATER), Terrain_Building);
			break;
		case BUILDING_COLOSSEUM:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_COLOSSEUM), Terrain_Building);
			break;
		case BUILDING_GLADIATOR_SCHOOL:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_GLADIATOR_SCHOOL), Terrain_Building);
			break;
		case BUILDING_LION_HOUSE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_LION_HOUSE), Terrain_Building);
			break;
		case BUILDING_ACTOR_COLONY:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_ACTOR_COLONY), Terrain_Building);
			break;
		case BUILDING_CHARIOT_MAKER:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_CHARIOT_MAKER), Terrain_Building);
			break;
		// statues
		case BUILDING_SMALL_STATUE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_STATUE), Terrain_Building);
			break;
		case BUILDING_MEDIUM_STATUE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_STATUE) + 1, Terrain_Building);
			break;
		case BUILDING_LARGE_STATUE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_STATUE) + 2, Terrain_Building);
			break;
		// health
		case BUILDING_DOCTOR:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_DOCTOR), Terrain_Building);
			break;
		case BUILDING_HOSPITAL:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_HOSPITAL), Terrain_Building);
			break;
		case BUILDING_BATHHOUSE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_BATHHOUSE_NO_WATER), Terrain_Building);
			break;
		case BUILDING_BARBER:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_BARBER), Terrain_Building);
			break;
		// education
		case BUILDING_SCHOOL:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_SCHOOL), Terrain_Building);
			break;
		case BUILDING_ACADEMY:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_ACADEMY), Terrain_Building);
			break;
		case BUILDING_LIBRARY:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_LIBRARY), Terrain_Building);
			break;
		// security
		case BUILDING_PREFECTURE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_PREFECTURE), Terrain_Building);
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
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_MARBLE_QUARRY), Terrain_Building);
			break;
		case BUILDING_IRON_MINE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_IRON_MINE), Terrain_Building);
			break;
		case BUILDING_TIMBER_YARD:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_TIMBER_YARD), Terrain_Building);
			break;
		case BUILDING_CLAY_PIT:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_CLAY_PIT), Terrain_Building);
			break;
		// workshops
		case BUILDING_WINE_WORKSHOP:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_WINE_WORKSHOP), Terrain_Building);
			break;
		case BUILDING_OIL_WORKSHOP:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_OIL_WORKSHOP), Terrain_Building);
			break;
		case BUILDING_WEAPONS_WORKSHOP:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_WEAPONS_WORKSHOP), Terrain_Building);
			break;
		case BUILDING_FURNITURE_WORKSHOP:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_FURNITURE_WORKSHOP), Terrain_Building);
			break;
		case BUILDING_POTTERY_WORKSHOP:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_POTTERY_WORKSHOP), Terrain_Building);
			break;
		// distribution
		case BUILDING_GRANARY:
			Data_Buildings[buildingId].storage_id = building_storage_create();
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_GRANARY), Terrain_Building);
			break;
		case BUILDING_MARKET:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_MARKET), Terrain_Building);
			break;
		// government
		case BUILDING_GOVERNORS_HOUSE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_GOVERNORS_HOUSE), Terrain_Building);
			break;
		case BUILDING_GOVERNORS_VILLA:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_GOVERNORS_VILLA), Terrain_Building);
			break;
		case BUILDING_GOVERNORS_PALACE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_GOVERNORS_PALACE), Terrain_Building);
			break;
		case BUILDING_MISSION_POST:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_MISSION_POST), Terrain_Building);
			break;
		case BUILDING_ENGINEERS_POST:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_ENGINEERS_POST), Terrain_Building);
			break;
		case BUILDING_FORUM:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_FORUM), Terrain_Building);
			break;
		// water
		case BUILDING_FOUNTAIN:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_FOUNTAIN_1), Terrain_Building);
			break;
		case BUILDING_WELL:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_WELL), Terrain_Building);
			break;
		// military
		case BUILDING_MILITARY_ACADEMY:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_MILITARY_ACADEMY), Terrain_Building);
			break;
		// religion
		case BUILDING_SMALL_TEMPLE_CERES:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_TEMPLE_CERES), Terrain_Building);
			break;
		case BUILDING_SMALL_TEMPLE_NEPTUNE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_TEMPLE_NEPTUNE), Terrain_Building);
			break;
		case BUILDING_SMALL_TEMPLE_MERCURY:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_TEMPLE_MERCURY), Terrain_Building);
			break;
		case BUILDING_SMALL_TEMPLE_MARS:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_TEMPLE_MARS), Terrain_Building);
			break;
		case BUILDING_SMALL_TEMPLE_VENUS:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_TEMPLE_VENUS), Terrain_Building);
			break;
		case BUILDING_LARGE_TEMPLE_CERES:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_TEMPLE_CERES) + 1, Terrain_Building);
			break;
		case BUILDING_LARGE_TEMPLE_NEPTUNE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_TEMPLE_NEPTUNE) + 1, Terrain_Building);
			break;
		case BUILDING_LARGE_TEMPLE_MERCURY:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_TEMPLE_MERCURY) + 1, Terrain_Building);
			break;
		case BUILDING_LARGE_TEMPLE_MARS:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_TEMPLE_MARS) + 1, Terrain_Building);
			break;
		case BUILDING_LARGE_TEMPLE_VENUS:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_TEMPLE_VENUS) + 1, Terrain_Building);
			break;
		case BUILDING_ORACLE:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_ORACLE), Terrain_Building);
			break;
		// ships
		case BUILDING_SHIPYARD:
			Data_Buildings[buildingId].data.other.dockOrientation = watersideOrientationAbs;
			Terrain_addWatersideBuildingToGrids(buildingId, x, y, 2,
				image_group(GROUP_BUILDING_SHIPYARD) + watersideOrientationRel);
			break;
		case BUILDING_WHARF:
			Data_Buildings[buildingId].data.other.dockOrientation = watersideOrientationAbs;
			Terrain_addWatersideBuildingToGrids(buildingId, x, y, 2,
				image_group(GROUP_BUILDING_WHARF) + watersideOrientationRel);
			break;
		case BUILDING_DOCK:
			Data_CityInfo.numWorkingDocks++;
			Data_Buildings[buildingId].data.other.dockOrientation = watersideOrientationAbs;
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
			Terrain_clearWithRadius(x, y, 2, 0, ~Terrain_Wall);
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_TOWER),
				Terrain_Building | Terrain_Gatehouse);
			TerrainGraphics_updateAreaWalls(x, y, 5);
			break;
		case BUILDING_GATEHOUSE:
			Terrain_addBuildingToGrids(buildingId, x, y, size,
				image_group(GROUP_BUILDING_TOWER) + orientation, Terrain_Building | Terrain_Gatehouse);
			Data_Buildings[buildingId].subtype.orientation = orientation;
			Building_determineGraphicIdsForOrientedBuildings();
			Terrain_addRoadsForGatehouse(x, y, orientation);
			TerrainGraphics_updateAreaRoads(x, y, 5);
			TerrainGraphics_updateRegionPlazas(0, 0, Data_State.map.width - 1, Data_State.map.height - 1);
			TerrainGraphics_updateAreaWalls(x, y, 5);
			break;
		case BUILDING_TRIUMPHAL_ARCH:
			Terrain_addBuildingToGrids(buildingId, x, y, size,
				image_group(GROUP_BUILDING_TRIUMPHAL_ARCH) + orientation - 1, Terrain_Building);
			Data_Buildings[buildingId].subtype.orientation = orientation;
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
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_SENATE), Terrain_Building);
			if (!Data_CityInfo.buildingSenateGridOffset) {
				Data_CityInfo.buildingSenateBuildingId = buildingId;
				Data_CityInfo.buildingSenateX = x;
				Data_CityInfo.buildingSenateY = y;
				Data_CityInfo.buildingSenateGridOffset = GridOffset(x, y);
			}
			break;
		case BUILDING_BARRACKS:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_BARRACKS), Terrain_Building);
			if (!Data_CityInfo.buildingBarracksGridOffset) {
				Data_CityInfo.buildingBarracksBuildingId = buildingId;
				Data_CityInfo.buildingBarracksX = x;
				Data_CityInfo.buildingBarracksY = y;
				Data_CityInfo.buildingBarracksGridOffset = GridOffset(x, y);
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
				image_group(GROUP_BUILDING_NATIVE) + (random_byte() & 1), Terrain_Building);
			break;
		case BUILDING_NATIVE_MEETING:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_NATIVE) + 2, Terrain_Building);
			break;
		case BUILDING_NATIVE_CROPS:
			Terrain_addBuildingToGrids(buildingId, x, y, size, image_group(GROUP_BUILDING_FARM_CROPS), Terrain_Building);
			break;
		// distribution center (also unused)
		case BUILDING_DISTRIBUTION_CENTER_UNUSED:
			Data_CityInfo.buildingDistributionCenterPlaced = 1;
			if (!Data_CityInfo.buildingDistributionCenterGridOffset) {
				Data_CityInfo.buildingDistributionCenterBuildingId = buildingId;
				Data_CityInfo.buildingDistributionCenterX = x;
				Data_CityInfo.buildingDistributionCenterY = y;
				Data_CityInfo.buildingDistributionCenterGridOffset = GridOffset(x, y);
			}
			break;
	}
	Routing_determineLandCitizen();
	Routing_determineLandNonCitizen();
	Routing_determineWalls();
}

static int placeBuilding(int type, int x, int y)
{
	unsigned short terrainMask = Terrain_All;
	if (type == BUILDING_GATEHOUSE || type == BUILDING_TRIUMPHAL_ARCH) {
		terrainMask = ~Terrain_Road;
	} else if (type == BUILDING_TOWER) {
		terrainMask = ~Terrain_Wall;
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
		case Dir_2_Right: x = x - size + 1; break;
		case Dir_4_Bottom: x = x - size + 1; y = y - size + 1; break;
		case Dir_6_Left: y = y - size + 1; break;
	}
	// extra checks
	if (type == BUILDING_GATEHOUSE) {
		if (!Terrain_isClear(x, y, size, terrainMask, 0)) {
			UI_Warning_show(Warning_ClearLandNeeded);
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
			UI_Warning_show(Warning_ClearLandNeeded);
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
			UI_Warning_show(Warning_ShoreNeeded);
			return 0;
		}
	} else if (type == BUILDING_DOCK) {
		if (Terrain_determineOrientationWatersideSize3(
				x, y, 0, &watersideOrientationAbs, &watersideOrientationRel)) {
			UI_Warning_show(Warning_ShoreNeeded);
			return 0;
		}
		if (!Building_Dock_isConnectedToOpenWater(x, y)) {
			UI_Warning_show(Warning_DockOpenWaterNeeded);
			return 0;
		}
	} else {
		if (!Terrain_isClear(x, y, size, terrainMask, 0)) {
			UI_Warning_show(Warning_ClearLandNeeded);
			return 0;
		}
		if (Data_State.selectedBuilding.meadowRequired) {
			if (!Terrain_existsTileWithinRadiusWithType(x, y, 3, 1, Terrain_Meadow)) {
				UI_Warning_show(Warning_MeadowNeeded);
				return 0;
			}
		} else if (Data_State.selectedBuilding.rockRequired) {
			if (!Terrain_existsTileWithinRadiusWithType(x, y, 2, 1, Terrain_Rock)) {
				UI_Warning_show(Warning_RockNeeded);
				return 0;
			}
		} else if (Data_State.selectedBuilding.treesRequired) {
			if (!Terrain_existsTileWithinRadiusWithType(x, y, 2, 1, Terrain_Scrub | Terrain_Tree)) {
				UI_Warning_show(Warning_TreeNeeded);
				return 0;
			}
		} else if (Data_State.selectedBuilding.waterRequired) {
			if (!Terrain_existsTileWithinRadiusWithType(x, y, 2, 3, Terrain_Water)) {
				UI_Warning_show(Warning_WaterNeeded);
				return 0;
			}
		} else if (Data_State.selectedBuilding.wallRequired) {
			if (!Terrain_allTilesWithinRadiusHaveType(x, y, 2, 0, Terrain_Wall)) {
				UI_Warning_show(Warning_WallNeeded);
				return 0;
			}
		}
	}
	if (type == BUILDING_FORT_LEGIONARIES || type == BUILDING_FORT_JAVELIN || type == BUILDING_FORT_MOUNTED) {
		if (!Terrain_isClear(x + 3, y - 1, 4, terrainMask, 0)) {
			UI_Warning_show(Warning_ClearLandNeeded);
			return 0;
		}
		if (formation_totals_get_num_legions() >= 6) {
			UI_Warning_show(Warning_MaxLegionsReached);
			return 0;
		}
	}
	if (type == BUILDING_HIPPODROME) {
		if (Data_CityInfo.buildingHippodromePlaced) {
			UI_Warning_show(Warning_OneBuildingOfType);
			return 0;
		}
		if (!Terrain_isClear(x + 5, y, 5, terrainMask, 0) ||
			!Terrain_isClear(x + 10, y, 5, terrainMask, 0)) {
			UI_Warning_show(Warning_ClearLandNeeded);
			return 0;
		}
	}
	if (type == BUILDING_SENATE_UPGRADED && Data_CityInfo.buildingSenatePlaced) {
		UI_Warning_show(Warning_OneBuildingOfType);
		return 0;
	}
	if (type == BUILDING_BARRACKS && building_count_total(BUILDING_BARRACKS) > 0) {
		UI_Warning_show(Warning_OneBuildingOfType);
		return 0;
	}
	UI_Warning_checkNewBuilding(type, x, y, size);

	// phew, checks done!
	int buildingId;
	if (type == BUILDING_FORT_LEGIONARIES || type == BUILDING_FORT_JAVELIN || type == BUILDING_FORT_MOUNTED) {
		buildingId = Building_create(BUILDING_FORT, x, y);
	} else {
		buildingId = Building_create(type, x, y);
	}
	Undo_addBuildingToList(buildingId);
	if (buildingId <= 0) {
		return 0;
	}
	addToTerrain(type, buildingId, x, y, size, buildingOrientation,
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
			int gridOffset = GridOffset(x,y);
			if (Data_Grid_terrain[gridOffset] & Terrain_NotClear) {
				continue;
			}
			if (measureOnly) {
				Data_Grid_bitfields[gridOffset] |= Bitfield_Overlay;
				itemsPlaced++;
			} else {
				int buildingId = Building_create(BUILDING_HOUSE_VACANT_LOT, x, y);
				Undo_addBuildingToList(buildingId);
				if (buildingId > 0) {
					itemsPlaced++;
					Terrain_addBuildingToGrids(buildingId, x, y, 1,
						image_group(GROUP_BUILDING_HOUSE_VACANT_LOT), Terrain_Building);
					if (!Terrain_existsTileWithinRadiusWithType(x, y, 1, 2, Terrain_Road)) {
						needsRoadWarning = 1;
					}
				}
			}
		}
	}
	if (!measureOnly) {
		UI_Warning_checkFoodStocks(BUILDING_HOUSE_VACANT_LOT);
		if (needsRoadWarning) {
			UI_Warning_show(Warning_HouseTooFarFromRoad);
		}
		Routing_determineLandCitizen();
		Routing_determineLandNonCitizen();
		UI_Window_requestRefresh();
	}
}

static void clearRegionConfirmed(int measureOnly, int xStart, int yStart, int xEnd, int yEnd)
{
	itemsPlaced = 0;
	Undo_restoreBuildings();
	Grid_copyShortGrid(Data_Grid_Undo_terrain, Data_Grid_terrain);
	Grid_copyByteGrid(Data_Grid_Undo_aqueducts, Data_Grid_aqueducts);
	Undo_restoreTerrainGraphics();

	int xMin, xMax, yMin, yMax;
	BOUND_REGION();

	for (int y = yMin; y <= yMax; y++) {
		for (int x = xMin; x <= xMax; x++) {
			int gridOffset = GridOffset(x,y);
			if (Data_Grid_terrain[gridOffset] & (Terrain_Rock | Terrain_Elevation)) {
				continue;
			}
			int terrain = Data_Grid_terrain[gridOffset] & Terrain_NotClear;
			if (terrain & Terrain_Building) {
				int buildingId = Data_Grid_buildingIds[gridOffset];
				if (!buildingId) {
					continue;
				}
				int type = Data_Buildings[buildingId].type;
				if (type == BUILDING_BURNING_RUIN || type == BUILDING_NATIVE_CROPS ||
					type == BUILDING_NATIVE_HUT || type == BUILDING_NATIVE_MEETING) {
					continue;
				}
				if (Data_Buildings[buildingId].state == BuildingState_DeletedByPlayer) {
					continue;
				}
				if (type == BUILDING_FORT_GROUND || type == BUILDING_FORT) {
					if (!measureOnly && confirm.fortConfirmed != 1) {
						continue;
					}
					if (!measureOnly && confirm.fortConfirmed == 1) {
						Data_State.undoAvailable = 0;
					}
				}
				struct Data_Building *b = &Data_Buildings[buildingId];
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
				int spaceId = buildingId;
				for (int i = 0; i < 9; i++) {
					spaceId = Data_Buildings[spaceId].prevPartBuildingId;
					if (spaceId <= 0) {
						break;
					}
					Undo_addBuildingToList(spaceId);
					Data_Buildings[spaceId].state = BuildingState_DeletedByPlayer;
				}
				spaceId = buildingId;
				for (int i = 0; i < 9; i++) {
					spaceId = Data_Buildings[spaceId].nextPartBuildingId;
					if (spaceId <= 0) {
						break;
					}
					Undo_addBuildingToList(spaceId);
					Data_Buildings[spaceId].state = BuildingState_DeletedByPlayer;
				}
			} else if (terrain & Terrain_Aqueduct) {
				Data_Grid_terrain[gridOffset] &= Terrain_2e80;
				Data_Grid_aqueducts[gridOffset] = 0;
				itemsPlaced++;
				if (Data_Grid_aqueducts[gridOffset - 162] == 5) {
					Data_Grid_aqueducts[gridOffset - 162] = 1;
				}
				if (Data_Grid_aqueducts[gridOffset + 1] == 6) {
					Data_Grid_aqueducts[gridOffset + 1] = 2;
				}
				if (Data_Grid_aqueducts[gridOffset + 162] == 5) {
					Data_Grid_aqueducts[gridOffset + 162] = 3;
				}
				if (Data_Grid_aqueducts[gridOffset - 1] == 6) {
					Data_Grid_aqueducts[gridOffset - 1] = 4;
				}
			} else if (terrain & Terrain_Water) {
				if (!measureOnly && TerrainBridge_countFiguresOnBridge(gridOffset) > 0) {
					UI_Warning_show(Warning_PeopleOnBridge);
				} else if (confirm.bridgeConfirmed == 1) {
					TerrainBridge_removeFromSpriteGrid(gridOffset, measureOnly);
					itemsPlaced++;
				}
			} else if (terrain) {
				Data_Grid_terrain[gridOffset] &= Terrain_2e80;
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
		Routing_determineLandCitizen();
		Routing_determineLandNonCitizen();
		Routing_determineWalls();
		Routing_determineWater();
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
			int gridOffset = GridOffset(x,y);
			int buildingId = Data_Grid_buildingIds[gridOffset];
			if (buildingId) {
				if (Data_Buildings[buildingId].type == BUILDING_FORT ||
					Data_Buildings[buildingId].type == BUILDING_FORT_GROUND) {
					askConfirmFort = 1;
				}
			}
			if (Data_Grid_spriteOffsets[gridOffset] && Data_Grid_terrain[gridOffset] & Terrain_Water) {
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

static void placeRoad(int measureOnly, int xStart, int yStart, int xEnd, int yEnd)
{
	Grid_copyShortGrid(Data_Grid_Undo_terrain, Data_Grid_terrain);
	Grid_copyByteGrid(Data_Grid_Undo_aqueducts, Data_Grid_aqueducts);
	Undo_restoreTerrainGraphics();

	itemsPlaced = 0;
	int startOffset = GridOffset(xStart, yStart);
	int endOffset = GridOffset(xEnd, yEnd);
	int forbiddenTerrainMask = Terrain_Building | Terrain_Wall | Terrain_1237;
	if (Data_Grid_terrain[startOffset] & forbiddenTerrainMask) {
		return;
	}
	if (Data_Grid_terrain[endOffset] & forbiddenTerrainMask) {
		return;
	}
	
	if (Routing_getDistanceForBuildingRoadOrAqueduct(xStart, yStart, 0) &&
		Routing_placeRoutedBuilding(xStart, yStart, xEnd, yEnd, RoutedBUILDING_ROAD, &itemsPlaced)) {
		if (!measureOnly) {
			Routing_determineLandCitizen();
			Routing_determineLandNonCitizen();
			UI_Window_requestRefresh();
		}
	}
}

static void placeWall(int measureOnly, int xStart, int yStart, int xEnd, int yEnd)
{
	Grid_copyShortGrid(Data_Grid_Undo_terrain, Data_Grid_terrain);
	Grid_copyByteGrid(Data_Grid_Undo_aqueducts, Data_Grid_aqueducts);
	Undo_restoreTerrainGraphics();

	itemsPlaced = 0;
	int startOffset = GridOffset(xStart, yStart);
	int endOffset = GridOffset(xEnd, yEnd);
	int forbiddenTerrainMask = Terrain_127f | Terrain_Aqueduct | Terrain_AccessRamp;
	if (Data_Grid_terrain[startOffset] & forbiddenTerrainMask) {
		return;
	}
	if (Data_Grid_terrain[endOffset] & forbiddenTerrainMask) {
		return;
	}
	
	if (Routing_getDistanceForBuildingWall(xStart, yStart) &&
		Routing_placeRoutedBuilding(xStart, yStart, xEnd, yEnd, RoutedBUILDING_WALL, &itemsPlaced)) {
		if (!measureOnly) {
			Routing_determineLandCitizen();
			Routing_determineLandNonCitizen();
			Routing_determineWalls();
			UI_Window_requestRefresh();
		}
	}
}

static void placePlaza(int measureOnly, int xStart, int yStart, int xEnd, int yEnd)
{
	int xMin, yMin, xMax, yMax;
	BOUND_REGION();
	Grid_copyShortGrid(Data_Grid_Undo_terrain, Data_Grid_terrain);
	Grid_copyByteGrid(Data_Grid_Undo_aqueducts, Data_Grid_aqueducts);
	Grid_copyByteGrid(Data_Grid_Undo_bitfields, Data_Grid_bitfields);
	Grid_copyByteGrid(Data_Grid_Undo_edge, Data_Grid_edge);
	Undo_restoreTerrainGraphics();
	
	itemsPlaced = 0;
	for (int y = yMin; y <= yMax; y++) {
		for (int x = xMin; x <= xMax; x++) {
			int gridOffset = GridOffset(x, y);
			int terrain = Data_Grid_terrain[gridOffset];
			if (terrain & Terrain_Road &&
				!(terrain & (Terrain_Water | Terrain_Building | Terrain_Aqueduct))) {
				if (!(Data_Grid_bitfields[gridOffset] & Bitfield_PlazaOrEarthquake)) {
					itemsPlaced++;
				}
				Data_Grid_graphicIds[gridOffset] = 0;
				Data_Grid_bitfields[gridOffset] |= Bitfield_PlazaOrEarthquake;
				Data_Grid_bitfields[gridOffset] &= Bitfield_NoSizes;
				Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
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
	
	Grid_copyShortGrid(Data_Grid_Undo_terrain, Data_Grid_terrain);
	Grid_copyByteGrid(Data_Grid_Undo_aqueducts, Data_Grid_aqueducts);
	Grid_copyByteGrid(Data_Grid_Undo_bitfields, Data_Grid_bitfields);
	Grid_copyByteGrid(Data_Grid_Undo_edge, Data_Grid_edge);
	Undo_restoreTerrainGraphics();

	itemsPlaced = 0;
	for (int y = yMin; y <= yMax; y++) {
		for (int x = xMin; x <= xMax; x++) {
			int gridOffset = GridOffset(x,y);
			if (!(Data_Grid_terrain[gridOffset] & Terrain_NotClear)) {
				itemsPlaced++;
				Data_Grid_terrain[gridOffset] |= Terrain_Garden;
			}
		}
	}
	TerrainGraphics_updateAllGardens();
}

static int placeAqueduct(int measureOnly, int xStart, int yStart, int xEnd, int yEnd, int *cost)
{
	Grid_copyShortGrid(Data_Grid_Undo_terrain, Data_Grid_terrain);
	Grid_copyByteGrid(Data_Grid_Undo_aqueducts, Data_Grid_aqueducts);
	Undo_restoreTerrainGraphics();
	int itemCost = model_get_building(BUILDING_AQUEDUCT)->cost;
	*cost = 0;
	int blocked = 0;
	int gridOffset = GridOffset(xStart, yStart);
	if (Data_Grid_terrain[gridOffset] & Terrain_Road) {
		if (Data_Grid_bitfields[gridOffset] & Bitfield_PlazaOrEarthquake) {
			blocked = 1;
		}
	} else if (Data_Grid_terrain[gridOffset] & Terrain_NotClear) {
		blocked = 1;
	}
	gridOffset = GridOffset(xEnd, yEnd);
	if (Data_Grid_terrain[gridOffset] & Terrain_Road) {
		if (Data_Grid_bitfields[gridOffset] & Bitfield_PlazaOrEarthquake) {
			blocked = 1;
		}
	} else if (Data_Grid_terrain[gridOffset] & Terrain_NotClear) {
		blocked = 1;
	}
	if (blocked) {
		return 0;
	}
	if (!Routing_getDistanceForBuildingRoadOrAqueduct(xStart, yStart, 1)) {
		return 0;
	}
	int numItems;
	Routing_placeRoutedBuilding(xStart, yStart, xEnd, yEnd, RoutedBuilding_Aqueduct, &numItems);
	*cost = itemCost * numItems;
	return 1;
}

static int placeReservoirAndAqueducts(int measureOnly, int xStart, int yStart, int xEnd, int yEnd, struct ReservoirInfo *info)
{
	info->cost = 0;
	info->placeReservoirAtStart = PlaceReservoir_No;
	info->placeReservoirAtEnd = PlaceReservoir_No;

	Grid_copyShortGrid(Data_Grid_Undo_terrain, Data_Grid_terrain);
	Grid_copyByteGrid(Data_Grid_Undo_aqueducts, Data_Grid_aqueducts);
	Undo_restoreTerrainGraphics();

	int distance = calc_maximum_distance(xStart, yStart, xEnd, yEnd);
	if (measureOnly && !Data_State.selectedBuilding.placementInProgress) {
		distance = 0;
	}
	if (distance > 0) {
		if (Terrain_isReservoir(GridOffset(xStart - 1, yStart - 1))) {
			info->placeReservoirAtStart = PlaceReservoir_Exists;
		} else if (Terrain_isClear(xStart - 1, yStart - 1, 3, Terrain_All, 0)) {
			info->placeReservoirAtStart = PlaceReservoir_Yes;
		} else {
			info->placeReservoirAtStart = PlaceReservoir_Blocked;
		}
	}
	if (Terrain_isReservoir(GridOffset(xEnd - 1, yEnd - 1))) {
		info->placeReservoirAtEnd = PlaceReservoir_Exists;
	} else if (Terrain_isClear(xEnd - 1, yEnd - 1, 3, Terrain_All, 0)) {
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
	if (!Routing_getDistanceForBuildingRoadOrAqueduct(xStart, yStart, 1)) {
		return 0;
	}
	if (info->placeReservoirAtStart != PlaceReservoir_No) {
		Routing_block(xStart - 1, yStart - 1, 3);
		Terrain_updateToPlaceBuildingToOverlay(3, xStart - 1, yStart - 1, Terrain_All, 1);
	}
	if (info->placeReservoirAtEnd != PlaceReservoir_No) {
		Routing_block(xEnd - 1, yEnd - 1, 3);
		Terrain_updateToPlaceBuildingToOverlay(3, xEnd - 1, yEnd - 1, Terrain_All, 1);
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
			if (Routing_placeRoutedBuilding(
					xStart + dxStart, yStart + dyStart, xEnd + dxEnd, yEnd + dyEnd,
					RoutedBuilding_AqueductWithoutGraphic, &dist)) {
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
	Routing_placeRoutedBuilding(xStart + xAqStart, yStart + yAqStart,
		xEnd + xAqEnd, yEnd + yAqEnd, RoutedBuilding_Aqueduct, &aqItems);
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
	if (!type || Data_CityInfo.treasury <= MIN_TREASURY) {
		Data_State.selectedBuilding.cost = 0;
		return;
	}
	Grid_andByteGrid(Data_Grid_bitfields, Bitfield_NoOverlayAndDeleted);
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
		int length = TerrainBridge_getLength();
		if (length > 1) currentCost *= length;
	} else if (type == BUILDING_AQUEDUCT) {
		placeAqueduct(1, xStart, yStart, xEnd, yEnd, &currentCost);
		TerrainGraphics_updateRegionAqueduct(0, 0, Data_State.map.width - 1, Data_State.map.height - 1, 0);
	} else if (type == BUILDING_DRAGGABLE_RESERVOIR) {
		struct ReservoirInfo info;
		placeReservoirAndAqueducts(1, xStart, yStart, xEnd, yEnd, &info);
		currentCost = info.cost;
		TerrainGraphics_updateRegionAqueduct(0, 0, Data_State.map.width - 1, Data_State.map.height - 1, 1);
		Data_State.selectedBuilding.drawAsOverlay = 0;
	} else if (type == BUILDING_HOUSE_VACANT_LOT) {
		placeHouses(1, xStart, yStart, xEnd, yEnd);
		if (itemsPlaced >= 0) currentCost *= itemsPlaced;
	} else if (type == BUILDING_GATEHOUSE) {
		Terrain_updateToPlaceBuildingToOverlay(2, xEnd, yEnd, ~Terrain_Road, 0);
	} else if (type == BUILDING_TRIUMPHAL_ARCH) {
		Terrain_updateToPlaceBuildingToOverlay(3, xEnd, yEnd, ~Terrain_Road, 0);
	} else if (type == BUILDING_WAREHOUSE) {
		Terrain_updateToPlaceBuildingToOverlay(3, xEnd, yEnd, Terrain_All, 0);
	} else if (type == BUILDING_FORT_LEGIONARIES || type == BUILDING_FORT_JAVELIN || type == BUILDING_FORT_MOUNTED) {
		if (formation_totals_get_num_legions() < 6) {
			const int offsetsX[] = {3, 4, 4, 3};
			const int offsetsY[] = {-1, -1, 0, 0};
			int orientIndex = Data_State.map.orientation / 2;
			int xOffset = offsetsX[orientIndex];
			int yOffset = offsetsY[orientIndex];
			if (Terrain_isClearToBuild(3, xEnd, yEnd, Terrain_All) &&
				Terrain_isClearToBuild(4, xEnd + xOffset, yEnd + yOffset, Terrain_All)) {
				Terrain_updateToPlaceBuildingToOverlay(3, xEnd, yEnd, Terrain_All, 0);
			}
		}
	} else if (type == BUILDING_HIPPODROME) {
		if (Terrain_isClearToBuild(5, xEnd, yEnd, Terrain_All) &&
			Terrain_isClearToBuild(5, xEnd + 5, yEnd, Terrain_All) &&
			Terrain_isClearToBuild(5, xEnd + 10, yEnd, Terrain_All)) {
			Terrain_updateToPlaceBuildingToOverlay(5, xEnd, yEnd, Terrain_All, 0);
		}
	} else if (type == BUILDING_SHIPYARD || type == BUILDING_WHARF) {
		if (!Terrain_determineOrientationWatersideSize2(xEnd, yEnd, 1, 0, 0)) {
			Data_State.selectedBuilding.drawAsOverlay = 1;
		}
	} else if (type == BUILDING_DOCK) {
		if (!Terrain_determineOrientationWatersideSize3(xEnd, yEnd, 1, 0, 0)) {
			Data_State.selectedBuilding.drawAsOverlay = 1;
		}
	} else if (Data_State.selectedBuilding.meadowRequired) {
		Terrain_existsTileWithinRadiusWithType(xEnd, yEnd, 3, 1, Terrain_Meadow);
	} else if (Data_State.selectedBuilding.rockRequired) {
		Terrain_existsTileWithinRadiusWithType(xEnd, yEnd, 2, 1, Terrain_Rock);
	} else if (Data_State.selectedBuilding.treesRequired) {
		Terrain_existsTileWithinRadiusWithType(xEnd, yEnd, 2, 1, Terrain_Tree | Terrain_Scrub);
	} else if (Data_State.selectedBuilding.waterRequired) {
		Terrain_existsTileWithinRadiusWithType(xEnd, yEnd, 2, 3, Terrain_Water);
	} else if (Data_State.selectedBuilding.wallRequired) {
		Terrain_allTilesWithinRadiusHaveType(xEnd, yEnd, 2, 0, Terrain_Wall);
	} else {
		if (!(type == BUILDING_SENATE_UPGRADED && Data_CityInfo.buildingSenatePlaced) &&
			!(type == BUILDING_BARRACKS && building_count_total(BUILDING_BARRACKS) > 0) &&
			!(type == BUILDING_DISTRIBUTION_CENTER_UNUSED && Data_CityInfo.buildingDistributionCenterPlaced)) {
			int size = building_properties_for_type(type)->size;
			Terrain_updateToPlaceBuildingToOverlay(size, xEnd, yEnd, Terrain_All, 0);
		}
	}
	Data_State.selectedBuilding.cost = currentCost;
}

void BuildingPlacement_place(int orientation, int xStart, int yStart, int xEnd, int yEnd, int type)
{
	UI_Warning_clearHasWarningFlag();
	if (!type) {
		return;
	}
	if (Data_CityInfo.treasury <= MIN_TREASURY) {
		Grid_andByteGrid(Data_Grid_bitfields, Bitfield_NoOverlayAndDeleted);
		UI_Warning_show(Warning_OutOfMoney);
		return;
	}
	if (type >= BUILDING_LARGE_TEMPLE_CERES && type <= BUILDING_LARGE_TEMPLE_VENUS && Data_CityInfo.resourceStored[RESOURCE_MARBLE] < 2) {
		Grid_andByteGrid(Data_Grid_bitfields, Bitfield_NoOverlayAndDeleted);
		UI_Warning_show(Warning_MarbleNeededLargeTemple);
		return;
	}
	if (type == BUILDING_ORACLE && Data_CityInfo.resourceStored[RESOURCE_MARBLE] < 2) {
		Grid_andByteGrid(Data_Grid_bitfields, Bitfield_NoOverlayAndDeleted);
		UI_Warning_show(Warning_MarbleNeededOracle);
		return;
	}
	if (type != BUILDING_CLEAR_LAND && Figure_hasNearbyEnemy(xStart, yStart, xEnd, yEnd)) {
		if (type == BUILDING_WALL || type == BUILDING_ROAD || type == BUILDING_AQUEDUCT) {
			Grid_copyShortGrid(Data_Grid_Undo_terrain, Data_Grid_terrain);
			Grid_copyByteGrid(Data_Grid_Undo_aqueducts, Data_Grid_aqueducts);
			Undo_restoreTerrainGraphics();
		} else if (type == BUILDING_PLAZA || type == BUILDING_GARDENS) {
			Grid_copyShortGrid(Data_Grid_Undo_terrain, Data_Grid_terrain);
			Grid_copyByteGrid(Data_Grid_Undo_aqueducts, Data_Grid_aqueducts);
			Grid_copyByteGrid(Data_Grid_Undo_bitfields, Data_Grid_bitfields);
			Grid_copyByteGrid(Data_Grid_Undo_edge, Data_Grid_edge);
			Undo_restoreTerrainGraphics();
		} else if (type == BUILDING_LOW_BRIDGE || type == BUILDING_SHIP_BRIDGE) {
			TerrainBridge_resetLength();
		} else {
			Grid_andByteGrid(Data_Grid_bitfields, Bitfield_NoOverlayAndDeleted);
		}
		UI_Warning_show(Warning_EnemyNearby);
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
		Routing_determineLandCitizen();
		Routing_determineLandNonCitizen();
	} else if (type == BUILDING_LOW_BRIDGE) {
		int length = TerrainBridge_addToSpriteGrid(xEnd, yEnd, 0);
		if (length <= 1) {
			UI_Warning_show(Warning_ShoreNeeded);
			return;
		}
		placementCost *= length;
	} else if (type == BUILDING_SHIP_BRIDGE) {
		int length = TerrainBridge_addToSpriteGrid(xEnd, yEnd, 1);
		if (length <= 1) {
			UI_Warning_show(Warning_ShoreNeeded);
			return;
		}
		placementCost *= length;
	} else if (type == BUILDING_AQUEDUCT) {
		int cost;
		if (!placeAqueduct(0, xStart, yStart, xEnd, yEnd, &cost)) {
			UI_Warning_show(Warning_ClearLandNeeded);
			return;
		}
		placementCost = cost;
		TerrainGraphics_updateRegionAqueduct(0, 0, Data_State.map.width - 1, Data_State.map.height - 1, 0);
		Routing_determineLandCitizen();
		Routing_determineLandNonCitizen();
	} else if (type == BUILDING_DRAGGABLE_RESERVOIR) {
		struct ReservoirInfo info;
		if (!placeReservoirAndAqueducts(0, xStart, yStart, xEnd, yEnd, &info)) {
			UI_Warning_show(Warning_ClearLandNeeded);
			return;
		}
		if (info.placeReservoirAtStart == PlaceReservoir_Yes) {
			int reservoirId = Building_create(BUILDING_RESERVOIR, xStart - 1, yStart - 1);
			Undo_addBuildingToList(reservoirId);
			Terrain_addBuildingToGrids(reservoirId, xStart-1, yStart-1, 3, image_group(GROUP_BUILDING_RESERVOIR), Terrain_Building);
			Data_Grid_aqueducts[GridOffset(xStart-1, yStart-1)] = 0;
		}
		if (info.placeReservoirAtEnd == PlaceReservoir_Yes) {
			int reservoirId = Building_create(BUILDING_RESERVOIR, xEnd - 1, yEnd - 1);
			Undo_addBuildingToList(reservoirId);
			Terrain_addBuildingToGrids(reservoirId, xEnd-1, yEnd-1, 3, image_group(GROUP_BUILDING_RESERVOIR), Terrain_Building);
			Data_Grid_aqueducts[GridOffset(xEnd-1, yEnd-1)] = 0;
			if (!Terrain_existsTileWithinAreaWithType(xStart - 2, yStart - 2, 5, Terrain_Water) && info.placeReservoirAtStart == PlaceReservoir_No) {
				UI_Warning_checkReservoirWater(BUILDING_RESERVOIR);
			}
		}
		placementCost = info.cost;
		TerrainGraphics_updateRegionAqueduct(0, 0, Data_State.map.width - 1, Data_State.map.height - 1, 0);
		Routing_determineLandCitizen();
		Routing_determineLandNonCitizen();
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
	CityInfo_Finance_spendOnConstruction(placementCost);
	if (type != BUILDING_TRIUMPHAL_ARCH) {
		Undo_recordBuild(placementCost);
	}
}

