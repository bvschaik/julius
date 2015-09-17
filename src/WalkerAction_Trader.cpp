#include "WalkerAction_private.h"

#include "Empire.h"
#include "PlayerMessage.h"
#include "Resource.h"
#include "Terrain.h"
#include "Trader.h"
#include "Walker.h"

#include "Data/CityInfo.h"
#include "Data/Empire.h"
#include "Data/Grid.h"
#include "Data/Message.h"
#include "Data/Scenario.h"
#include "Data/Trade.h"

static void advanceTradeNextImportResourceCaravan()
{
	Data_CityInfo.tradeNextImportResourceCaravan++;
	if (Data_CityInfo.tradeNextImportResourceCaravan > 15) {
		Data_CityInfo.tradeNextImportResourceCaravan = 1;
	}
}

int WalkerAction_TradeCaravan_canBuy(int traderId, int warehouseId, int cityId)
{
	if (Data_Buildings[warehouseId].type != Building_Warehouse) {
		return 0;
	}
	if (Data_Walkers[traderId].traderAmountBought >= 8) {
		return 0;
	}
	for (int i = 0; i < 8; i++) {
		warehouseId = Data_Buildings[warehouseId].nextPartBuildingId;
		if (warehouseId > 0 && Data_Buildings[warehouseId].loadsStored > 0 &&
			Empire_canExportResourceToCity(cityId, Data_Buildings[warehouseId].subtype.warehouseResourceId)) {
			return 1;
		}
	}
	return 0;
}

static int traderGetBuyResource(int warehouseId, int cityId)
{
	if (Data_Buildings[warehouseId].type != Building_Warehouse) {
		return Resource_None;
	}
	for (int i = 0; i < 8; i++) {
		warehouseId = Data_Buildings[warehouseId].nextPartBuildingId;
		if (warehouseId <= 0) {
			continue;
		}
		int resource = Data_Buildings[warehouseId].subtype.warehouseResourceId;
		if (Data_Buildings[warehouseId].loadsStored > 0 && Empire_canExportResourceToCity(cityId, resource)) {
			// update stocks
			Data_CityInfo.resourceSpaceInWarehouses[resource]++;
			Data_CityInfo.resourceStored[resource]--;
			Data_Buildings[warehouseId].loadsStored--;
			if (Data_Buildings[warehouseId].loadsStored <= 0) {
				Data_Buildings[warehouseId].subtype.warehouseResourceId = Resource_None;
			}
			// update finances
			Data_CityInfo.treasury += Data_TradePrices[resource].sell;
			Data_CityInfo.financeExportsThisYear += Data_TradePrices[resource].sell;
			if (Data_CityInfo.godBlessingNeptuneDoubleTrade) {
				Data_CityInfo.treasury += Data_TradePrices[resource].sell;
				Data_CityInfo.financeExportsThisYear += Data_TradePrices[resource].sell;
			}
			// update graphics
			Resource_setWarehouseSpaceGraphic(warehouseId, resource);
			return resource;
		}
	}
	return 0;
}

int WalkerAction_TradeCaravan_canSell(int traderId, int warehouseId, int cityId)
{
	if (Data_Buildings[warehouseId].type != Building_Warehouse) {
		return 0;
	}
	if (Data_Walkers[traderId].loadsSoldOrCarrying >= 8) {
		return 0;
	}
	struct Data_Building_Storage *st = &Data_Building_Storages[Data_Buildings[warehouseId].storageId];
	if (st->emptyAll) {
		return 0;
	}
	int numImportable = 0;
	for (int r = 1; r < 16; r++) {
		if (st->resourceState[r] != BuildingStorageState_NotAccepting) {
			if (Empire_canImportResourceFromCity(cityId, r)) {
				numImportable++;
			}
		}
	}
	if (numImportable <= 0) {
		return 0;
	}
	int canImport = 0;
	if (st->resourceState[Data_CityInfo.tradeNextImportResourceCaravan] != BuildingStorageState_NotAccepting &&
		Empire_canImportResourceFromCity(cityId, Data_CityInfo.tradeNextImportResourceCaravan)) {
		canImport = 1;
	}
	if (!canImport) {
		for (int i = 1; i < 16; i++) {
			advanceTradeNextImportResourceCaravan();
			if (st->resourceState[Data_CityInfo.tradeNextImportResourceCaravan] != BuildingStorageState_NotAccepting &&
					Empire_canImportResourceFromCity(cityId, Data_CityInfo.tradeNextImportResourceCaravan)) {
				canImport = 1;
				break;
			}
		}
	}
	if (canImport) {
		// at least one resource can be imported and accepted by this warehouse
		// check if warehouse can store any importable goods
		int spaceId = warehouseId;
		for (int i = 0; i < 8; i++) {
			spaceId = Data_Buildings[spaceId].nextPartBuildingId;
			if (spaceId > 0 && Data_Buildings[spaceId].loadsStored < 4) {
				if (!Data_Buildings[spaceId].loadsStored) {
					// empty space
					return 1;
				}
				if (Empire_canImportResourceFromCity(cityId, Data_Buildings[spaceId].subtype.warehouseResourceId)) {
					return 1;
				}
			}
		}
	}
	return 0;
}

static int traderGetSellResource(int traderId, int warehouseId, int cityId)
{
	if (Data_Buildings[warehouseId].type != Building_Warehouse) {
		return 0;
	}
	int imp = 1;
	while (imp < 16 && !Empire_canImportResourceFromCity(cityId, Data_CityInfo.tradeNextImportResourceCaravan)) {
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
	for (int r = 1; r < 16; r++) {
		Data_CityInfo.tradeNextImportResourceCaravanBackup++;
		if (Data_CityInfo.tradeNextImportResourceCaravanBackup > 15) {
			Data_CityInfo.tradeNextImportResourceCaravanBackup = 1;
		}
		resourceToImport = Data_CityInfo.tradeNextImportResourceCaravanBackup;
		if (Empire_canImportResourceFromCity(cityId, resourceToImport)) {
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

static void goToNextWarehouse(int walkerId, struct Data_Walker *w, int xSrc, int ySrc, int distToEntry)
{
	int xDst, yDst;
	int warehouseId = Trader_getClosestWarehouseForTradeCaravan(
		walkerId, xSrc, ySrc, w->empireCityId, distToEntry, &xDst, &yDst);
	if (warehouseId) {
		w->destinationBuildingId = warehouseId;
		w->actionState = WalkerActionState_101_TradeCaravanArriving;
		w->destinationX = xDst;
		w->destinationY = yDst;
	} else {
		w->actionState = WalkerActionState_103_TradeCaravanLeaving;
		w->destinationX = Data_CityInfo.exitPointX;
		w->destinationY = Data_CityInfo.exitPointY;
	}
}

void WalkerAction_tradeCaravan(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->isGhost = 0;
	w->terrainUsage = 3;
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->cartGraphicId = 0;
	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_100_TradeCaravanCreated:
			w->isGhost = 1;
			w->waitTicks++;
			if (w->waitTicks > 20) {
				w->waitTicks = 0;
				int xBase, yBase;
				if (Data_CityInfo.buildingTradeCenterBuildingId) {
					xBase = Data_Buildings[Data_CityInfo.buildingTradeCenterBuildingId].x;
					yBase = Data_Buildings[Data_CityInfo.buildingTradeCenterBuildingId].y;
				} else {
					xBase = w->x;
					yBase = w->y;
				}
				goToNextWarehouse(walkerId, w, xBase, yBase, 0);
			}
			w->graphicOffset = 0;
			break;
		case WalkerActionState_101_TradeCaravanArriving:
			WalkerMovement_walkTicks(walkerId, 1);
			switch (w->direction) {
				case 8:
					w->actionState = WalkerActionState_102_TradeCaravanTrading;
					break;
				case 9:
					WalkerRoute_remove(walkerId);
					break;
				case 10:
					w->state = WalkerState_Dead;
					w->isGhost = 1;
					break;
			}
			if (!BuildingIsInUse(w->destinationBuildingId)) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_102_TradeCaravanTrading:
			w->waitTicks++;
			if (w->waitTicks > 10) {
				w->waitTicks = 0;
				int moveOn = 0;
				if (WalkerAction_TradeCaravan_canBuy(walkerId, w->destinationBuildingId, w->empireCityId)) {
					int resource = traderGetBuyResource(w->destinationBuildingId, w->empireCityId);
					if (resource) {
						Data_Empire_Trade.tradedThisYear[Data_Empire_Cities[w->empireCityId].routeId][resource]++;
						Trader_buyResource(walkerId, resource);
						w->traderAmountBought++;
					} else {
						moveOn++;
					}
				} else {
					moveOn++;
				}
				if (WalkerAction_TradeCaravan_canSell(walkerId, w->destinationBuildingId, w->empireCityId)) {
					int resource = traderGetSellResource(walkerId, w->destinationBuildingId, w->empireCityId);
					if (resource) {
						Data_Empire_Trade.tradedThisYear[Data_Empire_Cities[w->empireCityId].routeId][resource]++;
						Trader_sellResource(walkerId, resource);
						w->loadsSoldOrCarrying++;
					} else {
						moveOn++;
					}
				} else {
					moveOn++;
				}
				if (moveOn == 2) {
					goToNextWarehouse(walkerId, w, w->x, w->y, -1);
				}
			}
			w->graphicOffset = 0;
			break;
		case WalkerActionState_103_TradeCaravanLeaving:
			WalkerMovement_walkTicks(walkerId, 1);
			switch (w->direction) {
				case 8:
					w->actionState = WalkerActionState_100_TradeCaravanCreated;
					w->state = WalkerState_Dead;
					break;
				case 9:
					WalkerRoute_remove(walkerId);
					break;
				case 10:
					w->state = WalkerState_Dead;
					break;
			}
			break;
	}
	int dir = w->direction < 8 ? w->direction : w->previousTileDirection;
	WalkerActionNormalizeDirection(dir);
	w->graphicId = GraphicId(ID_Graphic_Walker_TradeCaravan) +
		dir + 8 * w->graphicOffset;
}

void WalkerAction_tradeCaravanDonkey(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->isGhost = 0;
	w->terrainUsage = 3;
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->cartGraphicId = 0;

	struct Data_Walker *leader = &Data_Walkers[w->inFrontWalkerId];
	if (w->inFrontWalkerId <= 0) {
		w->state = WalkerState_Dead;
	} else {
		if (leader->actionState == WalkerActionState_149_Corpse) {
			w->state = WalkerState_Dead;
		} else if (leader->state != WalkerState_Alive) {
			w->state = WalkerState_Dead;
		} else if (leader->type != Walker_TradeCaravan && leader->type != Walker_TradeCaravanDonkey) {
			w->state = WalkerState_Dead;
		} else {
			WalkerMovement_followTicks(walkerId, w->inFrontWalkerId, 1);
		}
	}

	if (leader->isGhost) {
		w->isGhost = 1;
	}
	int dir = w->direction < 8 ? w->direction : w->previousTileDirection;
	WalkerActionNormalizeDirection(dir);
	w->graphicId = GraphicId(ID_Graphic_Walker_TradeCaravan) +
		dir + 8 * w->graphicOffset;
}

void WalkerAction_nativeTrader(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->isGhost = 0;
	w->terrainUsage = 0;
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->cartGraphicId = 0;
	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_160_NativeTraderGoingToWarehouse:
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == 8) {
				w->actionState = WalkerActionState_163_NativeTraderAtWarehouse;
			} else if (w->direction == 9) {
				WalkerRoute_remove(walkerId);
			} else if (w->direction == 10) {
				w->state = WalkerState_Dead;
				w->isGhost = 1;
			}
			if (!BuildingIsInUse(w->destinationBuildingId)) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_161_NativeTraderReturning:
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == 8 || w->direction == 10) {
				w->state = WalkerState_Dead;
			} else if (w->direction == 9) {
				WalkerRoute_remove(walkerId);
			}
			break;
		case WalkerActionState_162_NativeTraderCreated:
			w->isGhost = 1;
			w->waitTicks++;
			if (w->waitTicks > 10) {
				w->waitTicks = 0;
				int xTile, yTile;
				int buildingId = Trader_getClosestWarehouseForTradeCaravan(walkerId, w->x, w->y, 0, -1, &xTile, &yTile);
				if (buildingId) {
					w->actionState = WalkerActionState_160_NativeTraderGoingToWarehouse;
					w->destinationBuildingId = buildingId;
					w->destinationX = xTile;
					w->destinationY = yTile;
				} else {
					w->state = WalkerState_Dead;
				}
			}
			w->graphicOffset = 0;
			break;
		case WalkerActionState_163_NativeTraderAtWarehouse:
			w->waitTicks++;
			if (w->waitTicks > 10) {
				w->waitTicks = 0;
				if (WalkerAction_TradeCaravan_canBuy(walkerId, w->destinationBuildingId, 0)) {
					int resource = traderGetBuyResource(w->destinationBuildingId, 0);
					Trader_buyResource(walkerId, resource);
					w->traderAmountBought += 3;
				} else {
					int xTile, yTile;
					int buildingId = Trader_getClosestWarehouseForTradeCaravan(walkerId, w->x, w->y, 0, -1, &xTile, &yTile);
					if (buildingId) {
						w->actionState = WalkerActionState_160_NativeTraderGoingToWarehouse;
						w->destinationBuildingId = buildingId;
						w->destinationX = xTile;
						w->destinationY = yTile;
					} else {
						w->actionState = WalkerActionState_161_NativeTraderReturning;
						w->destinationX = w->sourceX;
						w->destinationY = w->sourceY;
					}
				}
			}
			w->graphicOffset = 0;
			break;
	}
	int dir = (w->direction < 8) ? w->direction : w->previousTileDirection;
	WalkerActionNormalizeDirection(dir);
	
	if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Cartpusher) +
			96 + WalkerActionCorpseGraphicOffset(w);
		w->cartGraphicId = 0;
	} else {
		w->graphicId = GraphicId(ID_Graphic_Walker_Cartpusher) +
			dir + 8 * w->graphicOffset;
	}
	w->cartGraphicId = GraphicId(ID_Graphic_Walker_MigrantCart) +
		8 + 8 * w->resourceId; // BUGFIX should be within else statement?
	if (w->cartGraphicId) {
		w->cartGraphicId += dir;
		WalkerAction_Common_setCartOffset(walkerId, dir);
	}
}

static int tradeShipLostQueue(int walkerId)
{
	int buildingId = Data_Walkers[walkerId].destinationBuildingId;
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (BuildingIsInUse(buildingId) && b->type == Building_Dock &&
		b->numWorkers > 0 && b->data.other.boatWalkerId == walkerId) {
		return 0;
	}
	return 1;
}

static int tradeShipDoneTrading(int walkerId)
{
	int buildingId = Data_Walkers[walkerId].destinationBuildingId;
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (BuildingIsInUse(buildingId) && b->type == Building_Dock && b->numWorkers > 0) {
		for (int i = 0; i < 3; i++) {
			int dockerId = b->data.other.dockWalkerIds[i];
			if (dockerId && Data_Walkers[dockerId].state == WalkerState_Alive &&
				Data_Walkers[dockerId].actionState != WalkerActionState_132_DockerIdling) {
				return 0;
			}
		}
		Data_Walkers[walkerId].tradeShipFailedDockAttempts++;
		if (Data_Walkers[walkerId].tradeShipFailedDockAttempts >= 10) {
			Data_Walkers[walkerId].tradeShipFailedDockAttempts = 11;
			return 1;
		}
		return 0;
	}
	return 1;
}

void WalkerAction_tradeShip(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->isGhost = 0;
	w->isBoat = 1;
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->cartGraphicId = 0;
	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_110_TradeShipCreated:
			w->loadsSoldOrCarrying = 12;
			w->traderAmountBought = 0;
			w->isGhost = 1;
			w->waitTicks++;
			if (w->waitTicks > 20) {
				w->waitTicks = 0;
				int xTile, yTile;
				int dockId = Terrain_Water_getFreeDockDestination(walkerId, &xTile, &yTile);
				if (dockId) {
					w->destinationBuildingId = dockId;
					w->actionState = WalkerActionState_111_TradeShipGoingToDock;
					w->destinationX = xTile;
					w->destinationY = yTile;
				} else if (Terrain_Water_getQueueDockDestination(walkerId, &xTile, &yTile)) {
					w->actionState = WalkerActionState_113_TradeShipGoingToDockQueue;
					w->destinationX = xTile;
					w->destinationY = yTile;
				} else {
					w->state = WalkerState_Dead;
				}
			}
			w->graphicOffset = 0;
			break;
		case WalkerActionState_111_TradeShipGoingToDock:
			WalkerMovement_walkTicks(walkerId, 1);
			w->heightAdjustedTicks = 0;
			if (w->direction == 8) {
				w->actionState = WalkerActionState_112_TradeShipMoored;
			} else if (w->direction == 9) {
				WalkerRoute_remove(walkerId);
			} else if (w->direction == 10) {
				w->state = WalkerState_Dead;
				if (!Data_Message.messageCategoryCount[MessageDelay_BlockedDock]) {
					PlayerMessage_post(1, 15, 0, 0);
					Data_Message.messageCategoryCount[MessageDelay_BlockedDock]++;
				}
			}
			if (!BuildingIsInUse(w->destinationBuildingId)) {
				w->actionState = WalkerActionState_115_TradeShipLeaving;
				w->waitTicks = 0;
				w->destinationX = Data_Scenario.riverExitPoint.x;
				w->destinationY = Data_Scenario.riverExitPoint.y;
			}
			break;
		case WalkerActionState_112_TradeShipMoored:
			if (tradeShipLostQueue(walkerId)) {
				w->tradeShipFailedDockAttempts = 0;
				w->actionState = WalkerActionState_115_TradeShipLeaving;
				w->waitTicks = 0;
				w->destinationX = Data_Scenario.riverEntryPoint.x;
				w->destinationY = Data_Scenario.riverEntryPoint.y;
			} else if (tradeShipDoneTrading(walkerId)) {
				w->tradeShipFailedDockAttempts = 0;
				w->actionState = WalkerActionState_115_TradeShipLeaving;
				w->waitTicks = 0;
				w->destinationX = Data_Scenario.riverEntryPoint.x;
				w->destinationY = Data_Scenario.riverEntryPoint.y;
				Data_Buildings[w->destinationBuildingId].data.other.dockQueuedDockerId = 0;
				Data_Buildings[w->destinationBuildingId].data.other.dockNumShips = 0;
			}
			switch (Data_Buildings[w->destinationBuildingId].data.other.dockOrientation) {
				case 0: w->direction = 2; break;
				case 1: w->direction = 4; break;
				case 2: w->direction = 6; break;
				default:w->direction = 0; break;
			}
			w->graphicOffset = 0;
			Data_Message.messageCategoryCount[MessageDelay_BlockedDock] = 0;
			break;
		case WalkerActionState_113_TradeShipGoingToDockQueue:
			WalkerMovement_walkTicks(walkerId, 1);
			w->heightAdjustedTicks = 0;
			if (w->direction == 8) {
				w->actionState = WalkerActionState_114_TradeShipAnchored;
			} else if (w->direction == 9) {
				WalkerRoute_remove(walkerId);
			} else if (w->direction == 10) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_114_TradeShipAnchored:
			w->waitTicks++;
			if (w->waitTicks > 40) {
				int xTile, yTile;
				int dockId = Terrain_Water_getFreeDockDestination(walkerId, &xTile, &yTile);
				if (dockId) {
					w->destinationBuildingId = dockId;
					w->actionState = WalkerActionState_111_TradeShipGoingToDock;
					w->destinationX = xTile;
					w->destinationY = yTile;
				} else if (Data_Grid_walkerIds[w->gridOffset] != walkerId &&
					Terrain_Water_getQueueDockDestination(walkerId, &xTile, &yTile)) {
					w->actionState = WalkerActionState_113_TradeShipGoingToDockQueue;
					w->destinationX = xTile;
					w->destinationY = yTile;
				}
				w->waitTicks = 0;
			}
			w->graphicOffset = 0;
			break;
		case WalkerActionState_115_TradeShipLeaving:
			WalkerMovement_walkTicks(walkerId, 1);
			w->heightAdjustedTicks = 0;
			if (w->direction == 8) {
				w->actionState = WalkerActionState_110_TradeShipCreated;
				w->state = WalkerState_Dead;
			} else if (w->direction == 9) {
				WalkerRoute_remove(walkerId);
			} else if (w->direction == 10) {
				w->state = WalkerState_Dead;
			}
			break;
	}
	int dir = w->direction < 8 ? w->direction : w->previousTileDirection;
	WalkerActionNormalizeDirection(dir);
	w->graphicId = GraphicId(ID_Graphic_Walker_Ship) + dir;
}

int WalkerAction_TradeShip_isBuyingOrSelling(int walkerId)
{
	int buildingId = Data_Walkers[walkerId].destinationBuildingId;
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (!BuildingIsInUse(buildingId) || b->type != Building_Dock) {
		return TradeShipState_Buying;
	}
	for (int i = 0; i < 3; i++) {
		struct Data_Walker *w = &Data_Walkers[b->data.other.dockWalkerIds[i]];
		if (!b->data.other.dockWalkerIds[i] || w->state != WalkerState_Alive) {
			continue;
		}
		switch (w->actionState) {
			case WalkerActionState_133_DockerImportQueue:
			case WalkerActionState_135_DockerImportGoingToWarehouse:
			case WalkerActionState_138_DockerImportReturning:
			case WalkerActionState_139_DockerImportAtWarehouse:
				return TradeShipState_Buying;
			case WalkerActionState_134_DockerExportQueue:
			case WalkerActionState_136_DockerExportGoingToWarehouse:
			case WalkerActionState_137_DockerExportReturning:
			case WalkerActionState_140_DockerExportAtWarehouse:
				return TradeShipState_Selling;
		}
	}
	return TradeShipState_None;
}
