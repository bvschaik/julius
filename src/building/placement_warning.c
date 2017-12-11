#include "placement_warning.h"

#include "building/count.h"
#include "building/model.h"
#include "building/type.h"
#include "city/warning.h"
#include "core/calc.h"
#include "empire/city.h"
#include "map/grid.h"
#include "scenario/property.h"

#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Terrain.h"

static int has_warning = 0;

void building_placement_warning_reset()
{
    has_warning = 0;
}

static void show(warning_type warning)
{
    city_warning_show(warning);
    has_warning = 1;
}

static void checkRoadAccess(int buildingType, int x, int y, int size)
{
    switch (buildingType) {
        case BUILDING_SMALL_STATUE:
        case BUILDING_MEDIUM_STATUE:
        case BUILDING_LARGE_STATUE:
        case BUILDING_FOUNTAIN:
        case BUILDING_WELL:
        case BUILDING_RESERVOIR:
        case BUILDING_GATEHOUSE:
        case BUILDING_TRIUMPHAL_ARCH:
        case BUILDING_HOUSE_VACANT_LOT:
        case BUILDING_FORT:
        case BUILDING_FORT_LEGIONARIES:
        case BUILDING_FORT_JAVELIN:
        case BUILDING_FORT_MOUNTED:
            return;
    }

    int hasRoad = 0;
    if (Terrain_hasRoadAccess(x, y, size, 0, 0)) {
        hasRoad = 1;
    } else if (buildingType == BUILDING_WAREHOUSE && Terrain_hasRoadAccess(x, y, size, 0, 0)) {
        hasRoad = 1;
    } else if (buildingType == BUILDING_HIPPODROME && Terrain_hasRoadAccessHippodrome(x, y, 0, 0)) {
        hasRoad = 1;
    }
    if (!hasRoad) {
        show(WARNING_ROAD_ACCESS_NEEDED);
    }
}

static void checkWater(int buildingType, int x, int y)
{
    if (!has_warning) {
        if (buildingType == BUILDING_FOUNTAIN || buildingType == BUILDING_BATHHOUSE) {
            int gridOffset = map_grid_offset(x, y);
            int hasWater = 0;
            if (Data_Grid_terrain[gridOffset] & Terrain_ReservoirRange) {
                hasWater = 1;
            } else if (buildingType == BUILDING_BATHHOUSE) {
                if (Data_Grid_terrain[gridOffset + 1] & Terrain_ReservoirRange) {
                    hasWater = 1;
                } else if (Data_Grid_terrain[gridOffset + GRID_SIZE] & Terrain_ReservoirRange) {
                    hasWater = 1;
                } else if (Data_Grid_terrain[gridOffset + GRID_SIZE + 1] & Terrain_ReservoirRange) {
                    hasWater = 1;
                }
            }
            if (!hasWater) {
                show(WARNING_WATER_PIPE_ACCESS_NEEDED);
            }
        }
    }
}

static void checkWorkers(int buildingType)
{
    if (!has_warning && buildingType != BUILDING_WELL) {
        if (model_get_building(buildingType)->laborers > 0 &&
            Data_CityInfo.workersNeeded >= 10) {
            show(WARNING_WORKERS_NEEDED);
        }
    }
}

static void checkMarket(int buildingType)
{
    if (!has_warning && buildingType == BUILDING_GRANARY) {
        if (building_count_active(BUILDING_MARKET) <= 0) {
            show(WARNING_BUILD_MARKET);
        }
    }
}

static void checkBarracks(int buildingType)
{
    if (!has_warning) {
        if (buildingType == BUILDING_FORT_JAVELIN ||
            buildingType == BUILDING_FORT_LEGIONARIES ||
            buildingType == BUILDING_FORT_MOUNTED) {
            if (building_count_active(BUILDING_BARRACKS) <= 0) {
                show(WARNING_BUILD_BARRACKS);
            }
        }
    }
}

static void checkWeaponsAccess(int buildingType)
{
    if (!has_warning && buildingType == BUILDING_BARRACKS) {
        if (Data_CityInfo.resourceStored[RESOURCE_WEAPONS] <= 0) {
            show(WARNING_WEAPONS_NEEDED);
        }
    }
}

static void checkWall(int buildingType, int x, int y, int size)
{
    if (!has_warning && buildingType == BUILDING_TOWER) {
        if (!Terrain_isAdjacentToWall(x, y, size)) {
            show(WARNING_SENTRIES_NEED_WALL);
        }
    }
}

static void checkActorAccess(int buildingType)
{
    if (!has_warning && buildingType == BUILDING_THEATER) {
        if (building_count_active(BUILDING_ACTOR_COLONY) <= 0) {
            show(WARNING_BUILD_ACTOR_COLONY);
        }
    }
}

static void checkGladiatorAccess(int buildingType)
{
    if (!has_warning && buildingType == BUILDING_AMPHITHEATER) {
        if (building_count_active(BUILDING_GLADIATOR_SCHOOL) <= 0) {
            show(WARNING_BUILD_GLADIATOR_SCHOOL);
        }
    }
}

static void checkLionAccess(int buildingType)
{
    if (!has_warning && buildingType == BUILDING_COLOSSEUM) {
        if (building_count_active(BUILDING_LION_HOUSE) <= 0) {
            show(WARNING_BUILD_LION_HOUSE);
        }
    }
}

static void checkCharioteerAccess(int buildingType)
{
    if (!has_warning && buildingType == BUILDING_HIPPODROME) {
        if (building_count_active(BUILDING_CHARIOT_MAKER) <= 0) {
            show(WARNING_BUILD_CHARIOT_MAKER);
        }
    }
}

static void checkIronAccess(int buildingType)
{
    if (buildingType == BUILDING_WEAPONS_WORKSHOP &&
        building_count_industry_active(RESOURCE_IRON) <= 0) {
        if (Data_CityInfo.resourceStored[RESOURCE_WEAPONS] <= 0 &&
            Data_CityInfo.resourceStored[RESOURCE_IRON] <= 0) {
            show(WARNING_IRON_NEEDED);
            if (empire_can_produce_resource(RESOURCE_IRON)) {
                show(WARNING_BUILD_IRON_MINE);
            } else if (!empire_can_import_resource(RESOURCE_IRON)) {
                show(WARNING_OPEN_TRADE_TO_IMPORT);
            } else if (Data_CityInfo.resourceTradeStatus[RESOURCE_IRON] != TradeStatus_Import) {
                show(WARNING_TRADE_IMPORT_RESOURCE);
            }
        }
    }
}

static void checkVinesAccess(int buildingType)
{
    if (buildingType == BUILDING_WINE_WORKSHOP &&
        building_count_industry_active(RESOURCE_VINES) <= 0) {
        if (Data_CityInfo.resourceStored[RESOURCE_WINE] <= 0 &&
            Data_CityInfo.resourceStored[RESOURCE_VINES] <= 0) {
            show(WARNING_VINES_NEEDED);
            if (empire_can_produce_resource(RESOURCE_VINES)) {
                show(WARNING_BUILD_VINES_FARM);
            } else if (!empire_can_import_resource(RESOURCE_VINES)) {
                show(WARNING_OPEN_TRADE_TO_IMPORT);
            } else if (Data_CityInfo.resourceTradeStatus[RESOURCE_VINES] != TradeStatus_Import) {
                show(WARNING_TRADE_IMPORT_RESOURCE);
            }
        }
    }
}

static void checkOlivesAccess(int buildingType)
{
    if (buildingType == BUILDING_OIL_WORKSHOP &&
        building_count_industry_active(RESOURCE_OLIVES) <= 0) {
        if (Data_CityInfo.resourceStored[RESOURCE_OIL] <= 0 &&
            Data_CityInfo.resourceStored[RESOURCE_OLIVES] <= 0) {
            show(WARNING_OLIVES_NEEDED);
            if (empire_can_produce_resource(RESOURCE_OLIVES)) {
                show(WARNING_BUILD_OLIVE_FARM);
            } else if (!empire_can_import_resource(RESOURCE_OLIVES)) {
                show(WARNING_OPEN_TRADE_TO_IMPORT);
            } else if (Data_CityInfo.resourceTradeStatus[RESOURCE_OLIVES] != TradeStatus_Import) {
                show(WARNING_TRADE_IMPORT_RESOURCE);
            }
        }
    }
}

static void checkTimberAccess(int buildingType)
{
    if (buildingType == BUILDING_FURNITURE_WORKSHOP &&
        building_count_industry_active(RESOURCE_TIMBER) <= 0) {
        if (Data_CityInfo.resourceStored[RESOURCE_FURNITURE] <= 0 &&
            Data_CityInfo.resourceStored[RESOURCE_TIMBER] <= 0) {
            show(WARNING_TIMBER_NEEDED);
            if (empire_can_produce_resource(RESOURCE_TIMBER)) {
                show(WARNING_BUILD_TIMBER_YARD);
            } else if (!empire_can_import_resource(RESOURCE_TIMBER)) {
                show(WARNING_OPEN_TRADE_TO_IMPORT);
            } else if (Data_CityInfo.resourceTradeStatus[RESOURCE_TIMBER] != TradeStatus_Import) {
                show(WARNING_TRADE_IMPORT_RESOURCE);
            }
        }
    }
}

static void checkClayAccess(int buildingType)
{
    if (buildingType == BUILDING_POTTERY_WORKSHOP &&
        building_count_industry_active(RESOURCE_CLAY) <= 0) {
        if (Data_CityInfo.resourceStored[RESOURCE_POTTERY] <= 0 &&
            Data_CityInfo.resourceStored[RESOURCE_CLAY] <= 0) {
            show(WARNING_CLAY_NEEDED);
            if (empire_can_produce_resource(RESOURCE_CLAY)) {
                show(WARNING_BUILD_CLAY_PIT);
            } else if (!empire_can_import_resource(RESOURCE_CLAY)) {
                show(WARNING_OPEN_TRADE_TO_IMPORT);
            } else if (Data_CityInfo.resourceTradeStatus[RESOURCE_CLAY] != TradeStatus_Import) {
                show(WARNING_TRADE_IMPORT_RESOURCE);
            }
        }
    }
}

void building_placement_warning_check_all(building_type type, int x, int y, int size)
{
    building_placement_warning_check_food_stocks(type);
    checkWorkers(type);
    checkMarket(type);
    checkActorAccess(type);
    checkGladiatorAccess(type);
    checkLionAccess(type);
    checkCharioteerAccess(type);

    checkBarracks(type);
    checkWeaponsAccess(type);

    checkWall(type, x, y, size);
    checkWater(type, x, y);

    checkIronAccess(type);
    checkVinesAccess(type);
    checkOlivesAccess(type);
    checkTimberAccess(type);
    checkClayAccess(type);

    checkRoadAccess(type, x, y, size);
}

void building_placement_warning_check_food_stocks(building_type type)
{
    if (!has_warning && type == BUILDING_HOUSE_VACANT_LOT) {
        if (Data_CityInfo.population >= 200 && !scenario_property_rome_supplies_wheat()) {
            if (calc_percentage(Data_CityInfo.foodInfoFoodStoredLastMonth,
                    Data_CityInfo.foodInfoFoodConsumedLastMonth) <= 95) {
                show(WARNING_MORE_FOOD_NEEDED);
            }
        }
    }
}

void building_placement_warning_check_reservoir(building_type type)
{
    if (!has_warning && type == BUILDING_RESERVOIR) {
        if (building_count_active(BUILDING_RESERVOIR)) {
            show(WARNING_CONNECT_TO_RESERVOIR);
        } else {
            show(WARNING_PLACE_RESERVOIR_NEXT_TO_WATER);
        }
    }
}
