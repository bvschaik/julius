#include "menu.h"

#include "city/buildings.h"
#include "core/config.h"
#include "empire/city.h"
#include "game/tutorial.h"
#include "scenario/building.h"

static const building_type MENU_BUILDING_TYPE[BUILDING_MENU_MAX][BUILDING_MENU_MAX] = {
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
    {BUILDING_MENU_SMALL_TEMPLES, BUILDING_SMALL_TEMPLE_CERES, BUILDING_SMALL_TEMPLE_NEPTUNE, BUILDING_SMALL_TEMPLE_MERCURY, BUILDING_SMALL_TEMPLE_MARS, BUILDING_SMALL_TEMPLE_VENUS, 0},
    {BUILDING_MENU_LARGE_TEMPLES, BUILDING_LARGE_TEMPLE_CERES, BUILDING_LARGE_TEMPLE_NEPTUNE, BUILDING_LARGE_TEMPLE_MERCURY, BUILDING_LARGE_TEMPLE_MARS, BUILDING_LARGE_TEMPLE_VENUS, 0},
    {BUILDING_FORT_LEGIONARIES, BUILDING_FORT_JAVELIN, BUILDING_FORT_MOUNTED, 0},
    {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 0}, // 25 herd/fish points
    {0}, // 26 unused
    {0}, // 27 unused
    {0}, // 28 unused
    {0}, // 29 unused
};
static int menu_enabled[BUILDING_MENU_MAX][BUILDING_MENU_MAX];

static int changed = 1;

void building_menu_enable_all(void)
{
    for (int sub = 0; sub < BUILDING_MENU_MAX; sub++) {
        for (int item = 0; item < BUILDING_MENU_MAX; item++) {
            menu_enabled[sub][item] = 1;
        }
    }
}

static void enable_house(int *enabled, building_type menu_building_type)
{
    if (menu_building_type >= BUILDING_HOUSE_VACANT_LOT && menu_building_type <= BUILDING_HOUSE_LUXURY_PALACE) {
        *enabled = 1;
    }
}

static void enable_clear(int *enabled, building_type menu_building_type)
{
    if (menu_building_type == BUILDING_CLEAR_LAND) {
        *enabled = 1;
    }
}

static void enable_cycling_temples_if_allowed(building_type type)
{
    int sub = (type == BUILDING_MENU_SMALL_TEMPLES) ? BUILDING_MENU_ALL_SMALL_TEMPLES : BUILDING_MENU_ALL_LARGE_TEMPLES;
    menu_enabled[sub][0] = config_get(CONFIG_UI_ALLOW_CYCLING_TEMPLES);
}

static void enable_if_allowed(int *enabled, building_type menu_building_type, building_type type)
{
    if (menu_building_type == type && scenario_building_allowed(type)) {
        *enabled = 1;
        if (type == BUILDING_MENU_SMALL_TEMPLES || type == BUILDING_MENU_LARGE_TEMPLES) {
            enable_cycling_temples_if_allowed(type);
        }
    }
}

static void disable_raw(int *enabled, building_type menu_building_type, building_type type, int resource)
{
    if (type == menu_building_type && !empire_can_produce_resource(resource)) {
        *enabled = 0;
    }
}

static void disable_finished(int *enabled, building_type menu_building_type, building_type type, int resource)
{
    if (type == menu_building_type && !empire_can_produce_resource_potentially(resource)) {
        *enabled = 0;
    }
}

static void enable_normal(int *enabled, building_type type)
{
    enable_house(enabled, type);
    enable_clear(enabled, type);
    enable_if_allowed(enabled, type, BUILDING_ROAD);
    enable_if_allowed(enabled, type, BUILDING_DRAGGABLE_RESERVOIR);
    enable_if_allowed(enabled, type, BUILDING_AQUEDUCT);
    enable_if_allowed(enabled, type, BUILDING_FOUNTAIN);
    enable_if_allowed(enabled, type, BUILDING_WELL);
    enable_if_allowed(enabled, type, BUILDING_BARBER);
    enable_if_allowed(enabled, type, BUILDING_BATHHOUSE);
    enable_if_allowed(enabled, type, BUILDING_DOCTOR);
    enable_if_allowed(enabled, type, BUILDING_HOSPITAL);
    enable_if_allowed(enabled, type, BUILDING_MENU_SMALL_TEMPLES);
    enable_if_allowed(enabled, type, BUILDING_MENU_LARGE_TEMPLES);
    enable_if_allowed(enabled, type, BUILDING_ORACLE);
    enable_if_allowed(enabled, type, BUILDING_SCHOOL);
    enable_if_allowed(enabled, type, BUILDING_ACADEMY);
    enable_if_allowed(enabled, type, BUILDING_LIBRARY);
    enable_if_allowed(enabled, type, BUILDING_THEATER);
    enable_if_allowed(enabled, type, BUILDING_AMPHITHEATER);
    enable_if_allowed(enabled, type, BUILDING_COLOSSEUM);
    enable_if_allowed(enabled, type, BUILDING_HIPPODROME);
    enable_if_allowed(enabled, type, BUILDING_GLADIATOR_SCHOOL);
    enable_if_allowed(enabled, type, BUILDING_LION_HOUSE);
    enable_if_allowed(enabled, type, BUILDING_ACTOR_COLONY);
    enable_if_allowed(enabled, type, BUILDING_CHARIOT_MAKER);
    enable_if_allowed(enabled, type, BUILDING_FORUM);
    enable_if_allowed(enabled, type, BUILDING_SENATE_UPGRADED);
    enable_if_allowed(enabled, type, BUILDING_GOVERNORS_HOUSE);
    enable_if_allowed(enabled, type, BUILDING_GOVERNORS_VILLA);
    enable_if_allowed(enabled, type, BUILDING_GOVERNORS_PALACE);
    enable_if_allowed(enabled, type, BUILDING_SMALL_STATUE);
    enable_if_allowed(enabled, type, BUILDING_MEDIUM_STATUE);
    enable_if_allowed(enabled, type, BUILDING_LARGE_STATUE);
    enable_if_allowed(enabled, type, BUILDING_GARDENS);
    enable_if_allowed(enabled, type, BUILDING_PLAZA);
    enable_if_allowed(enabled, type, BUILDING_ENGINEERS_POST);
    enable_if_allowed(enabled, type, BUILDING_MISSION_POST);
    enable_if_allowed(enabled, type, BUILDING_SHIPYARD);
    enable_if_allowed(enabled, type, BUILDING_WHARF);
    enable_if_allowed(enabled, type, BUILDING_DOCK);
    enable_if_allowed(enabled, type, BUILDING_WALL);
    enable_if_allowed(enabled, type, BUILDING_TOWER);
    enable_if_allowed(enabled, type, BUILDING_GATEHOUSE);
    enable_if_allowed(enabled, type, BUILDING_PREFECTURE);
    enable_if_allowed(enabled, type, BUILDING_FORT);
    enable_if_allowed(enabled, type, BUILDING_MILITARY_ACADEMY);
    enable_if_allowed(enabled, type, BUILDING_BARRACKS);
    enable_if_allowed(enabled, type, BUILDING_DISTRIBUTION_CENTER_UNUSED);
    enable_if_allowed(enabled, type, BUILDING_MENU_FARMS);
    enable_if_allowed(enabled, type, BUILDING_MENU_RAW_MATERIALS);
    enable_if_allowed(enabled, type, BUILDING_MENU_WORKSHOPS);
    enable_if_allowed(enabled, type, BUILDING_MARKET);
    enable_if_allowed(enabled, type, BUILDING_GRANARY);
    enable_if_allowed(enabled, type, BUILDING_WAREHOUSE);
    enable_if_allowed(enabled, type, BUILDING_LOW_BRIDGE);
    enable_if_allowed(enabled, type, BUILDING_SHIP_BRIDGE);
    if (type == BUILDING_TRIUMPHAL_ARCH) {
        if (city_buildings_triumphal_arch_available()) {
            *enabled = 1;
        }
    }
}

static void enable_tutorial1_start(int *enabled, building_type type)
{
    enable_house(enabled, type);
    enable_clear(enabled, type);
    enable_if_allowed(enabled, type, BUILDING_WELL);
    enable_if_allowed(enabled, type, BUILDING_ROAD);
}

static void enable_tutorial1_after_fire(int *enabled, building_type type)
{
    enable_tutorial1_start(enabled, type);
    enable_if_allowed(enabled, type, BUILDING_PREFECTURE);
    enable_if_allowed(enabled, type, BUILDING_MARKET);
}

static void enable_tutorial1_after_collapse(int *enabled, building_type type)
{
    enable_tutorial1_after_fire(enabled, type);
    enable_if_allowed(enabled, type, BUILDING_ENGINEERS_POST);
    enable_if_allowed(enabled, type, BUILDING_SENATE_UPGRADED);
}

static void enable_tutorial2_start(int *enabled, building_type type)
{
    enable_house(enabled, type);
    enable_clear(enabled, type);
    enable_if_allowed(enabled, type, BUILDING_WELL);
    enable_if_allowed(enabled, type, BUILDING_ROAD);
    enable_if_allowed(enabled, type, BUILDING_PREFECTURE);
    enable_if_allowed(enabled, type, BUILDING_ENGINEERS_POST);
    enable_if_allowed(enabled, type, BUILDING_SENATE_UPGRADED);
    enable_if_allowed(enabled, type, BUILDING_MARKET);
    enable_if_allowed(enabled, type, BUILDING_GRANARY);
    enable_if_allowed(enabled, type, BUILDING_MENU_FARMS);
    enable_if_allowed(enabled, type, BUILDING_MENU_SMALL_TEMPLES);
}

static void enable_tutorial2_up_to_250(int *enabled, building_type type)
{
    enable_tutorial2_start(enabled, type);
    enable_if_allowed(enabled, type, BUILDING_DRAGGABLE_RESERVOIR);
    enable_if_allowed(enabled, type, BUILDING_AQUEDUCT);
    enable_if_allowed(enabled, type, BUILDING_FOUNTAIN);
}

static void enable_tutorial2_up_to_450(int *enabled, building_type type)
{
    enable_tutorial2_up_to_250(enabled, type);
    enable_if_allowed(enabled, type, BUILDING_GARDENS);
    enable_if_allowed(enabled, type, BUILDING_ACTOR_COLONY);
    enable_if_allowed(enabled, type, BUILDING_THEATER);
    enable_if_allowed(enabled, type, BUILDING_BATHHOUSE);
    enable_if_allowed(enabled, type, BUILDING_SCHOOL);
}

static void enable_tutorial2_after_450(int *enabled, building_type type)
{
    enable_tutorial2_up_to_450(enabled, type);
    enable_if_allowed(enabled, type, BUILDING_MENU_RAW_MATERIALS);
    enable_if_allowed(enabled, type, BUILDING_MENU_WORKSHOPS);
    enable_if_allowed(enabled, type, BUILDING_WAREHOUSE);
    enable_if_allowed(enabled, type, BUILDING_FORUM);
    enable_if_allowed(enabled, type, BUILDING_AMPHITHEATER);
    enable_if_allowed(enabled, type, BUILDING_GLADIATOR_SCHOOL);
}

static void disable_resources(int *enabled, building_type type)
{
    disable_raw(enabled, type, BUILDING_WHEAT_FARM, RESOURCE_WHEAT);
    disable_raw(enabled, type, BUILDING_VEGETABLE_FARM, RESOURCE_VEGETABLES);
    disable_raw(enabled, type, BUILDING_FRUIT_FARM, RESOURCE_FRUIT);
    disable_raw(enabled, type, BUILDING_PIG_FARM, RESOURCE_MEAT);
    disable_raw(enabled, type, BUILDING_OLIVE_FARM, RESOURCE_OLIVES);
    disable_raw(enabled, type, BUILDING_VINES_FARM, RESOURCE_VINES);
    disable_raw(enabled, type, BUILDING_CLAY_PIT, RESOURCE_CLAY);
    disable_raw(enabled, type, BUILDING_TIMBER_YARD, RESOURCE_TIMBER);
    disable_raw(enabled, type, BUILDING_IRON_MINE, RESOURCE_IRON);
    disable_raw(enabled, type, BUILDING_MARBLE_QUARRY, RESOURCE_MARBLE);
    disable_finished(enabled, type, BUILDING_POTTERY_WORKSHOP, RESOURCE_POTTERY);
    disable_finished(enabled, type, BUILDING_FURNITURE_WORKSHOP, RESOURCE_FURNITURE);
    disable_finished(enabled, type, BUILDING_OIL_WORKSHOP, RESOURCE_OIL);
    disable_finished(enabled, type, BUILDING_WINE_WORKSHOP, RESOURCE_WINE);
    disable_finished(enabled, type, BUILDING_WEAPONS_WORKSHOP, RESOURCE_WEAPONS);
}

void building_menu_update(void)
{
    tutorial_build_buttons tutorial_buttons = tutorial_get_build_buttons();
    for (int sub = 0; sub < BUILDING_MENU_MAX; sub++) {
        for (int item = 0; item < BUILDING_MENU_MAX; item++) {
            int building_type = MENU_BUILDING_TYPE[sub][item];
            int *menu_item = &menu_enabled[sub][item];
            // first 12 items always disabled
            if (sub < 12) {
                *menu_item = 0;
            } else {
                *menu_item = 1;
            }
            switch (tutorial_buttons) {
                case TUT1_BUILD_START:
                    enable_tutorial1_start(menu_item, building_type);
                    break;
                case TUT1_BUILD_AFTER_FIRE:
                    enable_tutorial1_after_fire(menu_item, building_type);
                    break;
                case TUT1_BUILD_AFTER_COLLAPSE:
                    enable_tutorial1_after_collapse(menu_item, building_type);
                    break;
                case TUT2_BUILD_START:
                    enable_tutorial2_start(menu_item, building_type);
                    break;
                case TUT2_BUILD_UP_TO_250:
                    enable_tutorial2_up_to_250(menu_item, building_type);
                    break;
                case TUT2_BUILD_UP_TO_450:
                    enable_tutorial2_up_to_450(menu_item, building_type);
                    break;
                case TUT2_BUILD_AFTER_450:
                    enable_tutorial2_after_450(menu_item, building_type);
                    break;
                default:
                    enable_normal(menu_item, building_type);
                    break;
            }

            disable_resources(menu_item, building_type);
        }
    }
    changed = 1;
}

int building_menu_count_items(int submenu)
{
    int count = 0;
    for (int item = 0; item < BUILDING_MENU_MAX; item++) {
        if (menu_enabled[submenu][item] && MENU_BUILDING_TYPE[submenu][item] > 0) {
            count++;
        }
    }
    return count;
}

int building_menu_next_index(int submenu, int current_index)
{
    for (int i = current_index + 1; i < BUILDING_MENU_MAX; i++) {
        if (MENU_BUILDING_TYPE[submenu][i] <= 0) {
            return 0;
        }
        if (menu_enabled[submenu][i]) {
            return i;
        }
    }
    return 0;
}

building_type building_menu_type(int submenu, int item)
{
    return MENU_BUILDING_TYPE[submenu][item];
}

int building_menu_has_changed(void)
{
    if (changed) {
        changed = 0;
        return 1;
    }
    return 0;
}
