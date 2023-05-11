#include "construction_warning.h"

#include "building/building.h"
#include "building/count.h"
#include "building/model.h"
#include "city/constants.h"
#include "city/labor.h"
#include "city/population.h"
#include "city/resource.h"
#include "city/warning.h"
#include "core/calc.h"
#include "empire/city.h"
#include "map/grid.h"
#include "map/road_access.h"
#include "map/terrain.h"
#include "scenario/property.h"

static int has_warning = 0;

void building_construction_warning_reset(void)
{
    has_warning = 0;
}

static void show(warning_type warning)
{
    city_warning_show(warning, NEW_WARNING_SLOT);
    has_warning = 1;
}

static void check_road_access(int type, int x, int y, int size)
{
    switch (type) {
        case BUILDING_SMALL_STATUE:
        case BUILDING_SMALL_STATUE_ALT:
        case BUILDING_SMALL_STATUE_ALT_B:
        case BUILDING_MEDIUM_STATUE:
        case BUILDING_LARGE_STATUE:
        case BUILDING_FOUNTAIN:
        case BUILDING_WELL:
        case BUILDING_RESERVOIR:
        case BUILDING_SMALL_POND:
        case BUILDING_LARGE_POND:
        case BUILDING_PINE_TREE:
        case BUILDING_FIR_TREE:
        case BUILDING_OAK_TREE:
        case BUILDING_ELM_TREE:
        case BUILDING_FIG_TREE:
        case BUILDING_PLUM_TREE:
        case BUILDING_PALM_TREE:
        case BUILDING_DATE_TREE:
        case BUILDING_PAVILION_BLUE:
        case BUILDING_PAVILION_RED:
        case BUILDING_PAVILION_ORANGE:
        case BUILDING_PAVILION_YELLOW:
        case BUILDING_PAVILION_GREEN:
        case BUILDING_OBELISK:
        case BUILDING_GATEHOUSE:
        case BUILDING_ROADBLOCK:
        case BUILDING_TRIUMPHAL_ARCH:
        case BUILDING_HOUSE_VACANT_LOT:
        case BUILDING_FORT:
        case BUILDING_FORT_LEGIONARIES:
        case BUILDING_FORT_JAVELIN:
        case BUILDING_FORT_MOUNTED:     
        case BUILDING_HORSE_STATUE:
        case BUILDING_DOLPHIN_FOUNTAIN:
        case BUILDING_HEDGE_DARK:
        case BUILDING_HEDGE_LIGHT:
        case BUILDING_GARDEN_WALL:
        case BUILDING_LEGION_STATUE:
        case BUILDING_DECORATIVE_COLUMN:
            return;
    }

    int has_road = 0;
    if (map_has_road_access(x, y, size, 0)) {
        has_road = 1;
    } else if (type == BUILDING_WAREHOUSE && map_has_road_access(x, y, size, 0)) {
        has_road = 1;
    } else if (type == BUILDING_HIPPODROME && map_has_road_access_hippodrome(x, y, 0)) {
        has_road = 1;
    } else if (type == BUILDING_LARARIUM && map_closest_road_within_radius(x, y, size, 2, 0, 0)) {
        has_road = 1;
    }

    if (!has_road) {
        show(WARNING_ROAD_ACCESS_NEEDED);
    }
}

static void check_water(int type, int x, int y)
{
    if (!has_warning) {
        if (type == BUILDING_FOUNTAIN || type == BUILDING_BATHHOUSE || type == BUILDING_SMALL_POND || type == BUILDING_LARGE_POND) {
            int grid_offset = map_grid_offset(x, y);
            int has_water = 0;
            if (map_terrain_is(grid_offset, TERRAIN_RESERVOIR_RANGE)) {
                has_water = 1;
            } else if (type == BUILDING_BATHHOUSE || type == BUILDING_SMALL_POND || type == BUILDING_LARGE_POND) {
                if (map_terrain_is(grid_offset + map_grid_delta(1, 0), TERRAIN_RESERVOIR_RANGE) ||
                    map_terrain_is(grid_offset + map_grid_delta(0, 1), TERRAIN_RESERVOIR_RANGE) ||
                    map_terrain_is(grid_offset + map_grid_delta(1, 1), TERRAIN_RESERVOIR_RANGE)) {
                    has_water = 1;
                }
            }
            if (!has_water) {
                show(WARNING_WATER_PIPE_ACCESS_NEEDED);
            }
        }
    }
}

static void check_workers(int type)
{
    if (!has_warning && type != BUILDING_WELL && !building_is_fort(type)) {
        if (model_get_building(type)->laborers > 0 && city_labor_workers_needed() >= 10) {
            show(WARNING_WORKERS_NEEDED);
        }
    }
}

static void check_market(int type)
{
    if (!has_warning && type == BUILDING_GRANARY) {
        if (building_count_active(BUILDING_MARKET) <= 0) {
            show(WARNING_BUILD_MARKET);
        }
    }
}

static void check_barracks(int type)
{
    if (!has_warning) {
        if (building_is_fort(type) && building_count_active(BUILDING_BARRACKS) <= 0 && building_count_active(BUILDING_GRAND_TEMPLE_MARS) <= 0) {
            show(WARNING_BUILD_BARRACKS);
        }
    }
}

static void check_weapons_access(int type)
{
    if (!has_warning && type == BUILDING_BARRACKS) {
        if (city_resource_count(RESOURCE_WEAPONS) <= 0) {
            show(WARNING_WEAPONS_NEEDED);
        }
    }
}

static void check_wall(int type, int x, int y, int size)
{
    if (!has_warning && type == BUILDING_TOWER) {
        if (!map_terrain_is_adjacent_to_wall(x, y, size)) {
            show(WARNING_SENTRIES_NEED_WALL);
        }
    }
}

static void check_actor_access(int type)
{
    if (!has_warning && type == BUILDING_THEATER) {
        if (building_count_active(BUILDING_ACTOR_COLONY) <= 0) {
            show(WARNING_BUILD_ACTOR_COLONY);
        }
    }
}

static void check_gladiator_access(int type)
{
    if (!has_warning && (type == BUILDING_AMPHITHEATER || type == BUILDING_COLOSSEUM || type == BUILDING_ARENA)) {
        if (building_count_active(BUILDING_GLADIATOR_SCHOOL) <= 0) {
            show(WARNING_BUILD_GLADIATOR_SCHOOL);
        }
    }
}

static void check_lion_access(int type)
{
    if (!has_warning && (type == BUILDING_COLOSSEUM || type == BUILDING_ARENA)) {
        if (building_count_active(BUILDING_LION_HOUSE) <= 0) {
            show(WARNING_BUILD_LION_HOUSE);
        }
    }
}

static void check_charioteer_access(int type)
{
    if (!has_warning && type == BUILDING_HIPPODROME) {
        if (building_count_active(BUILDING_CHARIOT_MAKER) <= 0) {
            show(WARNING_BUILD_CHARIOT_MAKER);
        }
    }
}

static void check_raw_material_access(building_type type)
{
    resource_type good = resource_get_from_industry(type);
    if (good == RESOURCE_NONE) {
        return;
    }
    const resource_type *raw_materials = resource_get_raw_materials_for_good(good);
    if (raw_materials == 0) {
        return;
    }
    for (int i = 0; raw_materials[i] != RESOURCE_NONE; i++) {
        const resource_data *data = resource_get_data(raw_materials[i]);
        if (building_count_active(data->industry) <= 0) {
            if (city_resource_count(good) <= 0 && city_resource_count(raw_materials[i]) <= 0) {
                show(data->warning.needed);
                if (empire_can_produce_resource(raw_materials[i])) {
                    show(data->warning.create_industry);
                } else if (!empire_can_import_resource(raw_materials[i])) {
                    show(WARNING_OPEN_TRADE_TO_IMPORT);
                } else if (!(city_resource_trade_status(raw_materials[i]) & TRADE_STATUS_IMPORT)) {
                    show(WARNING_TRADE_IMPORT_RESOURCE);
                }
            }
        }
    }
}

void building_construction_warning_check_all(building_type type, int x, int y, int size)
{
    building_construction_warning_check_food_stocks(type);
    check_workers(type);
    check_market(type);
    check_actor_access(type);
    check_gladiator_access(type);
    check_lion_access(type);
    check_charioteer_access(type);

    check_barracks(type);
    check_weapons_access(type);

    check_wall(type, x, y, size);
    check_water(type, x, y);

    check_raw_material_access(type);

    check_road_access(type, x, y, size);
}

void building_construction_warning_check_food_stocks(building_type type)
{
    if (!has_warning && type == BUILDING_HOUSE_VACANT_LOT) {
        if (city_population() >= 200 && !scenario_property_rome_supplies_wheat()) {
            if (city_resource_food_percentage_produced() <= 95) {
                show(WARNING_MORE_FOOD_NEEDED);
            }
        }
    }
}

void building_construction_warning_check_reservoir(building_type type)
{
    if (!has_warning && type == BUILDING_RESERVOIR) {
        if (building_count_active(BUILDING_RESERVOIR)) {
            show(WARNING_CONNECT_TO_RESERVOIR);
        } else {
            show(WARNING_PLACE_RESERVOIR_NEXT_TO_WATER);
        }
    }
}
