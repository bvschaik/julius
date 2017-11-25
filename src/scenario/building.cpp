#include "building.h"

#include "building/type.h"

#include <scenario>

int scenario_building_allowed(int building_type)
{
    switch (building_type)
    {
    case BUILDING_ROAD:
        return scenario.allowed_buildings.road;
    case BUILDING_DRAGGABLE_RESERVOIR:
    case BUILDING_AQUEDUCT:
    case BUILDING_FOUNTAIN:
        return scenario.allowed_buildings.aqueduct;
    case BUILDING_WELL:
        return scenario.allowed_buildings.well;
    case BUILDING_BARBER:
        return scenario.allowed_buildings.barber;
    case BUILDING_BATHHOUSE:
        return scenario.allowed_buildings.bathhouse;
    case BUILDING_DOCTOR:
        return scenario.allowed_buildings.doctor;
    case BUILDING_HOSPITAL:
        return scenario.allowed_buildings.hospital;
    case BUILDING_MENU_SMALL_TEMPLES:
        return scenario.allowed_buildings.small_temples;
    case BUILDING_MENU_LARGE_TEMPLES:
        return scenario.allowed_buildings.large_temples;
    case BUILDING_ORACLE:
        return scenario.allowed_buildings.oracle;
    case BUILDING_SCHOOL:
        return scenario.allowed_buildings.school;
    case BUILDING_ACADEMY:
        return scenario.allowed_buildings.academy;
    case BUILDING_LIBRARY:
        return scenario.allowed_buildings.library;
    case BUILDING_THEATER:
        return scenario.allowed_buildings.theater;
    case BUILDING_AMPHITHEATER:
        return scenario.allowed_buildings.amphitheater;
    case BUILDING_COLOSSEUM:
        return scenario.allowed_buildings.colosseum;
    case BUILDING_HIPPODROME:
        return scenario.allowed_buildings.hippodrome;
    case BUILDING_GLADIATOR_SCHOOL:
        return scenario.allowed_buildings.gladiator_school;
    case BUILDING_LION_HOUSE:
        return scenario.allowed_buildings.lion_house;
    case BUILDING_ACTOR_COLONY:
        return scenario.allowed_buildings.actor_colony;
    case BUILDING_CHARIOT_MAKER:
        return scenario.allowed_buildings.chariot_maker;
    case BUILDING_FORUM:
        return scenario.allowed_buildings.forum;
    case BUILDING_SENATE_UPGRADED:
        return scenario.allowed_buildings.senate;
    case BUILDING_GOVERNORS_HOUSE:
    case BUILDING_GOVERNORS_VILLA:
    case BUILDING_GOVERNORS_PALACE:
        return scenario.allowed_buildings.governor_home;
    case BUILDING_SMALL_STATUE:
    case BUILDING_MEDIUM_STATUE:
    case BUILDING_LARGE_STATUE:
        return scenario.allowed_buildings.statues;
    case BUILDING_GARDENS:
        return scenario.allowed_buildings.gardens;
    case BUILDING_PLAZA:
        return scenario.allowed_buildings.plaza;
    case BUILDING_ENGINEERS_POST:
        return scenario.allowed_buildings.engineers_post;
    case BUILDING_MISSION_POST:
        return scenario.allowed_buildings.mission_post;
    case BUILDING_SHIPYARD:
    case BUILDING_WHARF:
        return scenario.allowed_buildings.wharf;
    case BUILDING_DOCK:
        return scenario.allowed_buildings.dock;
    case BUILDING_WALL:
        return scenario.allowed_buildings.wall;
    case BUILDING_TOWER:
        return scenario.allowed_buildings.tower;
    case BUILDING_GATEHOUSE:
        return scenario.allowed_buildings.gatehouse;
    case BUILDING_PREFECTURE:
        return scenario.allowed_buildings.prefecture;
    case BUILDING_FORT:
        return scenario.allowed_buildings.fort;
    case BUILDING_MILITARY_ACADEMY:
        return scenario.allowed_buildings.military_academy;
    case BUILDING_BARRACKS:
        return scenario.allowed_buildings.barracks;
    case BUILDING_DISTRIBUTION_CENTER_UNUSED:
        return scenario.allowed_buildings.distribution_center;
    case BUILDING_MENU_FARMS:
        return scenario.allowed_buildings.farms;
    case BUILDING_MENU_RAW_MATERIALS:
        return scenario.allowed_buildings.raw_materials;
    case BUILDING_MENU_WORKSHOPS:
        return scenario.allowed_buildings.workshops;
    case BUILDING_MARKET:
        return scenario.allowed_buildings.market;
    case BUILDING_GRANARY:
        return scenario.allowed_buildings.granary;
    case BUILDING_WAREHOUSE:
        return scenario.allowed_buildings.warehouse;
    case BUILDING_LOW_BRIDGE:
    case BUILDING_SHIP_BRIDGE:
        return scenario.allowed_buildings.bridge;
    }
    return 1;
}

int scenario_building_image_native_hut()
{
    return scenario.native_images.hut;
}

int scenario_building_image_native_meeting()
{
    return scenario.native_images.meeting;
}

int scenario_building_image_native_crops()
{
    return scenario.native_images.crops;
}
