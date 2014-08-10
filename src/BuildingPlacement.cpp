
#include "Building.h"
#include "CityInfo.h"
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
#include "Walker.h"

#include "UI/PopupDialog.h"
#include "UI/Warning.h"
#include "UI/Window.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Formation.h"
#include "Data/Graphics.h"
#include "Data/Grid.h"
#include "Data/Model.h"
#include "Data/Settings.h"
#include "Data/State.h"
#include "Data/Walker.h"

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
	if (xMin < 0) xMin = 0;\
	if (xMax > Data_Settings_Map.width - 1) xMax = Data_Settings_Map.width - 1;\
	if (yMin < 0) yMin = 0;\
	if (yMax > Data_Settings_Map.height - 1) yMax = Data_Settings_Map.height - 1;

struct ReservoirInfo {
	int cost;
	int placeReservoirAtStart;
	int placeReservoirAtEnd;
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
	Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_Fort), Terrain_Building);
	int formationId = Formation_createLegion(buildingId);
	Data_Buildings[buildingId].formationId = formationId;
	if (type == Building_FortLegionaries) {
		Data_Buildings[buildingId].subtype.fortWalkerType = Walker_FortLegionary;
		Data_Formations[formationId].walkerType = Walker_FortLegionary;
	}
	if (type == Building_FortJavelin) {
		Data_Buildings[buildingId].subtype.fortWalkerType = Walker_FortJavelin;
		Data_Formations[formationId].walkerType = Walker_FortJavelin;
	}
	if (type == Building_FortMounted) {
		Data_Buildings[buildingId].subtype.fortWalkerType = Walker_FortMounted;
		Data_Formations[formationId].walkerType = Walker_FortMounted;
	}
	// create parade ground
	int groundId = Building_create(Building_FortGround, x + 3, y - 1);
	Undo_addBuildingToList(groundId);
	Data_Buildings[groundId].formationId = formationId;
	Data_Buildings[groundId].prevPartBuildingId = buildingId;
	Data_Buildings[buildingId].nextPartBuildingId = groundId;
	Data_Buildings[groundId].nextPartBuildingId = 0;
	Terrain_addBuildingToGrids(buildingId, x + 3, y - 1, 4,
		GraphicId(ID_Graphic_Fort) + 1, Terrain_Building);
}

static void addToTerrainHippodrome(int type, int buildingId, int x, int y, int size)
{
	int graphicId1 = GraphicId(ID_Graphic_Hippodrome1);
	int graphicId2 = GraphicId(ID_Graphic_Hippodrome2);
	Data_CityInfo.buildingHippodromePlaced = 1;

	struct Data_Building *part1 = &Data_Buildings[buildingId];
	if (Data_Settings_Map.orientation == 0 || Data_Settings_Map.orientation == 4) {
		part1->subtype.orientation = 0;
	} else {
		part1->subtype.orientation = 3;
	}
	int graphicId;
	switch (Data_Settings_Map.orientation) {
		case 0: graphicId = graphicId2; break;
		case 2: graphicId = graphicId1 + 4; break;
		case 4: graphicId = graphicId2 + 4; break;
		case 6: graphicId = graphicId1; break;
	}
	Terrain_addBuildingToGrids(buildingId, x, y, size, graphicId, Terrain_Building);

	int part2Id = Building_create(Building_Hippodrome, x + 5, y);
	struct Data_Building *part2 = &Data_Buildings[part2Id];
	Undo_addBuildingToList(part2Id);
	if (Data_Settings_Map.orientation == 0 || Data_Settings_Map.orientation == 4) {
		part2->subtype.orientation = 0;
	} else {
		part2->subtype.orientation = 3;
	}
	switch (Data_Settings_Map.orientation) {
		case 0: case 4: graphicId = graphicId2 + 2; break;
		case 2: case 6: graphicId = graphicId1 + 2; break;
	}
	Terrain_addBuildingToGrids(part2Id, x + 5, y, size, graphicId, Terrain_Building);

	int part3Id = Building_create(Building_Hippodrome, x + 10, y);
	struct Data_Building *part3 = &Data_Buildings[part3Id];
	Undo_addBuildingToList(part3Id);
	if (Data_Settings_Map.orientation == 0 || Data_Settings_Map.orientation == 4) {
		part3->subtype.orientation = 0;
	} else {
		part3->subtype.orientation = 3;
	}
	switch (Data_Settings_Map.orientation) {
		case 0: graphicId = graphicId2 + 4; break;
		case 2: graphicId = graphicId1; break;
		case 4: graphicId = graphicId2; break;
		case 6: graphicId = graphicId1 + 4; break;
	}
	Terrain_addBuildingToGrids(part3Id, x + 10, y, size, graphicId, Terrain_Building);
}

static int addToTerrainWarehouseSpace(int x, int y, int size, int prevId)
{
	int buildingId = Building_create(Building_WarehouseSpace, x, y);
	Undo_addBuildingToList(buildingId);
	Data_Buildings[buildingId].prevPartBuildingId = prevId;
	Data_Buildings[prevId].nextPartBuildingId = buildingId;
	Terrain_addBuildingToGrids(buildingId, x, y, size,
		GraphicId(ID_Graphic_WarehouseStorageEmpty), Terrain_Building);
	return buildingId;
}

static void addToTerrainWarehouse(int type, int buildingId, int x, int y, int size)
{
	Data_Buildings[buildingId].storageId = BuildingStorage_create();
	Data_Buildings[buildingId].prevPartBuildingId = 0;
	Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_Warehouse), Terrain_Building);

	int prev = buildingId;
	prev = addToTerrainWarehouseSpace(x + 1, y, size, prev);
	prev = addToTerrainWarehouseSpace(x + 2, y, size, prev);
	prev = addToTerrainWarehouseSpace(x, y + 1, size, prev);
	prev = addToTerrainWarehouseSpace(x + 1, y + 1, size, prev);
	prev = addToTerrainWarehouseSpace(x + 2, y + 1, size, prev);
	prev = addToTerrainWarehouseSpace(x, y + 2, size, prev);
	prev = addToTerrainWarehouseSpace(x + 1, y + 2, size, prev);
	prev = addToTerrainWarehouseSpace(x + 2, y + 2, size, prev);
	Data_Buildings[prev].nextPartBuildingId = 0;
}

static void addToTerrain(int type, int buildingId, int x, int y, int size,
	int orientation, int watersideOrientationAbs, int watersideOrientationRel)
{
	switch (type) {
		// houses
		case Building_HouseLargeTent:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_HouseTent) + 2, Terrain_Building);
			break;
		case Building_HouseSmallShack:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_HouseShack), Terrain_Building);
			break;
		case Building_HouseLargeShack:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_HouseShack) + 2, Terrain_Building);
			break;
		case Building_HouseSmallHovel:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_HouseHovel), Terrain_Building);
			break;
		case Building_HouseLargeHovel:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_HouseHovel) + 2, Terrain_Building);
			break;
		case Building_HouseSmallCasa:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_HouseCasa), Terrain_Building);
			break;
		case Building_HouseLargeCasa:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_HouseCasa) + 2, Terrain_Building);
			break;
		case Building_HouseSmallInsula:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_HouseInsula1), Terrain_Building);
			break;
		case Building_HouseMediumInsula:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_HouseInsula1) + 2, Terrain_Building);
			break;
		case Building_HouseLargeInsula:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_HouseInsula2), Terrain_Building);
			break;
		case Building_HouseGrandInsula:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_HouseInsula2) + 2, Terrain_Building);
			break;
		case Building_HouseSmallVilla:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_HouseVilla1), Terrain_Building);
			break;
		case Building_HouseMediumVilla:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_HouseVilla1) + 2, Terrain_Building);
			break;
		case Building_HouseLargeVilla:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_HouseVilla2), Terrain_Building);
			break;
		case Building_HouseGrandVilla:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_HouseVilla2) + 1, Terrain_Building);
			break;
		case Building_HouseSmallPalace:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_HousePalace1), Terrain_Building);
			break;
		case Building_HouseMediumPalace:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_HousePalace1) + 1, Terrain_Building);
			break;
		case Building_HouseLargePalace:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_HousePalace2), Terrain_Building);
			break;
		case Building_HouseLuxuryPalace:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_HousePalace2) + 1, Terrain_Building);
			break;
		// entertainment
		case Building_Amphitheater:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_Amphitheater), Terrain_Building);
			break;
		case Building_Theater:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_Theater), Terrain_Building);
			break;
		case Building_Colosseum:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_Colosseum), Terrain_Building);
			break;
		case Building_GladiatorSchool:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_GladiatorSchool), Terrain_Building);
			break;
		case Building_LionHouse:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_LionHouse), Terrain_Building);
			break;
		case Building_ActorColony:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_ActorColony), Terrain_Building);
			break;
		case Building_ChariotMaker:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_ChariotMaker), Terrain_Building);
			break;
		// statues
		case Building_SmallStatue:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_Statue), Terrain_Building);
			break;
		case Building_MediumStatue:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_Statue) + 1, Terrain_Building);
			break;
		case Building_LargeStatue:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_Statue) + 2, Terrain_Building);
			break;
		// health
		case Building_Doctor:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_Doctor), Terrain_Building);
			break;
		case Building_Hospital:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_Hospital), Terrain_Building);
			break;
		case Building_Bathhouse:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_BathhouseNoWater), Terrain_Building);
			break;
		case Building_Barber:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_Barber), Terrain_Building);
			break;
		// education
		case Building_School:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_School), Terrain_Building);
			break;
		case Building_Academy:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_Academy), Terrain_Building);
			break;
		case Building_Library:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_Library), Terrain_Building);
			break;
		// security
		case Building_Prefecture:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_Prefecture), Terrain_Building);
			break;
		// farms
		case Building_WheatFarm:
			TerrainGraphics_setBuildingFarm(buildingId, x, y, GraphicId(ID_Graphic_FarmCrops), 0);
			break;
		case Building_VegetableFarm:
			TerrainGraphics_setBuildingFarm(buildingId, x, y, GraphicId(ID_Graphic_FarmCrops) + 5, 0);
			break;
		case Building_FruitFarm:
			TerrainGraphics_setBuildingFarm(buildingId, x, y, GraphicId(ID_Graphic_FarmCrops) + 10, 0);
			break;
		case Building_OliveFarm:
			TerrainGraphics_setBuildingFarm(buildingId, x, y, GraphicId(ID_Graphic_FarmCrops) + 15, 0);
			break;
		case Building_VinesFarm:
			TerrainGraphics_setBuildingFarm(buildingId, x, y, GraphicId(ID_Graphic_FarmCrops) + 20, 0);
			break;
		case Building_PigFarm:
			TerrainGraphics_setBuildingFarm(buildingId, x, y, GraphicId(ID_Graphic_FarmCrops) + 25, 0);
			break;
		// industry
		case Building_MarbleQuarry:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_MarbleQuarry), Terrain_Building);
			break;
		case Building_IronMine:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_IronMine), Terrain_Building);
			break;
		case Building_TimberYard:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_TimberYard), Terrain_Building);
			break;
		case Building_ClayPit:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_ClayPit), Terrain_Building);
			break;
		// workshops
		case Building_WineWorkshop:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_WineWorkshop), Terrain_Building);
			break;
		case Building_OilWorkshop:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_OilWorkshop), Terrain_Building);
			break;
		case Building_WeaponsWorkshop:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_WeaponsWorkshop), Terrain_Building);
			break;
		case Building_FurnitureWorkshop:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_FurnitureWorkshop), Terrain_Building);
			break;
		case Building_PotteryWorkshop:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_PotteryWorkshop), Terrain_Building);
			break;
		// distribution
		case Building_Granary:
			Data_Buildings[buildingId].storageId = BuildingStorage_create();
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_Granary), Terrain_Building);
			break;
		case Building_Market:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_Market), Terrain_Building);
			break;
		// government
		case Building_GovernorsHouse:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_GovernorsHouse), Terrain_Building);
			break;
		case Building_GovernorsVilla:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_GovernorsVilla), Terrain_Building);
			break;
		case Building_GovernorsPalace:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_GovernorsPalace), Terrain_Building);
			break;
		case Building_MissionPost:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_MissionPost), Terrain_Building);
			break;
		case Building_EngineersPost:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_EngineersPost), Terrain_Building);
			break;
		case Building_Forum:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_Forum), Terrain_Building);
			break;
		// water
		case Building_Fountain:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_Fountain1), Terrain_Building);
			break;
		case Building_Well:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_Well), Terrain_Building);
			break;
		// military
		case Building_MilitaryAcademy:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_MilitaryAcademy), Terrain_Building);
			break;
		// religion
		case Building_SmallTempleCeres:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_TempleCeres), Terrain_Building);
			break;
		case Building_SmallTempleNeptune:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_TempleNeptune), Terrain_Building);
			break;
		case Building_SmallTempleMercury:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_TempleMercury), Terrain_Building);
			break;
		case Building_SmallTempleMars:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_TempleMars), Terrain_Building);
			break;
		case Building_SmallTempleVenus:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_TempleVenus), Terrain_Building);
			break;
		case Building_LargeTempleCeres:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_TempleCeres) + 1, Terrain_Building);
			break;
		case Building_LargeTempleNeptune:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_TempleNeptune) + 1, Terrain_Building);
			break;
		case Building_LargeTempleMercury:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_TempleMercury) + 1, Terrain_Building);
			break;
		case Building_LargeTempleMars:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_TempleMars) + 1, Terrain_Building);
			break;
		case Building_LargeTempleVenus:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_TempleVenus) + 1, Terrain_Building);
			break;
		case Building_Oracle:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_Oracle), Terrain_Building);
			break;
		case Building_Shipyard:
			Data_Buildings[buildingId].data.other.dockOrientation = watersideOrientationAbs;
			Terrain_addWatersideBuildingToGrids(buildingId, x, y, 2,
				GraphicId(ID_Graphic_Shipyard) + watersideOrientationRel);
			break;
		case Building_Wharf:
			Data_Buildings[buildingId].data.other.dockOrientation = watersideOrientationAbs;
			Terrain_addWatersideBuildingToGrids(buildingId, x, y, 2,
				GraphicId(ID_Graphic_Wharf) + watersideOrientationRel);
			break;
		case Building_Dock:
			Data_CityInfo.numWorkingDocks++;
			Data_Buildings[buildingId].data.other.dockOrientation = watersideOrientationAbs;
			{
				int graphicId;
				switch (watersideOrientationRel) {
					case 0: graphicId = GraphicId(ID_Graphic_Dock1); break;
					case 1: graphicId = GraphicId(ID_Graphic_Dock2); break;
					case 2: graphicId = GraphicId(ID_Graphic_Dock3); break;
					default:graphicId = GraphicId(ID_Graphic_Dock4); break;
				}
				Terrain_addWatersideBuildingToGrids(buildingId, x, y, size, graphicId);
			}
			break;
		case Building_Tower:
			Terrain_clearWithRadius(x, y, 2, 0, ~Terrain_Wall);
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_Tower),
				Terrain_Building | Terrain_Gatehouse);
			TerrainGraphics_updateAreaWalls(x, y, 5);
			break;
		case Building_Gatehouse:
			Terrain_addBuildingToGrids(buildingId, x, y, size,
				GraphicId(ID_Graphic_Gatehouse) + orientation, Terrain_Building | Terrain_Gatehouse);
			Data_Buildings[buildingId].subtype.orientation = orientation;
			Building_determineGraphicIdsForOrientedBuildings();
			Terrain_addRoadsForGatehouse(x, y, orientation);
			TerrainGraphics_updateAreaRoads(x, y, 5);
			TerrainGraphics_updateRegionPlazas(0, 0, Data_Settings_Map.width - 1, Data_Settings_Map.height - 1);
			TerrainGraphics_updateAreaWalls(x, y, 5);
			break;
		case Building_TriumphalArch:
			Terrain_addBuildingToGrids(buildingId, x, y, size,
				GraphicId(ID_Graphic_TriumphalArch) + orientation - 1, Terrain_Building);
			Data_Buildings[buildingId].subtype.orientation = orientation;
			Building_determineGraphicIdsForOrientedBuildings();
			Terrain_addRoadsForTriumphalArch(x, y, orientation);
			TerrainGraphics_updateAreaRoads(x, y, 5);
			TerrainGraphics_updateRegionPlazas(0, 0, Data_Settings_Map.width - 1, Data_Settings_Map.height - 1);
			Data_CityInfo.triumphalArchesPlaced++;
			SidebarMenu_enableBuildingMenuItems();
			Data_State.selectedBuilding.type = 0;
			break;
		case Building_SenateUpgraded:
			Data_CityInfo.buildingSenatePlaced = 1;
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_Senate), Terrain_Building);
			if (!Data_CityInfo.buildingSenateGridOffset) {
				Data_CityInfo.buildingSenateBuildingId = buildingId;
				Data_CityInfo.buildingSenateX = x;
				Data_CityInfo.buildingSenateY = y;
				Data_CityInfo.buildingSenateGridOffset = GridOffset(x, y);
			}
			break;
		case Building_Barracks:
			Terrain_addBuildingToGrids(buildingId, x, y, size, GraphicId(ID_Graphic_Barracks), Terrain_Building);
			if (!Data_CityInfo.buildingBarracksGridOffset) {
				Data_CityInfo.buildingBarracksBuildingId = buildingId;
				Data_CityInfo.buildingBarracksX = x;
				Data_CityInfo.buildingBarracksY = y;
				Data_CityInfo.buildingBarracksGridOffset = GridOffset(x, y);
			}
			break;
		case Building_Warehouse:
			addToTerrainWarehouse(type, buildingId, x, y, size);
			break;
		case Building_Hippodrome:
			addToTerrainHippodrome(type, buildingId, x, y, size);
			break;
		case Building_FortLegionaries:
		case Building_FortJavelin:
		case Building_FortMounted:
			addToTerrainFort(type, buildingId, x, y, size);
			break;
	}
	Routing_determineLandCitizen();
	Routing_determineLandNonCitizen();
	Routing_determineWalls();
}

static int placeBuilding(int type, int x, int y)
{
	unsigned short terrainMask = Terrain_All;
	if (type == Building_Gatehouse || type == Building_TriumphalArch) {
		terrainMask = ~Terrain_Road;
	} else if (type == Building_Tower) {
		terrainMask = ~Terrain_Wall;
	}
	int size = Constant_BuildingProperties[type].size;
	if (type == Building_Warehouse) {
		size = 3;
	}
	int orientation = 0;
	if (type == Building_Gatehouse) {
		orientation = Terrain_getOrientationGatehouse(x, y);
	} else if (type == Building_TriumphalArch) {
		orientation = Terrain_getOrientationTriumphalArch(x, y);
	}
	switch (orientation) {
		case 2: x = x - size + 1; break;
		case 4: x = x - size + 1; y = y - size + 1; break;
		case 6: y = y - size + 1; break;
	}
	// extra checks
	if (type == Building_Gatehouse) {
		if (!Terrain_isClear(x, y, size, terrainMask, 0)) {
			UI_Warning_show(Warning_ClearLandNeeded);
			return 0;
		}
		if (!orientation) {
			if (Data_State.selectedBuilding.roadRequired == 1) {
				orientation = 1;
			} else {
				orientation = 2;
			}
		}
	}
	if (type == Building_TriumphalArch) {
		if (!Terrain_isClear(x, y, size, terrainMask, 0)) {
			UI_Warning_show(Warning_ClearLandNeeded);
			return 0;
		}
		if (!orientation) {
			if (Data_State.selectedBuilding.roadRequired == 1) {
				orientation = 1;
			} else {
				orientation = 3;
			}
		}
	}
	int watersideOrientationAbs, watersideOrientationRel;
	if (type == Building_Shipyard || type == Building_Wharf) {
		if (Terrain_determineOrientationWatersideSize2(
				x, y, 0, &watersideOrientationAbs, &watersideOrientationRel)) {
			UI_Warning_show(Warning_ShoreNeeded);
			return 0;
		}
	} else if (type == Building_Dock) {
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
	if (type == Building_FortLegionaries || type == Building_FortJavelin || type == Building_FortMounted) {
		if (!Terrain_isClear(x + 3, y - 1, 4, terrainMask, 0)) {
			UI_Warning_show(Warning_ClearLandNeeded);
			return 0;
		}
		if (Data_Formation_Extra.numLegions >= 6) {
			UI_Warning_show(Warning_MaxLegionsReached);
			return 0;
		}
	}
	if (type == Building_Hippodrome) {
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
	if (type == Building_SenateUpgraded && Data_CityInfo.buildingSenatePlaced) {
		UI_Warning_show(Warning_OneBuildingOfType);
		return 0;
	}
	if (type == Building_Barracks && Data_CityInfo_Buildings.barracks.total) {
		UI_Warning_show(Warning_OneBuildingOfType);
		return 0;
	}
	UI_Warning_checkNewBuilding(type, x, y, size);

	// phew, checks done!
	int buildingId;
	if (type == Building_FortLegionaries || type == Building_FortJavelin || type == Building_FortMounted) {
		buildingId = Building_create(Building_FortGround__, x, y);
	} else {
		buildingId = Building_create(type, x, y);
	}
	Undo_addBuildingToList(buildingId);
	if (buildingId <= 0) {
		return 0;
	}
	addToTerrain(type, buildingId, x, y, size, orientation,
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
				int buildingId = Building_create(Building_HouseVacantLot, x, y);
				Undo_addBuildingToList(buildingId);
				if (buildingId > 0) {
					itemsPlaced++;
					Terrain_addBuildingToGrids(buildingId, x, y, 1,
						GraphicId(ID_Graphic_HouseVacantLot), Terrain_Building);
					if (!Terrain_existsTileWithinRadiusWithType(x, y, 1, 2, Terrain_Road)) {
						needsRoadWarning = 1;
					}
				}
			}
		}
	}
	if (!measureOnly) {
		UI_Warning_checkFoodStocks(Building_HouseVacantLot);
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
				if (type == Building_BurningRuin || type == Building_NativeCrops ||
					type == Building_NativeHut || type == Building_NativeMeeting) {
					continue;
				}
				if (Data_Buildings[buildingId].inUse == 6) {
					continue;
				}
				if (type == Building_FortGround || type == Building_FortGround__) {
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
				if (b->inUse != 6) {
					itemsPlaced++;
					Undo_addBuildingToList(buildingId);
				}
				b->inUse = 6;
				b->isDeleted = 1;
				int spaceId = buildingId;
				for (int i = 0; i < 9; i++) {
					spaceId = Data_Buildings[spaceId].prevPartBuildingId;
					if (spaceId <= 0) {
						break;
					}
					Undo_addBuildingToList(spaceId);
					Data_Buildings[spaceId].inUse = 6;
				}
				spaceId = buildingId;
				for (int i = 0; i < 9; i++) {
					spaceId = Data_Buildings[spaceId].nextPartBuildingId;
					if (spaceId <= 0) {
						break;
					}
					Undo_addBuildingToList(spaceId);
					Data_Buildings[spaceId].inUse = 6;
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
				if (!measureOnly && TerrainBridge_countWalkersOnBridge(gridOffset) > 0) {
					UI_Warning_show(Warning_PeopleOnBridge);
				} else {
					if (confirm.bridgeConfirmed == 1) {
						TerrainBridge_removeFromSpriteGrid(gridOffset, measureOnly);
						itemsPlaced++;
					}
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
	TerrainGraphics_updateRegionPlazas(0, 0, Data_Settings_Map.width - 1, Data_Settings_Map.height - 1);
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
	int askConfirmBridge;
	int askConfirmFort;
	for (int y = yMin; y <= yMax; y++) {
		for (int x = xMin; x <= xMax; x++) {
			int gridOffset = GridOffset(x,y);
			int buildingId = Data_Grid_buildingIds[gridOffset];
			if (buildingId) {
				if (Data_Buildings[buildingId].type == Building_FortGround__ ||
					Data_Buildings[buildingId].type == Building_FortGround) {
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
		UI_PopupDialog_show(14, confirmDeleteFort, 2);
	} else if (askConfirmBridge) {
		UI_PopupDialog_show(18, confirmDeleteBridge, 2);
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
		Routing_placeRoutedBuilding(xStart, yStart, xEnd, yEnd, RoutedBuilding_Road, &itemsPlaced)) {
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
		Routing_placeRoutedBuilding(xStart, yStart, xEnd, yEnd, RoutedBuilding_Wall, &itemsPlaced)) {
		if (!measureOnly) {
			Routing_determineLandCitizen();
			Routing_determineLandNonCitizen();
			UI_Window_requestRefresh();
		}
	}
}

static void placePlaza(int xStart, int yStart, int xEnd, int yEnd)
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
		Data_Settings_Map.width - 1, Data_Settings_Map.height - 1);
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

static int placeAqueduct(int xStart, int yStart, int xEnd, int yEnd, int *cost)
{
	// TODO fun_buildingAttemptToPlaceAqueduct
	return 0;
}

static int placeReservoirAndAqueducts(int xStart, int yStart, int xEnd, int yEnd, struct ReservoirInfo *info)
{
	// TODO
	return 0;
}

void BuildingPlacement_place(int orientation, int xStart, int yStart, int xEnd, int yEnd, int type)
{
	UI_Warning_clearHasWarningFlag();
	if (!type) {
		return;
	}
	if (Data_CityInfo.treasury <= MIN_TREASURY) {
		Grid_andByteGrid(Data_Grid_bitfields, 0xaf);
		UI_Warning_show(3);
		return;
	}
	if (type >= Building_LargeTempleCeres && type <= Building_LargeTempleVenus && Data_CityInfo.resourceStored[Resource_Marble] < 2) {
		Grid_andByteGrid(Data_Grid_bitfields, 0xaf);
		UI_Warning_show(15);
		return;
	}
	if (type == Building_Oracle && Data_CityInfo.resourceStored[Resource_Marble] < 2) {
		Grid_andByteGrid(Data_Grid_bitfields, 0xaf);
		UI_Warning_show(16);
		return;
	}
	if (type != Building_ClearLand && Walker_hasNearbyEnemy(xStart, yStart, xEnd, yEnd)) {
		if (type == Building_Wall || type == Building_Road || type == Building_Aqueduct) {
			Grid_copyShortGrid(Data_Grid_Undo_terrain, Data_Grid_terrain);
			Grid_copyByteGrid(Data_Grid_Undo_aqueducts, Data_Grid_aqueducts);
			Undo_restoreTerrainGraphics();
		} else if (type == Building_Plaza || type == Building_Gardens) {
			Grid_copyShortGrid(Data_Grid_Undo_terrain, Data_Grid_terrain);
			Grid_copyByteGrid(Data_Grid_Undo_aqueducts, Data_Grid_aqueducts);
			Grid_copyByteGrid(Data_Grid_Undo_bitfields, Data_Grid_bitfields);
			Grid_copyByteGrid(Data_Grid_Undo_edge, Data_Grid_edge);
			Undo_restoreTerrainGraphics();
		} else if (type == Building_LowBridge || type == Building_ShipBridge) {
			TerrainBridge_resetLength();
		} else {
			Grid_andByteGrid(Data_Grid_bitfields, 0xaf);
		}
		UI_Warning_show(50);
		return;
	}

	int placementCost = Data_Model_Buildings[type].cost;
	if (type == Building_ClearLand) {
		clearRegion(0, xStart, yStart, xEnd, yEnd);
		placementCost *= itemsPlaced;
	} else if (type == Building_Wall) {
		placeWall(0, xStart, yStart, xEnd, yEnd);
		placementCost *= itemsPlaced;
	} else if (type == Building_Road) {
		placeRoad(0, xStart, yStart, xEnd, yEnd);
		placementCost *= itemsPlaced;
	} else if (type == Building_Plaza) {
		placePlaza(xStart, yStart, xEnd, yEnd);
		placementCost *= itemsPlaced;
	} else if (type == Building_Gardens) {
		placeGarden(xStart, yStart, xEnd, yEnd);
		placementCost *= itemsPlaced;
		Routing_determineLandCitizen();
		Routing_determineLandNonCitizen();
	} else if (type == Building_LowBridge) {
		int length = TerrainBridge_addToSpriteGrid(xEnd, yEnd, 0);
		if (length <= 1) {
			UI_Warning_show(25);
			return;
		}
		placementCost *= length;
	} else if (type == Building_ShipBridge) {
		int length = TerrainBridge_addToSpriteGrid(xEnd, yEnd, 1);
		if (length <= 1) {
			UI_Warning_show(25);
			return;
		}
		placementCost *= length;
	} else if (type == Building_Aqueduct) {
		int cost;
		if (!placeAqueduct(xStart, yStart, xEnd, yEnd, &cost)) {
			UI_Warning_show(2);
			return;
		}
		placementCost = cost;
		TerrainGraphics_updateRegionAqueduct(0, 0, Data_Settings_Map.width - 1, Data_Settings_Map.height - 1);
		Routing_determineLandCitizen();
		Routing_determineLandNonCitizen();
	} else if (type == Building_DraggableReservoir) {
		struct ReservoirInfo info;
		if (!placeReservoirAndAqueducts(xStart, yStart, xEnd, yEnd, &info)) {
			UI_Warning_show(2);
			return;
		}
		if (info.placeReservoirAtStart == 1) {
			int reservoirId = Building_create(Building_Reservoir, xStart - 1, yStart - 1);
			Undo_addBuildingToList(reservoirId);
			Terrain_addBuildingToGrids(reservoirId, xStart-1, yStart-1, 3, GraphicId(ID_Graphic_Reservoir), Terrain_Building);
			Data_Grid_aqueducts[GridOffset(xStart-1, yStart-1)] = 0;
		}
		if (info.placeReservoirAtEnd == 1) {
			int reservoirId = Building_create(Building_Reservoir, xEnd - 1, yEnd - 1);
			Undo_addBuildingToList(reservoirId);
			Terrain_addBuildingToGrids(reservoirId, xEnd-1, yEnd-1, 3, GraphicId(ID_Graphic_Reservoir), Terrain_Building);
			Data_Grid_aqueducts[GridOffset(xEnd-1, yEnd-1)] = 0;
			if (!Terrain_existsTileWithinAreaWithType(xStart - 2, yStart - 2, 5, Terrain_Water) && !info.placeReservoirAtStart) {
				UI_Warning_checkReservoirWater(Building_Reservoir);
			}
		}
		placementCost = info.cost;
		TerrainGraphics_updateRegionAqueduct(0, 0, Data_Settings_Map.width - 1, Data_Settings_Map.height - 1);
		Routing_determineLandCitizen();
		Routing_determineLandNonCitizen();
	} else if (type == Building_HouseVacantLot) {
		placeHouses(0, xStart, yStart, xEnd, yEnd);
		placementCost *= itemsPlaced;
	} else {
		if (!placeBuilding(type, xEnd, yEnd)) {
			return;
		}
	}
	if ((type >= Building_LargeTempleCeres && type <= Building_LargeTempleVenus) || type == Building_Oracle) {
		Resource_removeFromCityWarehouses(Resource_Marble, 2);
	}
	Formation_moveHerdsAwayFrom(xEnd, yEnd);
	CityInfo_Finance_spendOnConstruction(placementCost);
	if (type != Building_TriumphalArch) {
		Undo_recordBuild(placementCost);
	}
}
