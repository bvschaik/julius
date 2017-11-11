#include "SidebarMenu.h"
#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Scenario.h"
#include "Data/Settings.h"
#include "UI/Sidebar.h"

#include "empire/city.h"
#include "game/tutorial.h"

#define MAX_BUILDINGITEMS 30
static int menuBuildingType[MAX_BUILDINGITEMS][MAX_BUILDINGITEMS] = {
	{BUILDING_HOUSE_VACANT_LOT, 0},
	{BUILDING_CLEAR_LAND, 0},
	{BUILDING_ROAD, 0},
	{BUILDING_DRAGGABLE_RESERVOIR, BUILDING_AQUEDUCT, BUILDING_FOUNTAIN, BUILDING_WELL, 0},
	{BUILDING_BARBER, BUILDING_BATHHOUSE, BUILDING_DOCTOR, BUILDING_HOSPITAL, 0},
    {BUILDING_MENU_SMALL_TEMPLES, BUILDING_MENU_LARGE_TEMPLES, BUILDING_ORACLE, 0},
	{BUILDING_SCHOOL, BUILDING_ACADEMY, BUILDING_LIBRARY, BUILDING_MISSION_POST, 0},
	{BUILDING_THEATER, BUILDING_AMPHITHEATER, BUILDING_COLOSSEUM, BUILDING_HIPPODROME,
		BUILDING_GLADIATOR_SCHOOL, BUILDING_LION_HOUSE, BUILDING_ACTOR_COLONY, BUILDING_CHARIOT_MAKER, 0},
    {BUILDING_FORUM, BUILDING_SENATE_UPGRADED, BUILDING_GOVERNORS_HOUSE, BUILDING_GOVERNORS_VILLA, BUILDING_GOVERNORS_PALACE,
        BUILDING_SMALL_STATUE, BUILDING_MEDIUM_STATUE, BUILDING_LARGE_STATUE, BUILDING_TRIUMPHAL_ARCH, 0},
	{BUILDING_GARDENS, BUILDING_PLAZA, BUILDING_ENGINEERS_POST, BUILDING_LOW_BRIDGE, BUILDING_SHIP_BRIDGE,
		BUILDING_SHIPYARD, BUILDING_DOCK, BUILDING_WHARF, 0},
	{BUILDING_WALL, BUILDING_TOWER, BUILDING_GATEHOUSE, BUILDING_PREFECTURE,
		BUILDING_FORT, BUILDING_MILITARY_ACADEMY, BUILDING_BARRACKS, 0},
	{BUILDING_MENU_FARMS, BUILDING_MENU_RAW_MATERIALS, BUILDING_MENU_WORKSHOPS,
		BUILDING_MARKET, BUILDING_GRANARY, BUILDING_WAREHOUSE, 0},
	// 12 = housing for debug (TODO: remove editor menus)
	{BUILDING_HOUSE_VACANT_LOT, BUILDING_HOUSE_LARGE_TENT, BUILDING_HOUSE_SMALL_SHACK, BUILDING_HOUSE_LARGE_SHACK,
        BUILDING_HOUSE_SMALL_HOVEL, BUILDING_HOUSE_LARGE_HOVEL, BUILDING_HOUSE_SMALL_CASA, BUILDING_HOUSE_LARGE_CASA,
        BUILDING_HOUSE_SMALL_INSULA, BUILDING_HOUSE_MEDIUM_INSULA, BUILDING_HOUSE_LARGE_INSULA, BUILDING_HOUSE_GRAND_INSULA,
        BUILDING_HOUSE_SMALL_VILLA, BUILDING_HOUSE_MEDIUM_VILLA, BUILDING_HOUSE_LARGE_VILLA, BUILDING_HOUSE_GRAND_VILLA,
        BUILDING_HOUSE_SMALL_PALACE, BUILDING_HOUSE_MEDIUM_PALACE, BUILDING_HOUSE_LARGE_PALACE, BUILDING_HOUSE_LUXURY_PALACE, 0},
	{1, 2, 3, 4, 5, 0}, // 13 brush size?
	{1, 2, 0}, // 14 people entry/exit?
	{1, 2, 3, 0}, // 15 elevation raise/lower/access
	{1, 2, 3, 4, 5, 6, 7, 8, 0}, // 16 invasion points
	{1, 2, 0}, // 17 river entry/exit
	{1, 2, 3, 0}, // 18 natives
	{BUILDING_WHEAT_FARM, BUILDING_VEGETABLE_FARM, BUILDING_FRUIT_FARM, BUILDING_OLIVE_FARM, BUILDING_VINES_FARM, BUILDING_PIG_FARM, 0},
	{BUILDING_CLAY_PIT, BUILDING_MARBLE_QUARRY, BUILDING_IRON_MINE, BUILDING_TIMBER_YARD, 0},
	{BUILDING_WINE_WORKSHOP, BUILDING_OIL_WORKSHOP, BUILDING_WEAPONS_WORKSHOP, BUILDING_FURNITURE_WORKSHOP, BUILDING_POTTERY_WORKSHOP, 0},
	{BUILDING_SMALL_TEMPLE_CERES, BUILDING_SMALL_TEMPLE_NEPTUNE, BUILDING_SMALL_TEMPLE_MERCURY, BUILDING_SMALL_TEMPLE_MARS, BUILDING_SMALL_TEMPLE_VENUS, 0},
	{BUILDING_LARGE_TEMPLE_CERES, BUILDING_LARGE_TEMPLE_NEPTUNE, BUILDING_LARGE_TEMPLE_MERCURY, BUILDING_LARGE_TEMPLE_MARS, BUILDING_LARGE_TEMPLE_VENUS, 0},
	{BUILDING_FORT_LEGIONARIES, BUILDING_FORT_JAVELIN, BUILDING_FORT_MOUNTED, 0},
	{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 0}, // 25 herd/fish points
	{0}, // 26 unused
	{0}, // 27 unused
	{0}, // 28 unused
	{0}, // 29 unused
};
static int menuEnabled[MAX_BUILDINGITEMS][MAX_BUILDINGITEMS];

#define ENABLE_HOUSE() if (buildingType >= BUILDING_HOUSE_VACANT_LOT && buildingType <= BUILDING_HOUSE_LUXURY_PALACE) menuEnabled[sub][item] = 1
#define ENABLE_IF(b,a) if (buildingType == b && Data_Scenario.allowedBuildings.a) menuEnabled[sub][item] = 1
#define ENABLE(b) if (buildingType == b) menuEnabled[sub][item] = 1
#define DISABLE_RAW(b,r) if (buildingType == b && !empire_can_produce_resource(r)) menuEnabled[sub][item] = 0
#define DISABLE_FINISHED(b,r) if (buildingType == b && !empire_can_produce_resource_potentially(r)) menuEnabled[sub][item] = 0

void SidebarMenu_enableBuildingButtons()
{
	UI_Sidebar_enableBuildingButtons();
}

static void enableNormal(int sub, int item, int buildingType)
{
	ENABLE_HOUSE();
	ENABLE(BUILDING_CLEAR_LAND);
	ENABLE_IF(BUILDING_ROAD, road);
	ENABLE_IF(BUILDING_DRAGGABLE_RESERVOIR, aqueduct);
	ENABLE_IF(BUILDING_AQUEDUCT, aqueduct);
	ENABLE_IF(BUILDING_FOUNTAIN, aqueduct);
	ENABLE_IF(BUILDING_WELL, well);
	ENABLE_IF(BUILDING_BARBER, barber);
	ENABLE_IF(BUILDING_BATHHOUSE, bathhouse);
	ENABLE_IF(BUILDING_DOCTOR, doctor);
	ENABLE_IF(BUILDING_HOSPITAL, hospital);
	ENABLE_IF(BUILDING_MENU_SMALL_TEMPLES, smallTemples);
	ENABLE_IF(BUILDING_MENU_LARGE_TEMPLES, largeTemples);
	ENABLE_IF(BUILDING_ORACLE, oracle);
	ENABLE_IF(BUILDING_SCHOOL, school);
	ENABLE_IF(BUILDING_ACADEMY, academy);
	ENABLE_IF(BUILDING_LIBRARY, library);
	ENABLE_IF(BUILDING_THEATER, theater);
	ENABLE_IF(BUILDING_AMPHITHEATER, amphitheater);
	ENABLE_IF(BUILDING_COLOSSEUM, colosseum);
	ENABLE_IF(BUILDING_HIPPODROME, hippodrome);
	ENABLE_IF(BUILDING_GLADIATOR_SCHOOL, gladiatorSchool);
	ENABLE_IF(BUILDING_LION_HOUSE, lionHouse);
	ENABLE_IF(BUILDING_ACTOR_COLONY, actorColony);
	ENABLE_IF(BUILDING_CHARIOT_MAKER, chariotMaker);
	ENABLE_IF(BUILDING_FORUM, forum);
	ENABLE_IF(BUILDING_SENATE_UPGRADED, senate);
	ENABLE_IF(BUILDING_GOVERNORS_HOUSE, governorHome);
	ENABLE_IF(BUILDING_GOVERNORS_VILLA, governorHome);
	ENABLE_IF(BUILDING_GOVERNORS_PALACE, governorHome);
	ENABLE_IF(BUILDING_SMALL_STATUE, statues);
	ENABLE_IF(BUILDING_MEDIUM_STATUE, statues);
	ENABLE_IF(BUILDING_LARGE_STATUE, statues);
	ENABLE_IF(BUILDING_GARDENS, gardens);
	ENABLE_IF(BUILDING_PLAZA, plaza);
	ENABLE_IF(BUILDING_ENGINEERS_POST, engineersPost);
	ENABLE_IF(BUILDING_MISSION_POST, missionPost);
	ENABLE_IF(BUILDING_SHIPYARD, wharf);
	ENABLE_IF(BUILDING_WHARF, wharf);
	ENABLE_IF(BUILDING_DOCK, dock);
	ENABLE_IF(BUILDING_WALL, wall);
	ENABLE_IF(BUILDING_TOWER, tower);
	ENABLE_IF(BUILDING_GATEHOUSE, gatehouse);
	ENABLE_IF(BUILDING_PREFECTURE, prefecture);
	ENABLE_IF(BUILDING_FORT, fort);
	ENABLE_IF(BUILDING_MILITARY_ACADEMY, militaryAcademy);
	ENABLE_IF(BUILDING_BARRACKS, barracks);
	ENABLE_IF(BUILDING_DISTRIBUTION_CENTER_UNUSED, distributionCenter);
	ENABLE_IF(BUILDING_MENU_FARMS, farms);
	ENABLE_IF(BUILDING_MENU_RAW_MATERIALS, rawMaterials);
	ENABLE_IF(BUILDING_MENU_WORKSHOPS, workshops);
	ENABLE_IF(BUILDING_MARKET, market);
	ENABLE_IF(BUILDING_GRANARY, granary);
	ENABLE_IF(BUILDING_WAREHOUSE, warehouse);
	ENABLE_IF(BUILDING_LOW_BRIDGE, bridge);
	ENABLE_IF(BUILDING_SHIP_BRIDGE, bridge);
	if (buildingType == BUILDING_TRIUMPHAL_ARCH) {
		if (Data_CityInfo.triumphalArchesAvailable > Data_CityInfo.triumphalArchesPlaced) {
			menuEnabled[sub][item] = 1;
		}
	}
}

static void enableTutorial1Start(int sub, int item, int buildingType)
{
	ENABLE_HOUSE();
	ENABLE(BUILDING_CLEAR_LAND);
	ENABLE_IF(BUILDING_WELL, well);
	ENABLE_IF(BUILDING_ROAD, road);
}

static void enableTutorial1AfterFire(int sub, int item, int buildingType)
{
	enableTutorial1Start(sub, item, buildingType);
	ENABLE_IF(BUILDING_PREFECTURE, prefecture);
	ENABLE_IF(BUILDING_MARKET, market);
}

static void enableTutorial1AfterCollapse(int sub, int item, int buildingType)
{
	enableTutorial1AfterFire(sub, item, buildingType);
	ENABLE_IF(BUILDING_ENGINEERS_POST, engineersPost);
	ENABLE_IF(BUILDING_SENATE_UPGRADED, senate);
}

static void enableTutorial2Start(int sub, int item, int buildingType)
{
	ENABLE_HOUSE();
	ENABLE(BUILDING_CLEAR_LAND);
	ENABLE_IF(BUILDING_WELL, well);
	ENABLE_IF(BUILDING_ROAD, road);
	ENABLE_IF(BUILDING_PREFECTURE, prefecture);
	ENABLE_IF(BUILDING_ENGINEERS_POST, engineersPost);
	ENABLE_IF(BUILDING_SENATE_UPGRADED, senate);
	ENABLE_IF(BUILDING_MARKET, market);
	ENABLE_IF(BUILDING_GRANARY, granary);
	ENABLE_IF(BUILDING_MENU_FARMS, farms);
	ENABLE_IF(BUILDING_MENU_SMALL_TEMPLES, smallTemples);
}

static void enableTutorial2UpTo250(int sub, int item, int buildingType)
{
	enableTutorial2Start(sub, item, buildingType);
	ENABLE_IF(BUILDING_DRAGGABLE_RESERVOIR, aqueduct);
	ENABLE_IF(BUILDING_AQUEDUCT, aqueduct);
	ENABLE_IF(BUILDING_FOUNTAIN, aqueduct);
}

static void enableTutorial2UpTo450(int sub, int item, int buildingType)
{
	enableTutorial2UpTo250(sub, item, buildingType);
	ENABLE_IF(BUILDING_GARDENS, gardens);
	ENABLE_IF(BUILDING_ACTOR_COLONY, actorColony);
	ENABLE_IF(BUILDING_THEATER, theater);
	ENABLE_IF(BUILDING_BATHHOUSE, bathhouse);
	ENABLE_IF(BUILDING_SCHOOL, school);
}

static void enableTutorial2After450(int sub, int item, int buildingType)
{
	enableTutorial2UpTo450(sub, item, buildingType);
	ENABLE_IF(BUILDING_MENU_RAW_MATERIALS, rawMaterials);
	ENABLE_IF(BUILDING_MENU_WORKSHOPS, workshops);
	ENABLE_IF(BUILDING_WAREHOUSE, warehouse);
	ENABLE_IF(BUILDING_FORUM, forum);
	ENABLE_IF(BUILDING_AMPHITHEATER, amphitheater);
	ENABLE_IF(BUILDING_GLADIATOR_SCHOOL, gladiatorSchool);
}

static void disableResources(int sub, int item, int buildingType)
{
	DISABLE_RAW(BUILDING_WHEAT_FARM, Resource_Wheat);
	DISABLE_RAW(BUILDING_VEGETABLE_FARM, Resource_Vegetables);
	DISABLE_RAW(BUILDING_FRUIT_FARM, Resource_Fruit);
	DISABLE_RAW(BUILDING_PIG_FARM, Resource_Meat);
	DISABLE_RAW(BUILDING_OLIVE_FARM, Resource_Olives);
	DISABLE_RAW(BUILDING_VINES_FARM, Resource_Vines);
	DISABLE_RAW(BUILDING_CLAY_PIT, Resource_Clay);
	DISABLE_RAW(BUILDING_TIMBER_YARD, Resource_Timber);
	DISABLE_RAW(BUILDING_IRON_MINE, Resource_Iron);
	DISABLE_RAW(BUILDING_MARBLE_QUARRY, Resource_Marble);
	DISABLE_FINISHED(BUILDING_POTTERY_WORKSHOP, Resource_Pottery);
	DISABLE_FINISHED(BUILDING_FURNITURE_WORKSHOP, Resource_Furniture);
	DISABLE_FINISHED(BUILDING_OIL_WORKSHOP, Resource_Oil);
	DISABLE_FINISHED(BUILDING_WINE_WORKSHOP, Resource_Wine);
	DISABLE_FINISHED(BUILDING_WEAPONS_WORKSHOP, Resource_Weapons);
}

void SidebarMenu_enableBuildingMenuItems()
{
    tutorial_build_buttons tutorialButtons = tutorial_get_build_buttons();
    for (int sub = 0; sub < MAX_BUILDINGITEMS; sub++) {
        for (int item = 0; item < MAX_BUILDINGITEMS; item++) {
            int buildingType = menuBuildingType[sub][item];
            //first 12 items always disabled
            if (sub < 12) {
                menuEnabled[sub][item] = 0;
            } else {
                menuEnabled[sub][item] = 1;
            }
            switch (tutorialButtons) {
                case TUT1_BUILD_START: enableTutorial1Start(sub, item, buildingType); break;
                case TUT1_BUILD_AFTER_FIRE: enableTutorial1AfterFire(sub, item, buildingType); break;
                case TUT1_BUILD_AFTER_COLLAPSE: enableTutorial1AfterCollapse(sub, item, buildingType); break;
                case TUT2_BUILD_START: enableTutorial2Start(sub, item, buildingType); break;
                case TUT2_BUILD_UP_TO_250: enableTutorial2UpTo250(sub, item, buildingType); break;
                case TUT2_BUILD_UP_TO_450: enableTutorial2UpTo450(sub, item, buildingType); break;
                case TUT2_BUILD_AFTER_450: enableTutorial2After450(sub, item, buildingType); break;
                default: enableNormal(sub, item, buildingType); break;
            }

            disableResources(sub, item, buildingType);
        }
    }
}

void SidebarMenu_enableBuildingMenuItemsAndButtons()
{
	SidebarMenu_enableBuildingMenuItems();
	SidebarMenu_enableBuildingButtons();
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
		if (menuEnabled[submenu][item] && menuBuildingType[submenu][item] > 0) {
			count++;
		}
	}
	return count;
}

int SidebarMenu_getNextBuildingItemIndex(int submenu, int currentItem)
{
	for (int i = currentItem + 1; i < MAX_BUILDINGITEMS; i++) {
		if (menuBuildingType[submenu][i] <= 0) {
			return 0;
		}
		if (menuEnabled[submenu][i]) {
			return i;
		}
	}
	return 0;
}

int SidebarMenu_getBuildingType(int submenu, int item)
{
	return menuBuildingType[submenu][item];
}
