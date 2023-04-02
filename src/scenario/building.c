#include "building.h"

#include "building/type.h"
#include "scenario/data.h"

int scenario_building_allowed(int building_type)
{
    switch (building_type) {
        case BUILDING_ROAD:
            return scenario.allowed_buildings[ALLOWED_BUILDING_ROAD];
        case BUILDING_DRAGGABLE_RESERVOIR:
        case BUILDING_AQUEDUCT:
        case BUILDING_FOUNTAIN:
            return scenario.allowed_buildings[ALLOWED_BUILDING_AQUEDUCT];
        case BUILDING_WELL:
            return scenario.allowed_buildings[ALLOWED_BUILDING_WELL];
        case BUILDING_BARBER:
            return scenario.allowed_buildings[ALLOWED_BUILDING_BARBER];
        case BUILDING_BATHHOUSE:
            return scenario.allowed_buildings[ALLOWED_BUILDING_BATHHOUSE];
        case BUILDING_DOCTOR:
            return scenario.allowed_buildings[ALLOWED_BUILDING_DOCTOR];
        case BUILDING_HOSPITAL:
            return scenario.allowed_buildings[ALLOWED_BUILDING_HOSPITAL];
        case BUILDING_MENU_SMALL_TEMPLES:
        case BUILDING_SMALL_TEMPLE_CERES:
        case BUILDING_SMALL_TEMPLE_NEPTUNE:
        case BUILDING_SMALL_TEMPLE_MERCURY:
        case BUILDING_SMALL_TEMPLE_MARS:
        case BUILDING_SMALL_TEMPLE_VENUS:
            return scenario.allowed_buildings[ALLOWED_BUILDING_SMALL_TEMPLES];
        case BUILDING_MENU_LARGE_TEMPLES:
        case BUILDING_LARGE_TEMPLE_CERES:
        case BUILDING_LARGE_TEMPLE_NEPTUNE:
        case BUILDING_LARGE_TEMPLE_MERCURY:
        case BUILDING_LARGE_TEMPLE_MARS:
        case BUILDING_LARGE_TEMPLE_VENUS:
            return scenario.allowed_buildings[ALLOWED_BUILDING_LARGE_TEMPLES];
        case BUILDING_ORACLE:
            return scenario.allowed_buildings[ALLOWED_BUILDING_ORACLE];
        case BUILDING_SCHOOL:
            return scenario.allowed_buildings[ALLOWED_BUILDING_SCHOOL];
        case BUILDING_ACADEMY:
            return scenario.allowed_buildings[ALLOWED_BUILDING_ACADEMY];
        case BUILDING_LIBRARY:
            return scenario.allowed_buildings[ALLOWED_BUILDING_LIBRARY];
        case BUILDING_THEATER:
            return scenario.allowed_buildings[ALLOWED_BUILDING_THEATER];
        case BUILDING_AMPHITHEATER:
            return scenario.allowed_buildings[ALLOWED_BUILDING_AMPHITHEATER];
        case BUILDING_COLOSSEUM:
            return scenario.allowed_buildings[ALLOWED_BUILDING_COLOSSEUM];
        case BUILDING_HIPPODROME:
            return scenario.allowed_buildings[ALLOWED_BUILDING_HIPPODROME];
        case BUILDING_GLADIATOR_SCHOOL:
            return scenario.allowed_buildings[ALLOWED_BUILDING_GLADIATOR_SCHOOL];
        case BUILDING_LION_HOUSE:
            return scenario.allowed_buildings[ALLOWED_BUILDING_LION_HOUSE];
        case BUILDING_ACTOR_COLONY:
            return scenario.allowed_buildings[ALLOWED_BUILDING_ACTOR_COLONY];
        case BUILDING_CHARIOT_MAKER:
            return scenario.allowed_buildings[ALLOWED_BUILDING_CHARIOT_MAKER];
        case BUILDING_FORUM:
            return scenario.allowed_buildings[ALLOWED_BUILDING_FORUM];
        case BUILDING_SENATE:
            return scenario.allowed_buildings[ALLOWED_BUILDING_SENATE];
        case BUILDING_GOVERNORS_HOUSE:
        case BUILDING_GOVERNORS_VILLA:
        case BUILDING_GOVERNORS_PALACE:
            return scenario.allowed_buildings[ALLOWED_BUILDING_GOVERNOR_HOME];
        case BUILDING_SMALL_STATUE:
        case BUILDING_MEDIUM_STATUE:
        case BUILDING_LARGE_STATUE:
            return scenario.allowed_buildings[ALLOWED_BUILDING_STATUES];
        case BUILDING_GARDENS:
            return scenario.allowed_buildings[ALLOWED_BUILDING_GARDENS];
        case BUILDING_PLAZA:
            return scenario.allowed_buildings[ALLOWED_BUILDING_PLAZA];
        case BUILDING_ENGINEERS_POST:
            return scenario.allowed_buildings[ALLOWED_BUILDING_ENGINEERS_POST];
        case BUILDING_MISSION_POST:
            return scenario.allowed_buildings[ALLOWED_BUILDING_MISSION_POST];
        case BUILDING_SHIPYARD:
        case BUILDING_WHARF:
            return scenario.allowed_buildings[ALLOWED_BUILDING_WHARF];
        case BUILDING_DOCK:
            return scenario.allowed_buildings[ALLOWED_BUILDING_DOCK];
        case BUILDING_WALL:
            return scenario.allowed_buildings[ALLOWED_BUILDING_WALL];
        case BUILDING_TOWER:
            return scenario.allowed_buildings[ALLOWED_BUILDING_TOWER];
        case BUILDING_GATEHOUSE:
            return scenario.allowed_buildings[ALLOWED_BUILDING_GATEHOUSE];
        case BUILDING_PREFECTURE:
            return scenario.allowed_buildings[ALLOWED_BUILDING_PREFECTURE];
        case BUILDING_FORT:
        case BUILDING_FORT_LEGIONARIES:
        case BUILDING_FORT_JAVELIN:
        case BUILDING_FORT_MOUNTED:
            return scenario.allowed_buildings[ALLOWED_BUILDING_FORT];
        case BUILDING_MILITARY_ACADEMY:
            return scenario.allowed_buildings[ALLOWED_BUILDING_MILITARY_ACADEMY];
        case BUILDING_BARRACKS:
            return scenario.allowed_buildings[ALLOWED_BUILDING_BARRACKS];
        case BUILDING_DISTRIBUTION_CENTER_UNUSED:
            return scenario.allowed_buildings[ALLOWED_BUILDING_DISTRIBUTION_CENTER];
        case BUILDING_MENU_FARMS:
        case BUILDING_WHEAT_FARM:
        case BUILDING_VEGETABLE_FARM:
        case BUILDING_FRUIT_FARM:
        case BUILDING_OLIVE_FARM:
        case BUILDING_VINES_FARM:
        case BUILDING_PIG_FARM:
            return scenario.allowed_buildings[ALLOWED_BUILDING_FARMS];
        case BUILDING_MENU_RAW_MATERIALS:
        case BUILDING_MARBLE_QUARRY:
        case BUILDING_IRON_MINE:
        case BUILDING_TIMBER_YARD:
        case BUILDING_CLAY_PIT:
            return scenario.allowed_buildings[ALLOWED_BUILDING_RAW_MATERIALS];
        case BUILDING_MENU_WORKSHOPS:
        case BUILDING_WINE_WORKSHOP:
        case BUILDING_OIL_WORKSHOP:
        case BUILDING_WEAPONS_WORKSHOP:
        case BUILDING_FURNITURE_WORKSHOP:
        case BUILDING_POTTERY_WORKSHOP:
            return scenario.allowed_buildings[ALLOWED_BUILDING_WORKSHOPS];
        case BUILDING_MARKET:
            return scenario.allowed_buildings[ALLOWED_BUILDING_MARKET];
        case BUILDING_GRANARY:
            return scenario.allowed_buildings[ALLOWED_BUILDING_GRANARY];
        case BUILDING_WAREHOUSE:
            return scenario.allowed_buildings[ALLOWED_BUILDING_WAREHOUSE];
        case BUILDING_LOW_BRIDGE:
        case BUILDING_SHIP_BRIDGE:
            return scenario.allowed_buildings[ALLOWED_BUILDING_BRIDGE];
    }
    return 1;
}

int scenario_building_image_native_hut(void)
{
    return scenario.native_images.hut;
}

int scenario_building_image_native_meeting(void)
{
    return scenario.native_images.meeting;
}

int scenario_building_image_native_crops(void)
{
    return scenario.native_images.crops;
}
