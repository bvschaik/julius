#include "resource.h"

#include "Data/CityInfo.h"

#include "building/building.h"
#include "building/industry.h"
#include "building/model.h"
#include "city/data_private.h"
#include "core/calc.h"
#include "empire/city.h"
#include "game/tutorial.h"
#include "map/road_access.h"
#include "scenario/building.h"
#include "scenario/property.h"

static struct {
    resource_list resource_list;
    resource_list food_list;
} available;

int city_resource_count(resource_type resource)
{
    return city_data.resource.stored_in_warehouses[resource];
}

const resource_list *city_resource_get_available()
{
    return &available.resource_list;
}

const resource_list *city_resource_get_available_foods()
{
    return &available.food_list;
}

int city_resource_has_workshop_with_room(int workshop_type)
{
    return city_data.resource.space_in_workshops[workshop_type] > 0;
}

void city_resource_add_to_warehouse(resource_type resource, int amount)
{
    city_data.resource.space_in_warehouses[resource] -= amount;
    city_data.resource.stored_in_warehouses[resource] += amount;
}

void city_resource_remove_from_warehouse(resource_type resource, int amount)
{
    city_data.resource.space_in_warehouses[resource] += amount;
    city_data.resource.stored_in_warehouses[resource] -= amount;
}

void city_resource_calculate_warehouse_stocks()
{
    for (int i = 0; i < RESOURCE_MAX; i++) {
        city_data.resource.space_in_warehouses[i] = 0;
        city_data.resource.stored_in_warehouses[i] = 0;
    }
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_WAREHOUSE) {
            b->hasRoadAccess = 0;
            if (map_has_road_access(b->x, b->y, b->size, 0, 0)) {
                b->hasRoadAccess = 1;
            } else if (map_has_road_access(b->x, b->y, 3, 0, 0)) {
                b->hasRoadAccess = 2;
            }
        }
    }
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE || b->type != BUILDING_WAREHOUSE_SPACE) {
            continue;
        }
        building *warehouse = building_main(b);
        if (warehouse->hasRoadAccess) {
            b->hasRoadAccess = warehouse->hasRoadAccess;
            if (b->subtype.warehouseResourceId) {
                int loads = b->loadsStored;
                int resource = b->subtype.warehouseResourceId;
                city_data.resource.stored_in_warehouses[resource] += loads;
                city_data.resource.space_in_warehouses[resource] += 4 - loads;
            } else {
                city_data.resource.space_in_warehouses[RESOURCE_NONE] += 4;
            }
        }
    }
}

void city_resource_determine_available()
{
    for (int i = 0; i < RESOURCE_MAX; i++) {
        available.resource_list.items[i] = 0;
        available.food_list.items[i] = 0;
    }
    available.resource_list.size = 0;
    available.food_list.size = 0;

    for (int i = RESOURCE_MIN; i < RESOURCE_MAX; i++) {
        if (empire_can_produce_resource(i) || empire_can_import_resource(i) ||
            (i == RESOURCE_MEAT && scenario_building_allowed(BUILDING_WHARF))) {
            available.resource_list.items[available.resource_list.size++] = i;
        }
    }
    for (int i = RESOURCE_MIN_FOOD; i < RESOURCE_MAX_FOOD; i++) {
        if (i == RESOURCE_OLIVES || i == RESOURCE_VINES) {
            continue;
        }
        if (empire_can_produce_resource(i) || empire_can_import_resource(i) ||
            (i == RESOURCE_MEAT && scenario_building_allowed(BUILDING_WHARF))) {
            available.food_list.items[available.food_list.size++] = i;
        }
    }
}

static void calculate_available_food()
{
    for (int i = 0; i < RESOURCE_MAX_FOOD; i++) {
        Data_CityInfo.resourceGranaryFoodStored[i] = 0;
    }
    Data_CityInfo.foodInfoFoodStoredInGranaries = 0;
    Data_CityInfo.foodInfoFoodTypesAvailable = 0;
    Data_CityInfo.foodInfoFoodSupplyMonths = 0;
    Data_CityInfo.foodInfoGranariesOperating = 0;
    Data_CityInfo.foodInfoGranariesUnderstaffed = 0;
    Data_CityInfo.foodInfoGranariesNotOperating = 0;
    Data_CityInfo.foodInfoGranariesNotOperatingWithFood = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE || b->type != BUILDING_GRANARY) {
            continue;
        }
        b->hasRoadAccess = 0;
        if (map_has_road_access_granary(b->x, b->y, 0, 0)) {
            b->hasRoadAccess = 1;
            int pct_workers = calc_percentage(
                b->numWorkers, model_get_building(b->type)->laborers);
            if (pct_workers < 100) {
                Data_CityInfo.foodInfoGranariesUnderstaffed++;
            }
            int amount_stored = 0;
            for (int r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
                amount_stored += b->data.granary.resource_stored[r];
            }
            if (pct_workers < 50) {
                Data_CityInfo.foodInfoGranariesNotOperating++;
                if (amount_stored > 0) {
                    Data_CityInfo.foodInfoGranariesNotOperatingWithFood++;
                }
            } else {
                Data_CityInfo.foodInfoGranariesOperating++;
                for (int r = 0; r < RESOURCE_MAX_FOOD; r++) {
                    Data_CityInfo.resourceGranaryFoodStored[r] += b->data.granary.resource_stored[r];
                }
                if (amount_stored > 400) {
                    tutorial_on_filled_granary();
                }
            }
        }
    }
    for (int i = RESOURCE_MIN_FOOD; i < RESOURCE_MAX_FOOD; i++) {
        if (Data_CityInfo.resourceGranaryFoodStored[i]) {
            Data_CityInfo.foodInfoFoodStoredInGranaries += Data_CityInfo.resourceGranaryFoodStored[i];
            Data_CityInfo.foodInfoFoodTypesAvailable++;
        }
    }
    Data_CityInfo.foodInfoFoodNeededPerMonth =
        calc_adjust_with_percentage(city_data.population.population, 50);
    if (Data_CityInfo.foodInfoFoodNeededPerMonth > 0) {
        Data_CityInfo.foodInfoFoodSupplyMonths =
            Data_CityInfo.foodInfoFoodStoredInGranaries / Data_CityInfo.foodInfoFoodNeededPerMonth;
    } else {
        Data_CityInfo.foodInfoFoodSupplyMonths =
            Data_CityInfo.foodInfoFoodStoredInGranaries > 0 ? 1 : 0;
    }
    if (scenario_property_rome_supplies_wheat()) {
        Data_CityInfo.foodInfoFoodTypesAvailable = 1;
        Data_CityInfo.foodInfoFoodSupplyMonths = 12;
    }
}

void city_resource_calculate_food_stocks_and_supply_wheat()
{
    calculate_available_food();
    if (scenario_property_rome_supplies_wheat()) {
        for (int i = 1; i < MAX_BUILDINGS; i++) {
            building *b = building_get(i);
            if (b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_MARKET) {
                b->data.market.inventory[INVENTORY_WHEAT] = 200;
            }
        }
    }
}

void city_resource_calculate_workshop_stocks()
{
    for (int i = 0; i < 6; i++) {
        city_data.resource.stored_in_workshops[i] = 0;
        city_data.resource.space_in_workshops[i] = 0;
    }
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE || !building_is_workshop(b->type)) {
            continue;
        }
        b->hasRoadAccess = 0;
        if (map_has_road_access(b->x, b->y, b->size, 0, 0)) {
            b->hasRoadAccess = 1;
            int room = 2 - b->loadsStored;
            if (room < 0) {
                room = 0;
            }
            int workshop_resource = b->subtype.workshopType;
            city_data.resource.space_in_workshops[workshop_resource] += room;
            city_data.resource.stored_in_workshops[workshop_resource] += b->loadsStored;
        }
    }
}

void city_resource_consume_food()
{
    calculate_available_food();
    Data_CityInfo.foodInfoFoodTypesEaten = 0;
    city_data.unused.unknown_00c0 = 0;
    int total_consumed = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->houseSize) {
            int num_types = model_get_house(b->subtype.houseLevel)->food_types;
            int amount_per_type = calc_adjust_with_percentage(b->housePopulation, 50);
            if (num_types > 1) {
                amount_per_type /= num_types;
            }
            b->data.house.numFoods = 0;
            if (scenario_property_rome_supplies_wheat()) {
                Data_CityInfo.foodInfoFoodTypesEaten = 1;
                Data_CityInfo.foodInfoFoodTypesAvailable = 1;
                b->data.house.inventory[INVENTORY_WHEAT] = amount_per_type;
                b->data.house.numFoods = 1;
            } else if (num_types > 0) {
                for (int t = INVENTORY_MIN_FOOD; t < INVENTORY_MAX_FOOD && b->data.house.numFoods < num_types; t++) {
                    if (b->data.house.inventory[t] >= amount_per_type) {
                        b->data.house.inventory[t] -= amount_per_type;
                        b->data.house.numFoods++;
                        total_consumed += amount_per_type;
                    } else if (b->data.house.inventory[t]) {
                        // has food but not enough
                        b->data.house.inventory[t] = 0;
                        b->data.house.numFoods++;
                        total_consumed += amount_per_type;
                    }
                    if (b->data.house.numFoods > Data_CityInfo.foodInfoFoodTypesEaten) {
                        Data_CityInfo.foodInfoFoodTypesEaten = b->data.house.numFoods;
                    }
                }
            }
        }
    }
    Data_CityInfo.foodInfoFoodConsumedLastMonth = total_consumed;
    Data_CityInfo.foodInfoFoodStoredLastMonth = Data_CityInfo.foodInfoFoodStoredSoFarThisMonth;
    Data_CityInfo.foodInfoFoodStoredSoFarThisMonth = 0;
}
