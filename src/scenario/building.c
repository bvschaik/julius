#include "building.h"

#include "building/monument.h"
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
            return scenario.allowed_buildings[ALLOWED_BUILDING_SMALL_TEMPLES];
        case BUILDING_MENU_LARGE_TEMPLES:
            return scenario.allowed_buildings[ALLOWED_BUILDING_LARGE_TEMPLES];
        case BUILDING_MENU_GRAND_TEMPLES:
            return scenario.allowed_buildings[ALLOWED_BUILDING_LARGE_TEMPLES] && scenario.allowed_buildings[ALLOWED_BUILDING_MONUMENTS];
        case BUILDING_ORACLE:
        case BUILDING_SMALL_MAUSOLEUM:
        case BUILDING_LARGE_MAUSOLEUM:
        case BUILDING_NYMPHAEUM:
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
        case BUILDING_ARENA:
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
        case BUILDING_SENATE_UPGRADED:
            return scenario.allowed_buildings[ALLOWED_BUILDING_SENATE];
        case BUILDING_GOVERNORS_HOUSE:
        case BUILDING_GOVERNORS_VILLA:
        case BUILDING_GOVERNORS_PALACE:
            return scenario.allowed_buildings[ALLOWED_BUILDING_GOVERNOR_HOME];
        case BUILDING_SMALL_STATUE:
        case BUILDING_MEDIUM_STATUE:
        case BUILDING_LARGE_STATUE:
        case BUILDING_SMALL_STATUE_ALT:
        case BUILDING_SMALL_STATUE_ALT_B:
        case BUILDING_HORSE_STATUE:
        case BUILDING_LEGION_STATUE:
        case BUILDING_GLADIATOR_STATUE:
        case BUILDING_OBELISK:
        case BUILDING_SMALL_POND:
        case BUILDING_LARGE_POND:
        case BUILDING_MENU_STATUES:
            return scenario.allowed_buildings[ALLOWED_BUILDING_STATUES];
        case BUILDING_GARDENS:
        case BUILDING_MENU_TREES:
        case BUILDING_MENU_PATHS:
        case BUILDING_HEDGE_DARK:
        case BUILDING_HEDGE_LIGHT:
        case BUILDING_PAVILION_BLUE:
        case BUILDING_COLONNADE:
        case BUILDING_GARDEN_WALL:
        case BUILDING_ROOFED_GARDEN_WALL:
        case BUILDING_DECORATIVE_COLUMN:
            return scenario.allowed_buildings[ALLOWED_BUILDING_GARDENS];
        case BUILDING_MENU_PARKS:
            return scenario.allowed_buildings[ALLOWED_BUILDING_GARDENS] || scenario.allowed_buildings[ALLOWED_BUILDING_STATUES];
        case BUILDING_PLAZA:
            return scenario.allowed_buildings[ALLOWED_BUILDING_PLAZA];
        case BUILDING_ENGINEERS_POST:
            return scenario.allowed_buildings[ALLOWED_BUILDING_ENGINEERS_POST];
        case BUILDING_MISSION_POST:
            return scenario.allowed_buildings[ALLOWED_BUILDING_MISSION_POST];
        case BUILDING_SHIPYARD:
        case BUILDING_WHARF:
            return scenario.allowed_buildings[ALLOWED_BUILDING_WHARF];
        case BUILDING_LIGHTHOUSE:
            return (scenario.allowed_buildings[ALLOWED_BUILDING_WHARF] || scenario.allowed_buildings[ALLOWED_BUILDING_DOCK]) && scenario.allowed_buildings[ALLOWED_BUILDING_MONUMENTS];
        case BUILDING_DOCK:
            return scenario.allowed_buildings[ALLOWED_BUILDING_DOCK];
        case BUILDING_WALL:
        case BUILDING_PALISADE:
            return scenario.allowed_buildings[ALLOWED_BUILDING_WALL];
        case BUILDING_TOWER:
        case BUILDING_WATCHTOWER:
            return scenario.allowed_buildings[ALLOWED_BUILDING_TOWER];
        case BUILDING_GATEHOUSE:
            return scenario.allowed_buildings[ALLOWED_BUILDING_GATEHOUSE];
        case BUILDING_PREFECTURE:
            return scenario.allowed_buildings[ALLOWED_BUILDING_PREFECTURE];
        case BUILDING_FORT:
        case BUILDING_MESS_HALL:
            return scenario.allowed_buildings[ALLOWED_BUILDING_FORT];
        case BUILDING_MILITARY_ACADEMY:
            return scenario.allowed_buildings[ALLOWED_BUILDING_MILITARY_ACADEMY];
        case BUILDING_BARRACKS:
            return scenario.allowed_buildings[ALLOWED_BUILDING_BARRACKS];
        case BUILDING_MENU_FARMS:
            return scenario.allowed_buildings[ALLOWED_BUILDING_FARMS];
        case BUILDING_MENU_RAW_MATERIALS:
            return scenario.allowed_buildings[ALLOWED_BUILDING_RAW_MATERIALS];
        case BUILDING_MENU_WORKSHOPS:
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
