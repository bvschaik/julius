#include "resource.h"

#include "Data/CityInfo.h"

#include "building/building.h"
#include "building/model.h"
#include "city/data_private.h"
#include "core/calc.h"
#include "empire/city.h"
#include "game/tutorial.h"
#include "map/road_access.h"
#include "scenario/building.h"
#include "scenario/property.h"

void city_resource_determine_available()
{
    for (int i = 0; i < RESOURCE_MAX; i++) {
        Data_CityInfo_Resource.availableResources[i] = 0;
        Data_CityInfo_Resource.availableFoods[i] = 0;
    }
    Data_CityInfo_Resource.numAvailableResources = 0;
    Data_CityInfo_Resource.numAvailableFoods = 0;

    for (int i = RESOURCE_MIN; i < RESOURCE_MAX; i++) {
        if (empire_can_produce_resource(i) || empire_can_import_resource(i) ||
            (i == RESOURCE_MEAT && scenario_building_allowed(BUILDING_WHARF))) {
            Data_CityInfo_Resource.availableResources[Data_CityInfo_Resource.numAvailableResources++] = i;
        }
    }
    for (int i = RESOURCE_MIN_FOOD; i < RESOURCE_MAX_FOOD; i++) {
        if (i == RESOURCE_OLIVES || i == RESOURCE_VINES) {
            continue;
        }
        if (empire_can_produce_resource(i) || empire_can_import_resource(i) ||
            (i == RESOURCE_MEAT && scenario_building_allowed(BUILDING_WHARF))) {
            Data_CityInfo_Resource.availableFoods[Data_CityInfo_Resource.numAvailableFoods++] = i;
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
