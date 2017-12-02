#include "cityinfo.h"

#include "terrain.h"
#include "game/tutorial.h"

#include <data>
#include <scenario>

#include "empire/city.h"

#include "building/model.h"
#include "core/calc.h"

void CityInfo_Resource_calculateAvailableResources()
{
    for (int i = 0; i < RESOURCE_MAX; i++)
    {
        Data_CityInfo_Resource.availableResources[i] = 0;
        Data_CityInfo_Resource.availableFoods[i] = 0;
    }
    Data_CityInfo_Resource.numAvailableResources = 0;
    Data_CityInfo_Resource.numAvailableFoods = 0;

    for (int i = RESOURCE_MIN; i < RESOURCE_MAX; i++)
    {
        if (empire_can_produce_resource(i) || empire_can_import_resource(i) ||
                (i == RESOURCE_MEAT && scenario_building_allowed(BUILDING_WHARF)))
        {
            Data_CityInfo_Resource.availableResources[Data_CityInfo_Resource.numAvailableResources++] = i;
        }
    }
    for (int i = RESOURCE_MIN_FOOD; i < RESOURCE_MAX_FOOD; i++)
    {
        if (i == RESOURCE_OLIVES || i == RESOURCE_VINES)
        {
            continue;
        }
        if (empire_can_produce_resource(i) || empire_can_import_resource(i) ||
                (i == RESOURCE_MEAT && scenario_building_allowed(BUILDING_WHARF)))
        {
            Data_CityInfo_Resource.availableFoods[Data_CityInfo_Resource.numAvailableFoods++] = i;
        }
    }
}

void CityInfo_Resource_calculateFood()
{
    for (int i = 0; i < RESOURCE_MAX_FOOD; i++)
    {
        Data_CityInfo.resourceGranaryFoodStored[i] = 0;
    }
    Data_CityInfo.foodInfoFoodStoredInGranaries = 0;
    Data_CityInfo.foodInfoFoodTypesAvailable = 0;
    Data_CityInfo.foodInfoFoodSupplyMonths = 0;
    Data_CityInfo.foodInfoGranariesOperating = 0;
    Data_CityInfo.foodInfoGranariesUnderstaffed = 0;
    Data_CityInfo.foodInfoGranariesNotOperating = 0;
    Data_CityInfo.foodInfoGranariesNotOperatingWithFood = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++)
    {
        struct Data_Building *b = &Data_Buildings[i];
        if (!BuildingIsInUse(i) || b->type != BUILDING_GRANARY)
        {
            continue;
        }
        b->hasRoadAccess = 0;
        if (Terrain_hasRoadAccessGranary(b->x, b->y, 0, 0))
        {
            b->hasRoadAccess = 1;
            int pctWorkers = calc_percentage(
                                 b->numWorkers, model_get_building((building_type)b->type).laborers);
            if (pctWorkers < 100)
            {
                Data_CityInfo.foodInfoGranariesUnderstaffed++;
            }
            int amountStored = 0;
            for (int r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++)
            {
                amountStored += b->data.storage.resourceStored[r];
            }
            if (pctWorkers < 50)
            {
                Data_CityInfo.foodInfoGranariesNotOperating++;
                if (amountStored > 0)
                {
                    Data_CityInfo.foodInfoGranariesNotOperatingWithFood++;
                }
            }
            else
            {
                Data_CityInfo.foodInfoGranariesOperating++;
                for (int r = 0; r < RESOURCE_MAX_FOOD; r++)
                {
                    Data_CityInfo.resourceGranaryFoodStored[r] += b->data.storage.resourceStored[r];
                }
                if (amountStored > 400)
                {
                    Tutorial::on_filled_granary();
                }
            }
        }
    }
    for (int i = RESOURCE_MIN_FOOD; i < RESOURCE_MAX_FOOD; i++)
    {
        if (Data_CityInfo.resourceGranaryFoodStored[i])
        {
            Data_CityInfo.foodInfoFoodStoredInGranaries += Data_CityInfo.resourceGranaryFoodStored[i];
            Data_CityInfo.foodInfoFoodTypesAvailable++;
        }
    }
    Data_CityInfo.foodInfoFoodNeededPerMonth =
        calc_adjust_with_percentage(Data_CityInfo.population, 50);
    if (Data_CityInfo.foodInfoFoodNeededPerMonth > 0)
    {
        Data_CityInfo.foodInfoFoodSupplyMonths =
            Data_CityInfo.foodInfoFoodStoredInGranaries / Data_CityInfo.foodInfoFoodNeededPerMonth;
    }
    else
    {
        Data_CityInfo.foodInfoFoodSupplyMonths =
            Data_CityInfo.foodInfoFoodStoredInGranaries > 0 ? 1 : 0;
    }
    if (scenario_property_rome_supplies_wheat())
    {
        Data_CityInfo.foodInfoFoodTypesAvailable = 1;
        Data_CityInfo.foodInfoFoodSupplyMonths = 12;
    }
}

void CityInfo_Resource_calculateFoodAndSupplyRomeWheat()
{
    CityInfo_Resource_calculateFood();
    if (scenario_property_rome_supplies_wheat())
    {
        for (int i = 1; i < MAX_BUILDINGS; i++)
        {
            if (BuildingIsInUse(i) && Data_Buildings[i].type == BUILDING_MARKET)
            {
                Data_Buildings[i].data.market.inventory[INVENTORY_WHEAT] = 200;
            }
        }
    }
}

void CityInfo_Resource_housesConsumeFood()
{
    CityInfo_Resource_calculateFood();
    Data_CityInfo.foodInfoFoodTypesEaten = 0;
    Data_CityInfo.__unknown_00c0 = 0;
    int totalConsumed = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++)
    {
        struct Data_Building *b = &Data_Buildings[i];
        if (BuildingIsInUse(i) && b->houseSize)
        {
            int numTypes = model_get_house((house_level)b->subtype.houseLevel)->food_types;
            int amountPerType = calc_adjust_with_percentage(b->housePopulation, 50);
            if (numTypes > 1)
            {
                amountPerType /= numTypes;
            }
            b->data.house.numFoods = 0;
            if (scenario_property_rome_supplies_wheat())
            {
                Data_CityInfo.foodInfoFoodTypesEaten = 1;
                Data_CityInfo.foodInfoFoodTypesAvailable = 1;
                b->data.house.inventory[INVENTORY_WHEAT] = amountPerType;
                b->data.house.numFoods = 1;
            }
            else if (numTypes > 0)
            {
                for (int t = INVENTORY_MIN_FOOD; t < INVENTORY_MAX_FOOD && b->data.house.numFoods < numTypes; t++)
                {
                    if (b->data.house.inventory[t] >= amountPerType)
                    {
                        b->data.house.inventory[t] -= amountPerType;
                        b->data.house.numFoods++;
                        totalConsumed += amountPerType;
                    }
                    else if (b->data.house.inventory[t])
                    {
                        // has food but not enough
                        b->data.house.inventory[t] = 0;
                        b->data.house.numFoods++;
                        totalConsumed += amountPerType; // BUG?
                    }
                    if (b->data.house.numFoods > Data_CityInfo.foodInfoFoodTypesEaten)
                    {
                        Data_CityInfo.foodInfoFoodTypesEaten = b->data.house.numFoods;
                    }
                }
            }
        }
    }
    Data_CityInfo.foodInfoFoodConsumedLastMonth = totalConsumed;
    Data_CityInfo.foodInfoFoodStoredLastMonth = Data_CityInfo.foodInfoFoodStoredSoFarThisMonth;
    Data_CityInfo.foodInfoFoodStoredSoFarThisMonth = 0;
}
