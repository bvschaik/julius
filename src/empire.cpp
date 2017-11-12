#include "empire.h"
#include "playermessage.h"

#include <data>
#include "building/count.h"
#include "core/buffer.h"
#include "core/calc.h"
#include "core/io.h"
#include "empire/city.h"
#include "empire/empire.h"
#include "empire/object.h"
#include "empire/trade_route.h"
#include "empire/type.h"
#include "game/time.h"
#include "graphics/image.h"

#include <string.h>

static struct
{
    short initialScrollX;
    short initialScrollY;
    short inUse;
    short __padding[13];
} Data_Empire_Index[40];

void Empire_load(int isCustomScenario, int empireId)
{
    const char *filename = isCustomScenario ? "c32.emp" : "c3.emp";
    io_read_file_part_into_buffer(filename, Data_Empire_Index, 1280, 0);
    int offset = 1280 + 12800 * Data_Scenario.empireId;
    char obj_buffer[12800];
    io_read_file_part_into_buffer(filename, obj_buffer, 12800, offset);
    buffer buf;
    buffer_init(&buf, obj_buffer, 12800);
    empire_object_load(&buf);
}

void Empire_initScroll()
{
    empire_init_scroll(
        Data_Empire_Index[Data_Scenario.empireId].initialScrollX,
        Data_Empire_Index[Data_Scenario.empireId].initialScrollY
    );
}

void Empire_initCities()
{
    empire_object_init_cities();
}

void Empire_initTradeAmountCodes()
{
    empire_object_init_trade_amounts();
}

int Empire_cityBuysResource(int objectId, int resource)
{
    const empire_object *object = empire_object_get(objectId);
    for (int i = 0; i < 8; i++)
    {
        if (object->city_buys_resource[i] == resource)
        {
            return 1;
        }
    }
    return 0;
}

int Empire_citySellsResource(int objectId, int resource)
{
    const empire_object *object = empire_object_get(objectId);
    for (int i = 0; i < 10; i++)
    {
        if (object->city_sells_resource[i] == resource)
        {
            return 1;
        }
    }
    return 0;
}

int Empire_canExportResourceToCity(int cityId, int resource)
{
    empire_city *city = empire_city_get(cityId);
    if (cityId && trade_route_limit_reached(city->route_id, resource))
    {
        // quota reached
        return 0;
    }
    if (Data_CityInfo.resourceStored[resource] <= Data_CityInfo.resourceTradeExportOver[resource])
    {
        // stocks too low
        return 0;
    }
    if (cityId == 0 || city->buys_resource[resource])
    {
        return Data_CityInfo.resourceTradeStatus[resource] == TradeStatus_Export;
    }
    else
    {
        return 0;
    }
}

int Empire_canImportResourceFromCity(int cityId, int resource)
{
    empire_city *city = empire_city_get(cityId);
    if (!city->sells_resource[resource])
    {
        return 0;
    }
    if (Data_CityInfo.resourceTradeStatus[resource] != TradeStatus_Import)
    {
        return 0;
    }
    if (trade_route_limit_reached(city->route_id, resource))
    {
        return 0;
    }

    int inStock = Data_CityInfo.resourceStored[resource];
    int maxInStock = 0;
    int finishedGood = Resource_None;
    switch (resource)
    {
    // food and finished materials
    case Resource_Wheat:
    case Resource_Vegetables:
    case Resource_Fruit:
    case Resource_Meat:
    case Resource_Pottery:
    case Resource_Furniture:
    case Resource_Oil:
    case Resource_Wine:
        if (Data_CityInfo.population < 2000)
        {
            maxInStock = 10;
        }
        else if (Data_CityInfo.population < 4000)
        {
            maxInStock = 20;
        }
        else if (Data_CityInfo.population < 6000)
        {
            maxInStock = 30;
        }
        else
        {
            maxInStock = 40;
        }
        break;

    case Resource_Marble:
    case Resource_Weapons:
        maxInStock = 10;
        break;

    case Resource_Clay:
        finishedGood = Resource_Pottery;
        break;
    case Resource_Timber:
        finishedGood = Resource_Furniture;
        break;
    case Resource_Olives:
        finishedGood = Resource_Oil;
        break;
    case Resource_Vines:
        finishedGood = Resource_Wine;
        break;
    case Resource_Iron:
        finishedGood = Resource_Weapons;
        break;
    }
    if (finishedGood)
    {
        maxInStock = 2 + 2 * building_count_industry_active(finishedGood);
    }
    return inStock < maxInStock ? 1 : 0;
}

void Empire_determineDistantBattleCity()
{
    Data_CityInfo.distantBattleCityId = empire_city_determine_distant_battle_city();
}

void Empire_handleExpandEvent()
{
    if (Data_Scenario.empireHasExpanded || Data_Scenario.empireExpansionYear <= 0)
    {
        return;
    }
    if (game_time_year() < Data_Scenario.empireExpansionYear + Data_Scenario.startYear)
    {
        return;
    }

    empire_city_expand_empire();

    Data_Scenario.empireHasExpanded = 1;
    PlayerMessage_post(1, Message_77_EmpireHasExpanded, 0, 0);
}
