#include "SidebarMenu.h"
#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "UI/Sidebar.h"

#include "empire/city.h"
#include "game/tutorial.h"
#include "scenario/building.h"

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


void SidebarMenu_enableBuildingButtons()
{
	UI_Sidebar_enableBuildingButtons();
}

static void enable_house(int *menu, int menu_building_type)
{
    if (menu_building_type >= BUILDING_HOUSE_VACANT_LOT && menu_building_type <= BUILDING_HOUSE_LUXURY_PALACE) {
        *menu = 1;
    }
}

static void enable_clear(int *menu, int menu_building_type)
{
    if (menu_building_type == BUILDING_CLEAR_LAND) {
        *menu = 1;
    }
}

static void enable_if_allowed(int *menu, int menu_building_type, int building_type)
{
    if (menu_building_type == building_type && scenario_building_allowed(building_type)) {
        *menu = 1;
    }
}

static void disable_raw(int *menu, int menu_building_type, int building_type, int resource)
{
    if (building_type == menu_building_type && !empire_can_produce_resource(resource)) {
        *menu = 0;
    }
}

static void disable_finished(int *menu, int menu_building_type, int building_type, int resource)
{
    if (building_type == menu_building_type && !empire_can_produce_resource_potentially(resource)) {
        *menu = 0;
    }
}

static void enableNormal(int *menu, int buildingType)
{
    enable_house(menu, buildingType);
	enable_clear(menu, buildingType);
	enable_if_allowed(menu, buildingType, BUILDING_ROAD);
	enable_if_allowed(menu, buildingType, BUILDING_DRAGGABLE_RESERVOIR);
	enable_if_allowed(menu, buildingType, BUILDING_AQUEDUCT);
	enable_if_allowed(menu, buildingType, BUILDING_FOUNTAIN);
	enable_if_allowed(menu, buildingType, BUILDING_WELL);
	enable_if_allowed(menu, buildingType, BUILDING_BARBER);
	enable_if_allowed(menu, buildingType, BUILDING_BATHHOUSE);
	enable_if_allowed(menu, buildingType, BUILDING_DOCTOR);
	enable_if_allowed(menu, buildingType, BUILDING_HOSPITAL);
	enable_if_allowed(menu, buildingType, BUILDING_MENU_SMALL_TEMPLES);
	enable_if_allowed(menu, buildingType, BUILDING_MENU_LARGE_TEMPLES);
	enable_if_allowed(menu, buildingType, BUILDING_ORACLE);
	enable_if_allowed(menu, buildingType, BUILDING_SCHOOL);
	enable_if_allowed(menu, buildingType, BUILDING_ACADEMY);
	enable_if_allowed(menu, buildingType, BUILDING_LIBRARY);
	enable_if_allowed(menu, buildingType, BUILDING_THEATER);
	enable_if_allowed(menu, buildingType, BUILDING_AMPHITHEATER);
	enable_if_allowed(menu, buildingType, BUILDING_COLOSSEUM);
	enable_if_allowed(menu, buildingType, BUILDING_HIPPODROME);
	enable_if_allowed(menu, buildingType, BUILDING_GLADIATOR_SCHOOL);
	enable_if_allowed(menu, buildingType, BUILDING_LION_HOUSE);
	enable_if_allowed(menu, buildingType, BUILDING_ACTOR_COLONY);
	enable_if_allowed(menu, buildingType, BUILDING_CHARIOT_MAKER);
	enable_if_allowed(menu, buildingType, BUILDING_FORUM);
	enable_if_allowed(menu, buildingType, BUILDING_SENATE_UPGRADED);
	enable_if_allowed(menu, buildingType, BUILDING_GOVERNORS_HOUSE);
	enable_if_allowed(menu, buildingType, BUILDING_GOVERNORS_VILLA);
	enable_if_allowed(menu, buildingType, BUILDING_GOVERNORS_PALACE);
	enable_if_allowed(menu, buildingType, BUILDING_SMALL_STATUE);
	enable_if_allowed(menu, buildingType, BUILDING_MEDIUM_STATUE);
	enable_if_allowed(menu, buildingType, BUILDING_LARGE_STATUE);
	enable_if_allowed(menu, buildingType, BUILDING_GARDENS);
	enable_if_allowed(menu, buildingType, BUILDING_PLAZA);
	enable_if_allowed(menu, buildingType, BUILDING_ENGINEERS_POST);
	enable_if_allowed(menu, buildingType, BUILDING_MISSION_POST);
	enable_if_allowed(menu, buildingType, BUILDING_SHIPYARD);
	enable_if_allowed(menu, buildingType, BUILDING_WHARF);
	enable_if_allowed(menu, buildingType, BUILDING_DOCK);
	enable_if_allowed(menu, buildingType, BUILDING_WALL);
	enable_if_allowed(menu, buildingType, BUILDING_TOWER);
	enable_if_allowed(menu, buildingType, BUILDING_GATEHOUSE);
	enable_if_allowed(menu, buildingType, BUILDING_PREFECTURE);
	enable_if_allowed(menu, buildingType, BUILDING_FORT);
	enable_if_allowed(menu, buildingType, BUILDING_MILITARY_ACADEMY);
	enable_if_allowed(menu, buildingType, BUILDING_BARRACKS);
	enable_if_allowed(menu, buildingType, BUILDING_DISTRIBUTION_CENTER_UNUSED);
	enable_if_allowed(menu, buildingType, BUILDING_MENU_FARMS);
	enable_if_allowed(menu, buildingType, BUILDING_MENU_RAW_MATERIALS);
	enable_if_allowed(menu, buildingType, BUILDING_MENU_WORKSHOPS);
	enable_if_allowed(menu, buildingType, BUILDING_MARKET);
	enable_if_allowed(menu, buildingType, BUILDING_GRANARY);
	enable_if_allowed(menu, buildingType, BUILDING_WAREHOUSE);
	enable_if_allowed(menu, buildingType, BUILDING_LOW_BRIDGE);
	enable_if_allowed(menu, buildingType, BUILDING_SHIP_BRIDGE);
    if (buildingType == BUILDING_TRIUMPHAL_ARCH) {
        if (Data_CityInfo.triumphalArchesAvailable > Data_CityInfo.triumphalArchesPlaced) {
            *menu = 1;
        }
    }
}

static void enableTutorial1Start(int *menu, int buildingType)
{
	enable_house(menu, buildingType);
	enable_clear(menu, buildingType);
	enable_if_allowed(menu, buildingType, BUILDING_WELL);
	enable_if_allowed(menu, buildingType, BUILDING_ROAD);
}

static void enableTutorial1AfterFire(int *menu, int buildingType)
{
	enableTutorial1Start(menu, buildingType);
	enable_if_allowed(menu, buildingType, BUILDING_PREFECTURE);
	enable_if_allowed(menu, buildingType, BUILDING_MARKET);
}

static void enableTutorial1AfterCollapse(int *menu, int buildingType)
{
	enableTutorial1AfterFire(menu, buildingType);
	enable_if_allowed(menu, buildingType, BUILDING_ENGINEERS_POST);
	enable_if_allowed(menu, buildingType, BUILDING_SENATE_UPGRADED);
}

static void enableTutorial2Start(int *menu, int buildingType)
{
	enable_house(menu, buildingType);
	enable_clear(menu, buildingType);
	enable_if_allowed(menu, buildingType, BUILDING_WELL);
	enable_if_allowed(menu, buildingType, BUILDING_ROAD);
	enable_if_allowed(menu, buildingType, BUILDING_PREFECTURE);
	enable_if_allowed(menu, buildingType, BUILDING_ENGINEERS_POST);
	enable_if_allowed(menu, buildingType, BUILDING_SENATE_UPGRADED);
	enable_if_allowed(menu, buildingType, BUILDING_MARKET);
	enable_if_allowed(menu, buildingType, BUILDING_GRANARY);
	enable_if_allowed(menu, buildingType, BUILDING_MENU_FARMS);
	enable_if_allowed(menu, buildingType, BUILDING_MENU_SMALL_TEMPLES);
}

static void enableTutorial2UpTo250(int *menu, int buildingType)
{
	enableTutorial2Start(menu, buildingType);
	enable_if_allowed(menu, buildingType, BUILDING_DRAGGABLE_RESERVOIR);
	enable_if_allowed(menu, buildingType, BUILDING_AQUEDUCT);
	enable_if_allowed(menu, buildingType, BUILDING_FOUNTAIN);
}

static void enableTutorial2UpTo450(int *menu, int buildingType)
{
	enableTutorial2UpTo250(menu, buildingType);
	enable_if_allowed(menu, buildingType, BUILDING_GARDENS);
	enable_if_allowed(menu, buildingType, BUILDING_ACTOR_COLONY);
	enable_if_allowed(menu, buildingType, BUILDING_THEATER);
	enable_if_allowed(menu, buildingType, BUILDING_BATHHOUSE);
	enable_if_allowed(menu, buildingType, BUILDING_SCHOOL);
}

static void enableTutorial2After450(int *menu, int buildingType)
{
	enableTutorial2UpTo450(menu, buildingType);
	enable_if_allowed(menu, buildingType, BUILDING_MENU_RAW_MATERIALS);
	enable_if_allowed(menu, buildingType, BUILDING_MENU_WORKSHOPS);
	enable_if_allowed(menu, buildingType, BUILDING_WAREHOUSE);
	enable_if_allowed(menu, buildingType, BUILDING_FORUM);
	enable_if_allowed(menu, buildingType, BUILDING_AMPHITHEATER);
	enable_if_allowed(menu, buildingType, BUILDING_GLADIATOR_SCHOOL);
}

static void disableResources(int *menu, int building_type)
{
    disable_raw(menu, building_type, BUILDING_WHEAT_FARM, RESOURCE_WHEAT);
    disable_raw(menu, building_type, BUILDING_VEGETABLE_FARM, RESOURCE_VEGETABLES);
    disable_raw(menu, building_type, BUILDING_FRUIT_FARM, RESOURCE_FRUIT);
    disable_raw(menu, building_type, BUILDING_PIG_FARM, RESOURCE_MEAT);
    disable_raw(menu, building_type, BUILDING_OLIVE_FARM, RESOURCE_OLIVES);
    disable_raw(menu, building_type, BUILDING_VINES_FARM, RESOURCE_VINES);
    disable_raw(menu, building_type, BUILDING_CLAY_PIT, RESOURCE_CLAY);
    disable_raw(menu, building_type, BUILDING_TIMBER_YARD, RESOURCE_TIMBER);
    disable_raw(menu, building_type, BUILDING_IRON_MINE, RESOURCE_IRON);
    disable_raw(menu, building_type, BUILDING_MARBLE_QUARRY, RESOURCE_MARBLE);
    disable_finished(menu, building_type, BUILDING_POTTERY_WORKSHOP, RESOURCE_POTTERY);
    disable_finished(menu, building_type, BUILDING_FURNITURE_WORKSHOP, RESOURCE_FURNITURE);
    disable_finished(menu, building_type, BUILDING_OIL_WORKSHOP, RESOURCE_OIL);
    disable_finished(menu, building_type, BUILDING_WINE_WORKSHOP, RESOURCE_WINE);
    disable_finished(menu, building_type, BUILDING_WEAPONS_WORKSHOP, RESOURCE_WEAPONS);
}

void SidebarMenu_enableBuildingMenuItems()
{
    tutorial_build_buttons tutorialButtons = tutorial_get_build_buttons();
    for (int sub = 0; sub < MAX_BUILDINGITEMS; sub++) {
        for (int item = 0; item < MAX_BUILDINGITEMS; item++) {
            int building_type = menuBuildingType[sub][item];
            int *menu_item = &menuEnabled[sub][item];
            // first 12 items always disabled
            if (sub < 12) {
                *menu_item = 0;
            } else {
                *menu_item = 1;
            }
            switch (tutorialButtons) {
                case TUT1_BUILD_START: enableTutorial1Start(menu_item, building_type); break;
                case TUT1_BUILD_AFTER_FIRE: enableTutorial1AfterFire(menu_item, building_type); break;
                case TUT1_BUILD_AFTER_COLLAPSE: enableTutorial1AfterCollapse(menu_item, building_type); break;
                case TUT2_BUILD_START: enableTutorial2Start(menu_item, building_type); break;
                case TUT2_BUILD_UP_TO_250: enableTutorial2UpTo250(menu_item, building_type); break;
                case TUT2_BUILD_UP_TO_450: enableTutorial2UpTo450(menu_item, building_type); break;
                case TUT2_BUILD_AFTER_450: enableTutorial2After450(menu_item, building_type); break;
                default: enableNormal(menu_item, building_type); break;
            }

            disableResources(menu_item, building_type);
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
