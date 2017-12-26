#include "Trader.h"

#include "Data/CityInfo.h"
#include "Data/Constants.h"

#include "building/building.h"
#include "building/count.h"
#include "building/storage.h"
#include "building/warehouse.h"
#include "city/message.h"
#include "core/calc.h"
#include "empire/city.h"
#include "empire/empire.h"
#include "empire/trade_prices.h"
#include "empire/trade_route.h"
#include "figure/figure.h"
#include "figure/type.h"
#include "map/road_access.h"
#include "scenario/map.h"

#include <string.h>

static int generateTrader(int cityId, empire_city *city)
{
	int maxTradersOnMap = 0;
	int numResources = 0;
	for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
		if (city->buys_resource[r] || city->sells_resource[r]) {
			++numResources;
			switch (trade_route_limit(city->route_id, r)) {
				case 15: maxTradersOnMap += 1; break;
				case 25: maxTradersOnMap += 2; break;
				case 40: maxTradersOnMap += 3; break;
			}
		}
	}
	if (numResources > 1) {
		if (maxTradersOnMap % numResources) {
			maxTradersOnMap = maxTradersOnMap / numResources + 1;
		} else {
			maxTradersOnMap = maxTradersOnMap / numResources;
		}
	}
	if (maxTradersOnMap <= 0) {
		return 0;
	}

	int index;
	if (maxTradersOnMap == 1) {
		if (!city->trader_figure_ids[0]) {
			index = 0;
		} else {
			return 0;
		}
	} else if (maxTradersOnMap == 2) {
		if (!city->trader_figure_ids[0]) {
			index = 0;
		} else if (!city->trader_figure_ids[1]) {
			index = 1;
		} else {
			return 0;
		}
	} else { // 3
		if (!city->trader_figure_ids[0]) {
			index = 0;
		} else if (!city->trader_figure_ids[1]) {
			index = 1;
		} else if (!city->trader_figure_ids[2]) {
			index = 2;
		} else {
			return 0;
		}
	}

	if (city->trader_entry_delay > 0) {
		city->trader_entry_delay--;
		return 0;
	}
	city->trader_entry_delay = city->is_sea_trade ? 30 : 4;

	if (city->is_sea_trade) {
		// generate ship
		if (Data_CityInfo.numWorkingDocks > 0 && scenario_map_has_river_entry() &&
			!Data_CityInfo.tradeSeaProblemDuration) {
            map_point river_entry = scenario_map_river_entry();
			figure *ship = figure_create(FIGURE_TRADE_SHIP, river_entry.x, river_entry.y, DIR_0_TOP);
			city->trader_figure_ids[index] = ship->id;
			ship->empireCityId = cityId;
			ship->actionState = FigureActionState_110_TradeShipCreated;
			ship->waitTicks = 10;
			return 1;
		}
	} else {
		// generate caravan and donkeys
		if (!Data_CityInfo.tradeLandProblemDuration) {
			// caravan head
			figure *caravan = figure_create(FIGURE_TRADE_CARAVAN,
				Data_CityInfo.entryPointX, Data_CityInfo.entryPointY, DIR_0_TOP);
			city->trader_figure_ids[index] = caravan->id;
			caravan->empireCityId = cityId;
			caravan->actionState = FigureActionState_100_TradeCaravanCreated;
			caravan->waitTicks = 10;
			// donkey 1
			figure *donkey1 = figure_create(FIGURE_TRADE_CARAVAN_DONKEY,
				Data_CityInfo.entryPointX, Data_CityInfo.entryPointY, DIR_0_TOP);
			donkey1->actionState = FigureActionState_100_TradeCaravanCreated;
			donkey1->inFrontFigureId = caravan->id;
			// donkey 2
			figure *donkey2 = figure_create(FIGURE_TRADE_CARAVAN_DONKEY,
				Data_CityInfo.entryPointX, Data_CityInfo.entryPointY, DIR_0_TOP);
			donkey2->actionState = FigureActionState_100_TradeCaravanCreated;
			donkey2->inFrontFigureId = donkey1->id;
			return 1;
		}
	}
	return 0;
}

static int canGenerateTraderForCity(int city_id, empire_city *city)
{
    if (city->is_sea_trade) {
        if (Data_CityInfo.numWorkingDocks <= 0) {
            // delay of 384 = 1 year
            city_message_post_with_message_delay(MESSAGE_CAT_NO_WORKING_DOCK, 1, MESSAGE_NO_WORKING_DOCK, 384);
            return 0;
        }
        if (!scenario_map_has_river_entry()) {
            return 0;
        }
        Data_CityInfo.tradeNumOpenSeaRoutes++;
    } else {
        Data_CityInfo.tradeNumOpenLandRoutes++;
    }
    return generateTrader(city_id, city);
}

void Trader_tick()
{
	Data_CityInfo.tradeNumOpenSeaRoutes = 0;
	Data_CityInfo.tradeNumOpenLandRoutes = 0;
	// Wine types
	Data_CityInfo.resourceWineTypesAvailable = building_count_industry_total(RESOURCE_WINE) > 0 ? 1 : 0;
    if (Data_CityInfo.resourceTradeStatus[RESOURCE_WINE] == TradeStatus_Import) {
        Data_CityInfo.resourceWineTypesAvailable += empire_city_count_wine_sources();
    }
	// Update trade problems
	if (Data_CityInfo.tradeLandProblemDuration > 0) {
		Data_CityInfo.tradeLandProblemDuration--;
	} else {
		Data_CityInfo.tradeLandProblemDuration = 0;
	}
	if (Data_CityInfo.tradeSeaProblemDuration > 0) {
		Data_CityInfo.tradeSeaProblemDuration--;
	} else {
		Data_CityInfo.tradeSeaProblemDuration = 0;
	}
	// Generate traders
	empire_city_foreach_open_until(canGenerateTraderForCity);
}

int Trader_getClosestWarehouseForTradeCaravan(const figure *f, int x, int y, int cityId, int distanceFromEntry, int *warehouseX, int *warehouseY)
{
	int exportable[RESOURCE_MAX];
	int importable[RESOURCE_MAX];
	exportable[RESOURCE_NONE] = 0;
	importable[RESOURCE_NONE] = 0;
	for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
		exportable[r] = empire_can_export_resource_to_city(cityId, r);
		if (f->traderAmountBought >= 8) {
			exportable[r] = 0;
		}
		if (cityId) {
			importable[r] = empire_can_import_resource_from_city(cityId, r);
		} else { // exclude own city (id=0), shouldn't happen, but still..
			importable[r] = 0;
		}
		if (f->loadsSoldOrCarrying >= 8) {
			importable[r] = 0;
		}
	}
	int numImportable = 0;
	for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
		if (importable[r]) {
			numImportable++;
		}
	}
	int minDistance = 10000;
	int minBuildingId = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
		if (!BuildingIsInUse(b) || b->type != BUILDING_WAREHOUSE) {
			continue;
		}
		if (!b->hasRoadAccess || b->distanceFromEntry <= 0) {
			continue;
		}
		const building_storage *s = building_storage_get(b->storage_id);
		int numImportsForWarehouse = 0;
		for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
			if (s->resource_state[r] != BUILDING_STORAGE_STATE_NOT_ACCEPTING && empire_can_import_resource_from_city(cityId, r)) {
				numImportsForWarehouse++;
			}
		}
		int distancePenalty = 32;
		building *space = b;
		for (int spaceCounter = 0; spaceCounter < 8; spaceCounter++) {
			space = building_next(space);
			if (space->id && exportable[space->subtype.warehouseResourceId]) {
				distancePenalty -= 4;
			}
			if (numImportable && numImportsForWarehouse && !s->empty_all) {
				for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
					Data_CityInfo.tradeNextImportResourceCaravan++;
					if (Data_CityInfo.tradeNextImportResourceCaravan > 15) {
						Data_CityInfo.tradeNextImportResourceCaravan = 1;
					}
					if (s->resource_state[Data_CityInfo.tradeNextImportResourceCaravan] != BUILDING_STORAGE_STATE_NOT_ACCEPTING) {
						break;
					}
				}
				if (s->resource_state[Data_CityInfo.tradeNextImportResourceCaravan] != BUILDING_STORAGE_STATE_NOT_ACCEPTING) {
					if (space->subtype.warehouseResourceId == RESOURCE_NONE) {
						distancePenalty -= 16;
					}
					if (space->id && importable[space->subtype.warehouseResourceId] && space->loadsStored < 4 &&
						space->subtype.warehouseResourceId == Data_CityInfo.tradeNextImportResourceCaravan) {
						distancePenalty -= 8;
					}
				}
			}
		}
		if (distancePenalty < 32) {
			int distance = calc_distance_with_penalty(b->x, b->y, x, y, distanceFromEntry, b->distanceFromEntry);
			distance += distancePenalty;
			if (distance < minDistance) {
				minDistance = distance;
				minBuildingId = i;
			}
		}
	}
	if (!minBuildingId) {
		return 0;
	}
	building *min = building_get(minBuildingId);
	if (min->hasRoadAccess == 1) {
		*warehouseX = min->x;
		*warehouseY = min->y;
	} else if (!map_has_road_access(min->x, min->y, 3, warehouseX, warehouseY)) {
		return 0;
	}
	return minBuildingId;
}
