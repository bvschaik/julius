#include "WalkerAction_private.h"

#include "Empire.h"
#include "Resource.h"
#include "Trader.h"
#include "Walker.h"

#include "Data/CityInfo.h"
#include "Data/Empire.h"
#include "Data/Trade.h"

static void advanceTradeNextImportResourceCaravan()
{
	Data_CityInfo.tradeNextImportResourceCaravan++;
	if (Data_CityInfo.tradeNextImportResourceCaravan > 15) {
		Data_CityInfo.tradeNextImportResourceCaravan = 1;
	}
}

static void goToNextWarehouse(int walkerId, struct Data_Walker *w, int xSrc, int ySrc, int foo)
{
	int xDst, yDst;
	int warehouseId = Trader_getClosestWarehouseForTradeCaravan(
		walkerId, xSrc, ySrc, w->empireCityId, foo, &xDst, &yDst);
	if (warehouseId) {
		w->destinationBuildingId = warehouseId;
		w->actionState = WalkerActionState_101_TradeCaravanArriving;
		w->waitTicks = 0;
		w->destinationX = xDst;
		w->destinationY = yDst;
	} else {
		w->actionState = WalkerActionState_103_TradeCaravanLeaving;
		w->waitTicks = 0;
		w->destinationX = Data_CityInfo.exitPointX;
		w->destinationY = Data_CityInfo.exitPointY;
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
		if (warehouseId > 0 && Data_Buildings[i].loadsStored > 0 &&
			Empire_canExportResourceToCity(cityId, Data_Buildings[i].subtype.warehouseResourceId)) {
			return 1;
		}
	}
	return 0;
}

static int traderGetBuyResource(int traderId, int warehouseId, int cityId)
{
	if (Data_Buildings[warehouseId].type != Building_Warehouse) {
		return Resource_None;
	}
	for (int i = 0; i < 8; i++) {
		warehouseId = Data_Buildings[warehouseId].nextPartBuildingId;
		if (warehouseId <= 0) {
			break;
		}
		int resource = Data_Buildings[warehouseId].subtype.warehouseResourceId;
		if (Empire_canExportResourceToCity(cityId, resource)) {
			// update stocks
			Data_CityInfo.resourceSpaceInWarehouses[resource]++;
			Data_CityInfo.resourceStored[resource]--;
			Data_Buildings[warehouseId].loadsStored--;
			if (Data_Buildings[warehouseId].loadsStored == 0) {
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
			int graphicId;
			if (Data_Buildings[warehouseId].loadsStored == 0) {
				graphicId = GraphicId(ID_Graphic_WarehouseStorageEmpty);
			} else {
				graphicId = GraphicId(ID_Graphic_WarehouseStorageFilled) +
					Resource_getGraphicIdOffset(resource, 0) +
					4 * (resource - 1) +
					Data_Buildings[warehouseId].loadsStored - 1;
			}
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
	if (Data_Walkers[traderId].traderAmountSold >= 8) {
		return 0;
	}
	int storageId = Data_Buildings[warehouseId].storageId;
	if (Data_Building_Storages[storageId].emptyAll) {
		return 0;
	}
	int numImportable = 0;
	for (int r = 1; r < 16; r++) {
		if (Data_Building_Storages[storageId].resourceState[r] != BuildingStorageState_NotAccepting) {
			if (Empire_canImportResourceFromCity(cityId, r)) {
				numImportable++;
			}
		}
	}
	if (numImportable <= 0) {
		return 0;
	}
	int canImport = 0;
	if (Data_Building_Storages[storageId].resourceState[Data_CityInfo.tradeNextImportResourceCaravan]
			!= BuildingStorageState_NotAccepting &&
		Empire_canImportResourceFromCity(cityId, Data_CityInfo.tradeNextImportResourceCaravan)) {
		canImport = 1;
	}
	if (!canImport) {
		for (int i = 1; i < 16; i++) {
			advanceTradeNextImportResourceCaravan();
			if (Data_Building_Storages[storageId].resourceState[Data_CityInfo.tradeNextImportResourceCaravan]
					!= BuildingStorageState_NotAccepting &&
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
			if (Data_Buildings[w->destinationBuildingId].inUse != 1) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_102_TradeCaravanTrading:
			w->waitTicks++;
			if (w->waitTicks > 10) {
				w->waitTicks = 0;
				int moveOn = 0;
				if (WalkerAction_TradeCaravan_canBuy(walkerId, w->destinationBuildingId, w->empireCityId)) {
					int resource = traderGetBuyResource(walkerId, w->destinationBuildingId, w->empireCityId);
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
						w->traderAmountBought++;
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
	dir = (8 + dir - Data_Settings_Map.orientation) % 8;
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
	dir = (8 + dir - Data_Settings_Map.orientation) % 8;
	w->graphicId = GraphicId(ID_Graphic_Walker_TradeCaravan) +
		dir + 8 * w->graphicOffset;
}


int WalkerAction_TradeShip_canBuyOrSell(int walkerId)
{
	// TODO
	return 0;
}
