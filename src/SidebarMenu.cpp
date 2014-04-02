#include "SidebarMenu.h"
#include "Empire.h"
#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Scenario.h"
#include "Data/Settings.h"
#include "Data/Tutorial.h"
#include "UI/Sidebar.h"

#define MAX_BUILDINGITEMS 30
static int menuBuildingId[MAX_BUILDINGITEMS][MAX_BUILDINGITEMS] = {
	{Building_HouseVacantLot, 0},
	{Building_ClearLand, 0},
	{Building_Road, 0},
	{Building_ReservoirSimon, Building_Aqueduct, Building_Fountain, Building_Well, 0},
	{Building_Barber, Building_Bathhouse, Building_Doctor, Building_Hospital, 0},
	{Building_Menu_SmallTemples, Building_Menu_LargeTemples, Building_Oracle, 0},
	{Building_School, Building_Academy, Building_Library, Building_MissionPost, 0},
	{Building_Theater, Building_Amphitheater, Building_Colosseum, Building_Hippodrome,
		Building_GladiatorSchool, Building_LionHouse, Building_ActorColony, Building_ChariotMaker, 0},
	{Building_Forum, Building_SenateUpgraded, Building_GovernorsHouse, Building_GovernorsVilla, Building_GovernorsPalace,
		Building_SmallStatue, Building_MediumStatue, Building_LargeStatue, Building_TriumphalArch, 0},
	{Building_Gardens, Building_Plaza, Building_EngineersPost, Building_LowBridge, Building_ShipBridge,
		Building_Shipyard, Building_Dock, Building_Wharf, 0},
	{Building_Wall, Building_Tower, Building_Gatehouse, Building_Prefecture,
		Building_FortGround__, Building_MilitaryAcademy, Building_Barracks, 0},
	{Building_Menu_Farms, Building_Menu_RawMaterials, Building_Menu_Workshops,
		Building_Market, Building_Granary, Building_Warehouse, 0},
	// 12 = housing for debug (TODO: remove editor menus)
	{Building_HouseVacantLot, Building_HouseLargeTent, Building_HouseSmallShack, Building_HouseLargeShack,
		Building_HouseSmallHovel, Building_HouseLargeHovel, Building_HouseSmallCasa, Building_HouseLargeCasa,
		Building_HouseSmallInsula, Building_HouseMediumInsula, Building_HouseLargeInsula, Building_HouseGrandInsula,
		Building_HouseSmallVilla, Building_HouseMediumVilla, Building_HouseLargeVilla, Building_HouseGrandVilla,
		Building_HouseSmallPalace, Building_HouseMediumPalace, Building_HouseLargePalace, Building_HouseLuxuryPalace, 0},
	{1,2, 3, 4, 5, 0}, // 13 brush size?
	{1, 2, 0}, // 14 people entry/exit?
	{1, 2, 3, 0}, // 15 elevation raise/lower/access
	{1, 2, 3, 4, 5, 6, 7, 8, 0}, // 16 invasion points
	{1, 2, 0}, // 17 river entry/exit
	{1, 2, 3, 0}, // 18 natives
	{Building_WheatFarm, Building_VegetableFarm, Building_FruitFarm, Building_OliveFarm, Building_VinesFarm, Building_PigFarm, 0},
	{Building_ClayPit, Building_MarbleQuarry, Building_IronMine, Building_TimberYard, 0},
	{Building_WineWorkshop, Building_OilWorkshop, Building_WeaponsWorkshop, Building_FurnitureWorkshop, Building_PotteryWorkshop, 0},
	{Building_SmallTempleCeres, Building_SmallTempleNeptune, Building_SmallTempleMercury, Building_SmallTempleMars, Building_SmallTempleVenus, 0},
	{Building_LargeTempleCeres, Building_LargeTempleNeptune, Building_LargeTempleMercury, Building_LargeTempleMars, Building_LargeTempleVenus, 0},
	{Building_FortLegionaries, Building_FortJavelin, Building_FortMounted, 0},
	{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 0}, // 25 herd/fish points
	{0}, // 26 unused
	{0}, // 27 unused
	{0}, // 28 unused
	{0}, // 29 unused
};
static int menuEnabled[MAX_BUILDINGITEMS][MAX_BUILDINGITEMS];

#define ENABLE_HOUSE() if (buildingId >= Building_HouseVacantLot && buildingId <= Building_HouseLuxuryPalace) menuEnabled[sub][item] = 1
#define ENABLE_IF(b,a) if (buildingId == b && Data_Scenario.allowedBuildings.a) menuEnabled[sub][item] = 1
#define ENABLE(b) if (buildingId == b) menuEnabled[sub][item] = 1
#define DISABLE_RAW(b,r) if (buildingId == b && !Empire_ourCityCanProduceResource(r)) menuEnabled[sub][item] = 0
#define DISABLE_FINISHED(b,r) if (buildingId == b && !Empire_ourCityCanProduceResourcePotentially(r)) menuEnabled[sub][item] = 0

void SidebarMenu_enableBuildingButtons()
{
	UI_Sidebar_enableBuildingButtons();
}

static void enableNormal(int sub, int item, int buildingId)
{
	ENABLE_HOUSE();
	ENABLE(Building_ClearLand);
	ENABLE_IF(Building_Road, road);
	ENABLE_IF(Building_ReservoirSimon, aqueduct);
	ENABLE_IF(Building_Aqueduct, aqueduct);
	ENABLE_IF(Building_Fountain, aqueduct);
	ENABLE_IF(Building_Well, well);
	ENABLE_IF(Building_Barber, barber);
	ENABLE_IF(Building_Bathhouse, bathhouse);
	ENABLE_IF(Building_Doctor, doctor);
	ENABLE_IF(Building_Hospital, hospital);
	ENABLE_IF(Building_Menu_SmallTemples, smallTemples);
	ENABLE_IF(Building_Menu_LargeTemples, largeTemples);
	ENABLE_IF(Building_Oracle, oracle);
	ENABLE_IF(Building_School, school);
	ENABLE_IF(Building_Academy, academy);
	ENABLE_IF(Building_Library, library);
	ENABLE_IF(Building_Theater, theater);
	ENABLE_IF(Building_Amphitheater, amphitheater);
	ENABLE_IF(Building_Colosseum, colosseum);
	ENABLE_IF(Building_Hippodrome, hippodrome);
	ENABLE_IF(Building_GladiatorSchool, gladiatorSchool);
	ENABLE_IF(Building_LionHouse, lionHouse);
	ENABLE_IF(Building_ActorColony, actorColony);
	ENABLE_IF(Building_ChariotMaker, chariotMaker);
	ENABLE_IF(Building_Forum, forum);
	ENABLE_IF(Building_SenateUpgraded, senate);
	ENABLE_IF(Building_GovernorsHouse, governorHome);
	ENABLE_IF(Building_GovernorsVilla, governorHome);
	ENABLE_IF(Building_GovernorsPalace, governorHome);
	ENABLE_IF(Building_SmallStatue, statues);
	ENABLE_IF(Building_MediumStatue, statues);
	ENABLE_IF(Building_LargeStatue, statues);
	ENABLE_IF(Building_Gardens, gardens);
	ENABLE_IF(Building_Plaza, plaza);
	ENABLE_IF(Building_EngineersPost, engineersPost);
	ENABLE_IF(Building_MissionPost, missionPost);
	ENABLE_IF(Building_Shipyard, wharf);
	ENABLE_IF(Building_Wharf, wharf);
	ENABLE_IF(Building_Dock, dock);
	ENABLE_IF(Building_Wall, wall);
	ENABLE_IF(Building_Tower, tower);
	ENABLE_IF(Building_Gatehouse, gatehouse);
	ENABLE_IF(Building_Prefecture, prefecture);
	ENABLE_IF(Building_FortGround__, fort);
	ENABLE_IF(Building_MilitaryAcademy, militaryAcademy);
	ENABLE_IF(Building_Barracks, barracks);
	ENABLE_IF(Building_DistributionCenter_Unused, distributionCenter);
	ENABLE_IF(Building_Menu_Farms, farms);
	ENABLE_IF(Building_Menu_RawMaterials, rawMaterials);
	ENABLE_IF(Building_Menu_Workshops, workshops);
	ENABLE_IF(Building_Market, market);
	ENABLE_IF(Building_Granary, granary);
	ENABLE_IF(Building_Warehouse, warehouse);
	ENABLE_IF(Building_LowBridge, bridge);
	ENABLE_IF(Building_ShipBridge, bridge);
	if (buildingId == Building_TriumphalArch) {
		if (Data_CityInfo.triumphalArchesAvailable > Data_CityInfo.triumphalArchesPlaced) {
			menuEnabled[sub][item] = 1;
		}
	}
}

void enableTutorial1Start(int sub, int item, int buildingId)
{
	ENABLE_HOUSE();
	ENABLE(Building_ClearLand);
	ENABLE_IF(Building_Well, well);
	ENABLE_IF(Building_Road, road);
}

void enableTutorial1AfterFire(int sub, int item, int buildingId)
{
	enableTutorial1Start(sub, item, buildingId);
	ENABLE_IF(Building_Prefecture, prefecture);
	ENABLE_IF(Building_Market, market);
}

void enableTutorial1AfterCollapse(int sub, int item, int buildingId)
{
	enableTutorial1AfterFire(sub, item, buildingId);
	ENABLE_IF(Building_EngineersPost, engineersPost);
	ENABLE_IF(Building_SenateUpgraded, senate);
}

void enableTutorial2Start(int sub, int item, int buildingId)
{
	ENABLE_HOUSE();
	ENABLE(Building_ClearLand);
	ENABLE_IF(Building_Well, well);
	ENABLE_IF(Building_Road, road);
	ENABLE_IF(Building_Prefecture, prefecture);
	ENABLE_IF(Building_EngineersPost, engineersPost);
	ENABLE_IF(Building_SenateUpgraded, senate);
	ENABLE_IF(Building_Market, market);
	ENABLE_IF(Building_Granary, granary);
	ENABLE_IF(Building_Menu_Farms, farms);
	ENABLE_IF(Building_Menu_SmallTemples, smallTemples);
}

static void enableTutorial2UpTo250(int sub, int item, int buildingId)
{
	enableTutorial2Start(sub, item, buildingId);
	ENABLE_IF(Building_ReservoirSimon, aqueduct);
	ENABLE_IF(Building_Aqueduct, aqueduct);
	ENABLE_IF(Building_Fountain, aqueduct);
}

static void enableTutorial2UpTo450(int sub, int item, int buildingId)
{
	enableTutorial2UpTo250(sub, item, buildingId);
	ENABLE_IF(Building_Gardens, gardens);
	ENABLE_IF(Building_ActorColony, actorColony);
	ENABLE_IF(Building_Theater, theater);
	ENABLE_IF(Building_Bathhouse, bathhouse);
	ENABLE_IF(Building_School, school);
}

static void enableTutorial2After450(int sub, int item, int buildingId)
{
	enableTutorial2UpTo450(sub, item, buildingId);
	ENABLE_IF(Building_Menu_RawMaterials, rawMaterials);
	ENABLE_IF(Building_Menu_Workshops, workshops);
	ENABLE_IF(Building_Warehouse, warehouse);
	ENABLE_IF(Building_Forum, forum);
	ENABLE_IF(Building_Amphitheater, amphitheater);
	ENABLE_IF(Building_GladiatorSchool, gladiatorSchool);
}

static void disableResources(int sub, int item, int buildingId)
{
	DISABLE_RAW(Building_WheatFarm, Resource_Wheat);
	DISABLE_RAW(Building_VegetableFarm, Resource_Vegetables);
	DISABLE_RAW(Building_FruitFarm, Resource_Fruit);
	DISABLE_RAW(Building_PigFarm, Resource_Meat);
	DISABLE_RAW(Building_OliveFarm, Resource_Olives);
	DISABLE_RAW(Building_VinesFarm, Resource_Vines);
	DISABLE_RAW(Building_ClayPit, Resource_Clay);
	DISABLE_RAW(Building_TimberYard, Resource_Timber);
	DISABLE_RAW(Building_IronMine, Resource_Iron);
	DISABLE_RAW(Building_MarbleQuarry, Resource_Marble);
	DISABLE_FINISHED(Building_PotteryWorkshop, Resource_Pottery);
	DISABLE_FINISHED(Building_FurnitureWorkshop, Resource_Furniture);
	DISABLE_FINISHED(Building_OilWorkshop, Resource_Oil);
	DISABLE_FINISHED(Building_WineWorkshop, Resource_Wine);
	DISABLE_FINISHED(Building_WeaponsWorkshop, Resource_Weapons);
}

void SidebarMenu_enableBuildingMenuItems()
{
	for (int sub = 0; sub < MAX_BUILDINGITEMS; sub++) {
		for (int item = 0; item < MAX_BUILDINGITEMS; item++) {
			int buildingId = menuBuildingId[sub][item];
			//first 12 items always disabled
			if (sub < 12) {
				menuEnabled[sub][item] = 0;
			} else {
				menuEnabled[sub][item] = 1;
			}
			if (Data_Settings.currentMissionId == 0) { // tutorial 1
				if (!Data_Tutorial_tutorial1.fire && !Data_Tutorial_tutorial1.crime) {
					enableTutorial1Start(sub, item, buildingId);
				} else if (!Data_Tutorial_tutorial1.collapse) {
					enableTutorial1AfterFire(sub, item, buildingId);
				} else if (!Data_Tutorial_tutorial1.senateBuilt) {
					enableTutorial1AfterCollapse(sub, item, buildingId);
				} else {
					enableNormal(sub, item, buildingId);
				}
			} else if (Data_Settings.currentMissionId == 1) { // tutorial 2
				if (!Data_Tutorial_tutorial2.granaryBuilt) {
					enableTutorial2Start(sub, item, buildingId);
				} else if (!Data_Tutorial_tutorial2.population250Reached) {
					enableTutorial2UpTo250(sub, item, buildingId);
				} else if (!Data_Tutorial_tutorial2.population450Reached) {
					enableTutorial2UpTo450(sub, item, buildingId);
				} else if (!Data_Tutorial_tutorial2.potteryMade) {
					enableTutorial2After450(sub, item, buildingId);
				} else {
					enableNormal(sub, item, buildingId);
				}
			} else {
				enableNormal(sub, item, buildingId);
			}
			
			disableResources(sub, item, buildingId);
		}
	}
}

void SidebarMenu_enableAllBuildingMenuItems()
{
	for (int sub = 0; sub < MAX_BUILDINGITEMS; sub++) {
		for (int item = 0; item < MAX_BUILDINGITEMS; item++) {
			menuEnabled[sub][item] = 1;
		}
	}
}

int SidebarMenu_countBuildingMenuItems(int submenu)
{
	int count = 0;
	for (int item = 0; item < MAX_BUILDINGITEMS; item++) {
		if (menuEnabled[submenu][item]) {
			count++;
		}
	}
	return count;
}

int SidebarMenu_getNextBuildingItemIndex(int submenu, int currentItem)
{
	for (int i = currentItem + 1; i < MAX_BUILDINGITEMS; i++) {
		if (menuBuildingId[submenu][i] <= 0) {
			return 0;
		}
		if (menuEnabled[submenu][i]) {
			return menuBuildingId[submenu][i];
		}
	}
	return 0;
}

int SidebarMenu_getBuildingId(int submenu, int item)
{
	return menuBuildingId[submenu][item];
}
