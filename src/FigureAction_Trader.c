#include "FigureAction_private.h"

#include "Resource.h"
#include "Terrain.h"
#include "Trader.h"

#include "Data/CityInfo.h"

#include "building/storage.h"
#include "city/message.h"
#include "empire/city.h"
#include "empire/empire.h"
#include "empire/trade_prices.h"
#include "empire/trade_route.h"
#include "figure/route.h"
#include "figure/trader.h"
#include "figure/type.h"
#include "map/figure.h"
#include "scenario/map.h"

static void advanceTradeNextImportResourceCaravan()
{
	Data_CityInfo.tradeNextImportResourceCaravan++;
	if (Data_CityInfo.tradeNextImportResourceCaravan > 15) {
		Data_CityInfo.tradeNextImportResourceCaravan = 1;
	}
}

int FigureAction_TradeCaravan_canBuy(int traderId, int warehouseId, int cityId)
{
	if (Data_Buildings[warehouseId].type != BUILDING_WAREHOUSE) {
		return 0;
	}
	if (figure_get(traderId)->traderAmountBought >= 8) {
		return 0;
	}
	for (int i = 0; i < 8; i++) {
		warehouseId = Data_Buildings[warehouseId].nextPartBuildingId;
		if (warehouseId > 0 && Data_Buildings[warehouseId].loadsStored > 0 &&
			empire_can_export_resource_to_city(cityId, Data_Buildings[warehouseId].subtype.warehouseResourceId)) {
			return 1;
		}
	}
	return 0;
}

static int traderGetBuyResource(int warehouseId, int cityId)
{
	if (Data_Buildings[warehouseId].type != BUILDING_WAREHOUSE) {
		return RESOURCE_NONE;
	}
	for (int i = 0; i < 8; i++) {
		warehouseId = Data_Buildings[warehouseId].nextPartBuildingId;
		if (warehouseId <= 0) {
			continue;
		}
		int resource = Data_Buildings[warehouseId].subtype.warehouseResourceId;
		if (Data_Buildings[warehouseId].loadsStored > 0 && empire_can_export_resource_to_city(cityId, resource)) {
			// update stocks
			Data_CityInfo.resourceSpaceInWarehouses[resource]++;
			Data_CityInfo.resourceStored[resource]--;
			Data_Buildings[warehouseId].loadsStored--;
			if (Data_Buildings[warehouseId].loadsStored <= 0) {
				Data_Buildings[warehouseId].subtype.warehouseResourceId = RESOURCE_NONE;
			}
			// update finances
			int price = trade_price_sell(resource);
			Data_CityInfo.treasury += price;
			Data_CityInfo.financeExportsThisYear += price;
			if (Data_CityInfo.godBlessingNeptuneDoubleTrade) {
				Data_CityInfo.treasury += price;
				Data_CityInfo.financeExportsThisYear += price;
			}
			// update graphics
			Resource_setWarehouseSpaceGraphic(warehouseId, resource);
			return resource;
		}
	}
	return 0;
}

int FigureAction_TradeCaravan_canSell(int traderId, int warehouseId, int cityId)
{
	if (Data_Buildings[warehouseId].type != BUILDING_WAREHOUSE) {
		return 0;
	}
	if (figure_get(traderId)->loadsSoldOrCarrying >= 8) {
		return 0;
	}
	const building_storage *s = building_storage_get(Data_Buildings[warehouseId].storage_id);
	if (s->empty_all) {
		return 0;
	}
	int numImportable = 0;
	for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
		if (s->resource_state[r] != BUILDING_STORAGE_STATE_NOT_ACCEPTING) {
			if (empire_can_import_resource_from_city(cityId, r)) {
				numImportable++;
			}
		}
	}
	if (numImportable <= 0) {
		return 0;
	}
	int canImport = 0;
	if (s->resource_state[Data_CityInfo.tradeNextImportResourceCaravan] != BUILDING_STORAGE_STATE_NOT_ACCEPTING &&
		empire_can_import_resource_from_city(cityId, Data_CityInfo.tradeNextImportResourceCaravan)) {
		canImport = 1;
	} else {
		for (int i = RESOURCE_MIN; i < RESOURCE_MAX; i++) {
			advanceTradeNextImportResourceCaravan();
			if (s->resource_state[Data_CityInfo.tradeNextImportResourceCaravan] != BUILDING_STORAGE_STATE_NOT_ACCEPTING &&
					empire_can_import_resource_from_city(cityId, Data_CityInfo.tradeNextImportResourceCaravan)) {
				canImport = 1;
				break;
			}
		}
	}
	if (canImport) {
		// at least one resource can be imported and accepted by this warehouse
		// check if warehouse can store any importable goods
		int spaceId = warehouseId;
		for (int spaceCounter = 0; spaceCounter < 8; spaceCounter++) {
			spaceId = Data_Buildings[spaceId].nextPartBuildingId;
			if (spaceId > 0 && Data_Buildings[spaceId].loadsStored < 4) {
				if (!Data_Buildings[spaceId].loadsStored) {
					// empty space
					return 1;
				}
				if (empire_can_import_resource_from_city(cityId, Data_Buildings[spaceId].subtype.warehouseResourceId)) {
					return 1;
				}
			}
		}
	}
	return 0;
}

static int traderGetSellResource(int warehouseId, int cityId)
{
	if (Data_Buildings[warehouseId].type != BUILDING_WAREHOUSE) {
		return 0;
	}
	int imp = 1;
	while (imp < 16 && !empire_can_import_resource_from_city(cityId, Data_CityInfo.tradeNextImportResourceCaravan)) {
		imp++;
		advanceTradeNextImportResourceCaravan();
	}
	if (imp >= 16) {
		return 0;
	}
	int resourceToImport = Data_CityInfo.tradeNextImportResourceCaravan;
	// add to existing bay with room
	int spaceId = warehouseId;
	for (int i = 0; i < 8; i++) {
		spaceId = Data_Buildings[spaceId].nextPartBuildingId;
		struct Data_Building *b = &Data_Buildings[spaceId];
		if (spaceId > 0 && b->loadsStored > 0 && b->loadsStored < 4 &&
			b->subtype.warehouseResourceId == resourceToImport) {
			Resource_addImportedResourceToWarehouseSpace(spaceId, resourceToImport);
			advanceTradeNextImportResourceCaravan();
			return resourceToImport;
		}
	}
	// add to empty bay
	spaceId = warehouseId;
	for (int i = 0; i < 8; i++) {
		spaceId = Data_Buildings[spaceId].nextPartBuildingId;
		struct Data_Building *b = &Data_Buildings[spaceId];
		if (spaceId > 0 && !b->loadsStored) {
			Resource_addImportedResourceToWarehouseSpace(spaceId, resourceToImport);
			advanceTradeNextImportResourceCaravan();
			return resourceToImport;
		}
	}
	// find another importable resource that can be added to this warehouse
	for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
		Data_CityInfo.tradeNextImportResourceCaravanBackup++;
		if (Data_CityInfo.tradeNextImportResourceCaravanBackup > 15) {
			Data_CityInfo.tradeNextImportResourceCaravanBackup = 1;
		}
		resourceToImport = Data_CityInfo.tradeNextImportResourceCaravanBackup;
		if (empire_can_import_resource_from_city(cityId, resourceToImport)) {
			spaceId = warehouseId;
			for (int i = 0; i < 8; i++) {
				spaceId = Data_Buildings[spaceId].nextPartBuildingId;
				struct Data_Building *b = &Data_Buildings[spaceId];
				if (spaceId > 0 && b->loadsStored < 4 && b->subtype.warehouseResourceId == resourceToImport) {
					Resource_addImportedResourceToWarehouseSpace(spaceId, resourceToImport);
					return resourceToImport;
				}
			}
		}
	}
	return 0;
}

static void goToNextWarehouse(figure *f, int xSrc, int ySrc, int distToEntry)
{
	int xDst, yDst;
	int warehouseId = Trader_getClosestWarehouseForTradeCaravan(
		f, xSrc, ySrc, f->empireCityId, distToEntry, &xDst, &yDst);
	if (warehouseId) {
		f->destinationBuildingId = warehouseId;
		f->actionState = FigureActionState_101_TradeCaravanArriving;
		f->destinationX = xDst;
		f->destinationY = yDst;
	} else {
		f->actionState = FigureActionState_103_TradeCaravanLeaving;
		f->destinationX = Data_CityInfo.exitPointX;
		f->destinationY = Data_CityInfo.exitPointY;
	}
}

void FigureAction_tradeCaravan(figure *f)
{
	f->isGhost = 0;
	f->terrainUsage = FigureTerrainUsage_PreferRoads;
	FigureActionIncreaseGraphicOffset(f, 12);
	f->cartGraphicId = 0;
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(f);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(f);
			break;
		case FigureActionState_100_TradeCaravanCreated:
			f->isGhost = 1;
			f->waitTicks++;
			if (f->waitTicks > 20) {
				f->waitTicks = 0;
				int xBase, yBase;
				if (Data_CityInfo.buildingTradeCenterBuildingId) {
					xBase = Data_Buildings[Data_CityInfo.buildingTradeCenterBuildingId].x;
					yBase = Data_Buildings[Data_CityInfo.buildingTradeCenterBuildingId].y;
				} else {
					xBase = f->x;
					yBase = f->y;
				}
				goToNextWarehouse(f, xBase, yBase, 0);
			}
			f->graphicOffset = 0;
			break;
		case FigureActionState_101_TradeCaravanArriving:
			FigureMovement_walkTicks(f, 1);
			switch (f->direction) {
				case DirFigure_8_AtDestination:
					f->actionState = FigureActionState_102_TradeCaravanTrading;
					break;
				case DirFigure_9_Reroute:
					figure_route_remove(f);
					break;
				case DirFigure_10_Lost:
					f->state = FigureState_Dead;
					f->isGhost = 1;
					break;
			}
			if (!BuildingIsInUse(f->destinationBuildingId)) {
				f->state = FigureState_Dead;
			}
			break;
		case FigureActionState_102_TradeCaravanTrading:
			f->waitTicks++;
			if (f->waitTicks > 10) {
				f->waitTicks = 0;
				int moveOn = 0;
				if (FigureAction_TradeCaravan_canBuy(f->id, f->destinationBuildingId, f->empireCityId)) {
					int resource = traderGetBuyResource(f->destinationBuildingId, f->empireCityId);
					if (resource) {
						trade_route_increase_traded(empire_city_get_route_id(f->empireCityId), resource);
						trader_record_bought_resource(f->traderId, resource);
						f->traderAmountBought++;
					} else {
						moveOn++;
					}
				} else {
					moveOn++;
				}
				if (FigureAction_TradeCaravan_canSell(f->id, f->destinationBuildingId, f->empireCityId)) {
					int resource = traderGetSellResource(f->destinationBuildingId, f->empireCityId);
					if (resource) {
						trade_route_increase_traded(empire_city_get_route_id(f->empireCityId), resource);
						trader_record_sold_resource(f->traderId, resource);
						f->loadsSoldOrCarrying++;
					} else {
						moveOn++;
					}
				} else {
					moveOn++;
				}
				if (moveOn == 2) {
					goToNextWarehouse(f, f->x, f->y, -1);
				}
			}
			f->graphicOffset = 0;
			break;
		case FigureActionState_103_TradeCaravanLeaving:
			FigureMovement_walkTicks(f, 1);
			switch (f->direction) {
				case DirFigure_8_AtDestination:
					f->actionState = FigureActionState_100_TradeCaravanCreated;
					f->state = FigureState_Dead;
					break;
				case DirFigure_9_Reroute:
					figure_route_remove(f);
					break;
				case DirFigure_10_Lost:
					f->state = FigureState_Dead;
					break;
			}
			break;
	}
	int dir = f->direction < 8 ? f->direction : f->previousTileDirection;
	FigureActionNormalizeDirection(dir);
	f->graphicId = image_group(GROUP_FIGURE_TRADE_CARAVAN) + dir + 8 * f->graphicOffset;
}

void FigureAction_tradeCaravanDonkey(figure *f)
{
	f->isGhost = 0;
	f->terrainUsage = FigureTerrainUsage_PreferRoads;
	FigureActionIncreaseGraphicOffset(f, 12);
	f->cartGraphicId = 0;

	figure *leader = figure_get(f->inFrontFigureId);
	if (f->inFrontFigureId <= 0) {
		f->state = FigureState_Dead;
	} else {
		if (leader->actionState == FigureActionState_149_Corpse) {
			f->state = FigureState_Dead;
		} else if (leader->state != FigureState_Alive) {
			f->state = FigureState_Dead;
		} else if (leader->type != FIGURE_TRADE_CARAVAN && leader->type != FIGURE_TRADE_CARAVAN_DONKEY) {
			f->state = FigureState_Dead;
		} else {
			FigureMovement_followTicks(f, 1);
		}
	}

	if (leader->isGhost) {
		f->isGhost = 1;
	}
	int dir = f->direction < 8 ? f->direction : f->previousTileDirection;
	FigureActionNormalizeDirection(dir);
	f->graphicId = image_group(GROUP_FIGURE_TRADE_CARAVAN) + dir + 8 * f->graphicOffset;
}

void FigureAction_nativeTrader(figure *f)
{
	f->isGhost = 0;
	f->terrainUsage = FigureTerrainUsage_Any;
	FigureActionIncreaseGraphicOffset(f, 12);
	f->cartGraphicId = 0;
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(f);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(f);
			break;
		case FigureActionState_160_NativeTraderGoingToWarehouse:
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DirFigure_8_AtDestination) {
				f->actionState = FigureActionState_163_NativeTraderAtWarehouse;
			} else if (f->direction == DirFigure_9_Reroute) {
				figure_route_remove(f);
			} else if (f->direction == DirFigure_10_Lost) {
				f->state = FigureState_Dead;
				f->isGhost = 1;
			}
			if (!BuildingIsInUse(f->destinationBuildingId)) {
				f->state = FigureState_Dead;
			}
			break;
		case FigureActionState_161_NativeTraderReturning:
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DirFigure_8_AtDestination || f->direction == DirFigure_10_Lost) {
				f->state = FigureState_Dead;
			} else if (f->direction == DirFigure_9_Reroute) {
				figure_route_remove(f);
			}
			break;
		case FigureActionState_162_NativeTraderCreated:
			f->isGhost = 1;
			f->waitTicks++;
			if (f->waitTicks > 10) {
				f->waitTicks = 0;
				int xTile, yTile;
				int buildingId = Trader_getClosestWarehouseForTradeCaravan(f, f->x, f->y, 0, -1, &xTile, &yTile);
				if (buildingId) {
					f->actionState = FigureActionState_160_NativeTraderGoingToWarehouse;
					f->destinationBuildingId = buildingId;
					f->destinationX = xTile;
					f->destinationY = yTile;
				} else {
					f->state = FigureState_Dead;
				}
			}
			f->graphicOffset = 0;
			break;
		case FigureActionState_163_NativeTraderAtWarehouse:
			f->waitTicks++;
			if (f->waitTicks > 10) {
				f->waitTicks = 0;
				if (FigureAction_TradeCaravan_canBuy(f->id, f->destinationBuildingId, 0)) {
					int resource = traderGetBuyResource(f->destinationBuildingId, 0);
					trader_record_bought_resource(f->traderId, resource);
					f->traderAmountBought += 3;
				} else {
					int xTile, yTile;
					int buildingId = Trader_getClosestWarehouseForTradeCaravan(f, f->x, f->y, 0, -1, &xTile, &yTile);
					if (buildingId) {
						f->actionState = FigureActionState_160_NativeTraderGoingToWarehouse;
						f->destinationBuildingId = buildingId;
						f->destinationX = xTile;
						f->destinationY = yTile;
					} else {
						f->actionState = FigureActionState_161_NativeTraderReturning;
						f->destinationX = f->sourceX;
						f->destinationY = f->sourceY;
					}
				}
			}
			f->graphicOffset = 0;
			break;
	}
	int dir = (f->direction < 8) ? f->direction : f->previousTileDirection;
	FigureActionNormalizeDirection(dir);
	
	if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = image_group(GROUP_FIGURE_CARTPUSHER) +
			96 + FigureActionCorpseGraphicOffset(f);
		f->cartGraphicId = 0;
	} else {
		f->graphicId = image_group(GROUP_FIGURE_CARTPUSHER) +
			dir + 8 * f->graphicOffset;
	}
	f->cartGraphicId = image_group(GROUP_FIGURE_MIGRANT_CART) +
		8 + 8 * f->resourceId; // BUGFIX should be within else statement?
	if (f->cartGraphicId) {
		f->cartGraphicId += dir;
		FigureAction_Common_setCartOffset(f, dir);
	}
}

static int tradeShipLostQueue(const figure *f)
{
	int buildingId = f->destinationBuildingId;
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (BuildingIsInUse(buildingId) && b->type == BUILDING_DOCK &&
		b->numWorkers > 0 && b->data.other.boatFigureId == f->id) {
		return 0;
	}
	return 1;
}

static int tradeShipDoneTrading(figure *f)
{
	int buildingId = f->destinationBuildingId;
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (BuildingIsInUse(buildingId) && b->type == BUILDING_DOCK && b->numWorkers > 0) {
		for (int i = 0; i < 3; i++) {
            if (b->data.other.dockFigureIds[i]) {
                figure *docker = figure_get(b->data.other.dockFigureIds[i]);
                if (docker->state == FigureState_Alive && docker->actionState != FigureActionState_132_DockerIdling) {
                    return 0;
                }
            }
		}
		f->tradeShipFailedDockAttempts++;
		if (f->tradeShipFailedDockAttempts >= 10) {
			f->tradeShipFailedDockAttempts = 11;
			return 1;
		}
		return 0;
	}
	return 1;
}

void FigureAction_tradeShip(figure *f)
{
	f->isGhost = 0;
	f->isBoat = 1;
	FigureActionIncreaseGraphicOffset(f, 12);
	f->cartGraphicId = 0;
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(f);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(f);
			break;
		case FigureActionState_110_TradeShipCreated:
			f->loadsSoldOrCarrying = 12;
			f->traderAmountBought = 0;
			f->isGhost = 1;
			f->waitTicks++;
			if (f->waitTicks > 20) {
				f->waitTicks = 0;
				int xTile, yTile;
				int dockId = Terrain_Water_getFreeDockDestination(f->id, &xTile, &yTile);
				if (dockId) {
					f->destinationBuildingId = dockId;
					f->actionState = FigureActionState_111_TradeShipGoingToDock;
					f->destinationX = xTile;
					f->destinationY = yTile;
				} else if (Terrain_Water_getQueueDockDestination(&xTile, &yTile)) {
					f->actionState = FigureActionState_113_TradeShipGoingToDockQueue;
					f->destinationX = xTile;
					f->destinationY = yTile;
				} else {
					f->state = FigureState_Dead;
				}
			}
			f->graphicOffset = 0;
			break;
		case FigureActionState_111_TradeShipGoingToDock:
			FigureMovement_walkTicks(f, 1);
			f->heightAdjustedTicks = 0;
			if (f->direction == DirFigure_8_AtDestination) {
				f->actionState = FigureActionState_112_TradeShipMoored;
			} else if (f->direction == DirFigure_9_Reroute) {
				figure_route_remove(f);
			} else if (f->direction == DirFigure_10_Lost) {
				f->state = FigureState_Dead;
				if (!game.messages.get_category_count(MESSAGE_CAT_BLOCKED_DOCK)) {
					game.messages.post(1, MESSAGE_NAVIGATION_IMPOSSIBLE, 0, 0);
					game.messages.increase_category_count(MESSAGE_CAT_BLOCKED_DOCK);
				}
			}
			if (!BuildingIsInUse(f->destinationBuildingId)) {
				f->actionState = FigureActionState_115_TradeShipLeaving;
				f->waitTicks = 0;
                map_point river_exit = scenario_map_river_exit();
				f->destinationX = river_exit.x;
				f->destinationY = river_exit.y;
			}
			break;
		case FigureActionState_112_TradeShipMoored:
			if (tradeShipLostQueue(f)) {
				f->tradeShipFailedDockAttempts = 0;
				f->actionState = FigureActionState_115_TradeShipLeaving;
				f->waitTicks = 0;
                map_point river_entry = scenario_map_river_entry();
				f->destinationX = river_entry.x;
				f->destinationY = river_entry.y;
			} else if (tradeShipDoneTrading(f)) {
				f->tradeShipFailedDockAttempts = 0;
				f->actionState = FigureActionState_115_TradeShipLeaving;
				f->waitTicks = 0;
                map_point river_entry = scenario_map_river_entry();
				f->destinationX = river_entry.x;
				f->destinationY = river_entry.y;
				Data_Buildings[f->destinationBuildingId].data.other.dockQueuedDockerId = 0;
				Data_Buildings[f->destinationBuildingId].data.other.dockNumShips = 0;
			}
			switch (Data_Buildings[f->destinationBuildingId].data.other.dockOrientation) {
				case 0: f->direction = Dir_2_Right; break;
				case 1: f->direction = Dir_4_Bottom; break;
				case 2: f->direction = Dir_6_Left; break;
				default:f->direction = Dir_0_Top; break;
			}
			f->graphicOffset = 0;
			game.messages.reset_category_count(MESSAGE_CAT_BLOCKED_DOCK);
			break;
		case FigureActionState_113_TradeShipGoingToDockQueue:
			FigureMovement_walkTicks(f, 1);
			f->heightAdjustedTicks = 0;
			if (f->direction == DirFigure_8_AtDestination) {
				f->actionState = FigureActionState_114_TradeShipAnchored;
			} else if (f->direction == DirFigure_9_Reroute) {
				figure_route_remove(f);
			} else if (f->direction == DirFigure_10_Lost) {
				f->state = FigureState_Dead;
			}
			break;
		case FigureActionState_114_TradeShipAnchored:
			f->waitTicks++;
			if (f->waitTicks > 40) {
				int xTile, yTile;
				int dockId = Terrain_Water_getFreeDockDestination(f->id, &xTile, &yTile);
				if (dockId) {
					f->destinationBuildingId = dockId;
					f->actionState = FigureActionState_111_TradeShipGoingToDock;
					f->destinationX = xTile;
					f->destinationY = yTile;
				} else if (map_figure_at(f->gridOffset) != f->id &&
					Terrain_Water_getQueueDockDestination(&xTile, &yTile)) {
					f->actionState = FigureActionState_113_TradeShipGoingToDockQueue;
					f->destinationX = xTile;
					f->destinationY = yTile;
				}
				f->waitTicks = 0;
			}
			f->graphicOffset = 0;
			break;
		case FigureActionState_115_TradeShipLeaving:
			FigureMovement_walkTicks(f, 1);
			f->heightAdjustedTicks = 0;
			if (f->direction == DirFigure_8_AtDestination) {
				f->actionState = FigureActionState_110_TradeShipCreated;
				f->state = FigureState_Dead;
			} else if (f->direction == DirFigure_9_Reroute) {
				figure_route_remove(f);
			} else if (f->direction == DirFigure_10_Lost) {
				f->state = FigureState_Dead;
			}
			break;
	}
	int dir = f->direction < 8 ? f->direction : f->previousTileDirection;
	FigureActionNormalizeDirection(dir);
	f->graphicId = image_group(GROUP_FIGURE_SHIP) + dir;
}

int FigureAction_TradeShip_isBuyingOrSelling(int figureId)
{
	int buildingId = figure_get(figureId)->destinationBuildingId;
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (!BuildingIsInUse(buildingId) || b->type != BUILDING_DOCK) {
		return TradeShipState_Buying;
	}
	for (int i = 0; i < 3; i++) {
		figure *f = figure_get(b->data.other.dockFigureIds[i]);
		if (!b->data.other.dockFigureIds[i] || f->state != FigureState_Alive) {
			continue;
		}
		switch (f->actionState) {
			case FigureActionState_133_DockerImportQueue:
			case FigureActionState_135_DockerImportGoingToWarehouse:
			case FigureActionState_138_DockerImportReturning:
			case FigureActionState_139_DockerImportAtWarehouse:
				return TradeShipState_Buying;
			case FigureActionState_134_DockerExportQueue:
			case FigureActionState_136_DockerExportGoingToWarehouse:
			case FigureActionState_137_DockerExportReturning:
			case FigureActionState_140_DockerExportAtWarehouse:
				return TradeShipState_Selling;
		}
	}
	return TradeShipState_None;
}
