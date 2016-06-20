#include "FigureAction_private.h"

#include "Figure.h"
#include "Empire.h"
#include "PlayerMessage.h"
#include "Resource.h"
#include "Terrain.h"
#include "Trader.h"

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

int FigureAction_TradeCaravan_canBuy(int traderId, int warehouseId, int cityId)
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

int FigureAction_TradeCaravan_canSell(int traderId, int warehouseId, int cityId)
{
	if (Data_Buildings[warehouseId].type != Building_Warehouse) {
		return 0;
	}
	if (Data_Walkers[traderId].loadsSoldOrCarrying >= 8) {
		return 0;
	}
	struct Data_Building_Storage *s = &Data_Building_Storages[Data_Buildings[warehouseId].storageId];
	if (s->emptyAll) {
		return 0;
	}
	int numImportable = 0;
	for (int r = Resource_Min; r < Resource_Max; r++) {
		if (s->resourceState[r] != BuildingStorageState_NotAccepting) {
			if (Empire_canImportResourceFromCity(cityId, r)) {
				numImportable++;
			}
		}
	}
	if (numImportable <= 0) {
		return 0;
	}
	int canImport = 0;
	if (s->resourceState[Data_CityInfo.tradeNextImportResourceCaravan] != BuildingStorageState_NotAccepting &&
		Empire_canImportResourceFromCity(cityId, Data_CityInfo.tradeNextImportResourceCaravan)) {
		canImport = 1;
	} else {
		for (int i = Resource_Min; i < Resource_Max; i++) {
			advanceTradeNextImportResourceCaravan();
			if (s->resourceState[Data_CityInfo.tradeNextImportResourceCaravan] != BuildingStorageState_NotAccepting &&
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
		for (int spaceCounter = 0; spaceCounter < 8; spaceCounter++) {
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
	for (int r = Resource_Min; r < Resource_Max; r++) {
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

static void goToNextWarehouse(int walkerId, struct Data_Walker *f, int xSrc, int ySrc, int distToEntry)
{
	int xDst, yDst;
	int warehouseId = Trader_getClosestWarehouseForTradeCaravan(
		walkerId, xSrc, ySrc, f->empireCityId, distToEntry, &xDst, &yDst);
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

void FigureAction_tradeCaravan(int walkerId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	f->isGhost = 0;
	f->terrainUsage = FigureTerrainUsage_PreferRoads;
	FigureActionIncreaseGraphicOffset(f, 12);
	f->cartGraphicId = 0;
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(walkerId);
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
				goToNextWarehouse(walkerId, f, xBase, yBase, 0);
			}
			f->graphicOffset = 0;
			break;
		case FigureActionState_101_TradeCaravanArriving:
			FigureMovement_walkTicks(walkerId, 1);
			switch (f->direction) {
				case DirFigure_8_AtDestination:
					f->actionState = FigureActionState_102_TradeCaravanTrading;
					break;
				case DirFigure_9_Reroute:
					FigureRoute_remove(walkerId);
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
				if (FigureAction_TradeCaravan_canBuy(walkerId, f->destinationBuildingId, f->empireCityId)) {
					int resource = traderGetBuyResource(f->destinationBuildingId, f->empireCityId);
					if (resource) {
						Data_Empire_Trade.tradedThisYear[Data_Empire_Cities[f->empireCityId].routeId][resource]++;
						Trader_buyResource(walkerId, resource);
						f->traderAmountBought++;
					} else {
						moveOn++;
					}
				} else {
					moveOn++;
				}
				if (FigureAction_TradeCaravan_canSell(walkerId, f->destinationBuildingId, f->empireCityId)) {
					int resource = traderGetSellResource(walkerId, f->destinationBuildingId, f->empireCityId);
					if (resource) {
						Data_Empire_Trade.tradedThisYear[Data_Empire_Cities[f->empireCityId].routeId][resource]++;
						Trader_sellResource(walkerId, resource);
						f->loadsSoldOrCarrying++;
					} else {
						moveOn++;
					}
				} else {
					moveOn++;
				}
				if (moveOn == 2) {
					goToNextWarehouse(walkerId, f, f->x, f->y, -1);
				}
			}
			f->graphicOffset = 0;
			break;
		case FigureActionState_103_TradeCaravanLeaving:
			FigureMovement_walkTicks(walkerId, 1);
			switch (f->direction) {
				case DirFigure_8_AtDestination:
					f->actionState = FigureActionState_100_TradeCaravanCreated;
					f->state = FigureState_Dead;
					break;
				case DirFigure_9_Reroute:
					FigureRoute_remove(walkerId);
					break;
				case DirFigure_10_Lost:
					f->state = FigureState_Dead;
					break;
			}
			break;
	}
	int dir = f->direction < 8 ? f->direction : f->previousTileDirection;
	WalkerActionNormalizeDirection(dir);
	f->graphicId = GraphicId(ID_Graphic_Figure_TradeCaravan) + dir + 8 * f->graphicOffset;
}

void FigureAction_tradeCaravanDonkey(int walkerId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	f->isGhost = 0;
	f->terrainUsage = FigureTerrainUsage_PreferRoads;
	FigureActionIncreaseGraphicOffset(f, 12);
	f->cartGraphicId = 0;

	struct Data_Walker *leader = &Data_Walkers[f->inFrontWalkerId];
	if (f->inFrontWalkerId <= 0) {
		f->state = FigureState_Dead;
	} else {
		if (leader->actionState == FigureActionState_149_Corpse) {
			f->state = FigureState_Dead;
		} else if (leader->state != FigureState_Alive) {
			f->state = FigureState_Dead;
		} else if (leader->type != Figure_TradeCaravan && leader->type != Figure_TradeCaravanDonkey) {
			f->state = FigureState_Dead;
		} else {
			FigureMovement_followTicks(walkerId, f->inFrontWalkerId, 1);
		}
	}

	if (leader->isGhost) {
		f->isGhost = 1;
	}
	int dir = f->direction < 8 ? f->direction : f->previousTileDirection;
	WalkerActionNormalizeDirection(dir);
	f->graphicId = GraphicId(ID_Graphic_Figure_TradeCaravan) + dir + 8 * f->graphicOffset;
}

void FigureAction_nativeTrader(int walkerId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	f->isGhost = 0;
	f->terrainUsage = FigureTerrainUsage_Any;
	FigureActionIncreaseGraphicOffset(f, 12);
	f->cartGraphicId = 0;
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(walkerId);
			break;
		case FigureActionState_160_NativeTraderGoingToWarehouse:
			FigureMovement_walkTicks(walkerId, 1);
			if (f->direction == DirFigure_8_AtDestination) {
				f->actionState = FigureActionState_163_NativeTraderAtWarehouse;
			} else if (f->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(walkerId);
			} else if (f->direction == DirFigure_10_Lost) {
				f->state = FigureState_Dead;
				f->isGhost = 1;
			}
			if (!BuildingIsInUse(f->destinationBuildingId)) {
				f->state = FigureState_Dead;
			}
			break;
		case FigureActionState_161_NativeTraderReturning:
			FigureMovement_walkTicks(walkerId, 1);
			if (f->direction == DirFigure_8_AtDestination || f->direction == DirFigure_10_Lost) {
				f->state = FigureState_Dead;
			} else if (f->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(walkerId);
			}
			break;
		case FigureActionState_162_NativeTraderCreated:
			f->isGhost = 1;
			f->waitTicks++;
			if (f->waitTicks > 10) {
				f->waitTicks = 0;
				int xTile, yTile;
				int buildingId = Trader_getClosestWarehouseForTradeCaravan(walkerId, f->x, f->y, 0, -1, &xTile, &yTile);
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
				if (FigureAction_TradeCaravan_canBuy(walkerId, f->destinationBuildingId, 0)) {
					int resource = traderGetBuyResource(f->destinationBuildingId, 0);
					Trader_buyResource(walkerId, resource);
					f->traderAmountBought += 3;
				} else {
					int xTile, yTile;
					int buildingId = Trader_getClosestWarehouseForTradeCaravan(walkerId, f->x, f->y, 0, -1, &xTile, &yTile);
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
	WalkerActionNormalizeDirection(dir);
	
	if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = GraphicId(ID_Graphic_Figure_Cartpusher) +
			96 + WalkerActionCorpseGraphicOffset(f);
		f->cartGraphicId = 0;
	} else {
		f->graphicId = GraphicId(ID_Graphic_Figure_Cartpusher) +
			dir + 8 * f->graphicOffset;
	}
	f->cartGraphicId = GraphicId(ID_Graphic_Figure_MigrantCart) +
		8 + 8 * f->resourceId; // BUGFIX should be within else statement?
	if (f->cartGraphicId) {
		f->cartGraphicId += dir;
		FigureAction_Common_setCartOffset(walkerId, dir);
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
			if (dockerId && Data_Walkers[dockerId].state == FigureState_Alive &&
				Data_Walkers[dockerId].actionState != FigureActionState_132_DockerIdling) {
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

void FigureAction_tradeShip(int walkerId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	f->isGhost = 0;
	f->isBoat = 1;
	FigureActionIncreaseGraphicOffset(f, 12);
	f->cartGraphicId = 0;
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(walkerId);
			break;
		case FigureActionState_110_TradeShipCreated:
			f->loadsSoldOrCarrying = 12;
			f->traderAmountBought = 0;
			f->isGhost = 1;
			f->waitTicks++;
			if (f->waitTicks > 20) {
				f->waitTicks = 0;
				int xTile, yTile;
				int dockId = Terrain_Water_getFreeDockDestination(walkerId, &xTile, &yTile);
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
			FigureMovement_walkTicks(walkerId, 1);
			f->heightAdjustedTicks = 0;
			if (f->direction == DirFigure_8_AtDestination) {
				f->actionState = FigureActionState_112_TradeShipMoored;
			} else if (f->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(walkerId);
			} else if (f->direction == DirFigure_10_Lost) {
				f->state = FigureState_Dead;
				if (!Data_Message.messageCategoryCount[MessageDelay_BlockedDock]) {
					PlayerMessage_post(1, Message_15_NavigationImpossible, 0, 0);
					Data_Message.messageCategoryCount[MessageDelay_BlockedDock]++;
				}
			}
			if (!BuildingIsInUse(f->destinationBuildingId)) {
				f->actionState = FigureActionState_115_TradeShipLeaving;
				f->waitTicks = 0;
				f->destinationX = Data_Scenario.riverExitPoint.x;
				f->destinationY = Data_Scenario.riverExitPoint.y;
			}
			break;
		case FigureActionState_112_TradeShipMoored:
			if (tradeShipLostQueue(walkerId)) {
				f->tradeShipFailedDockAttempts = 0;
				f->actionState = FigureActionState_115_TradeShipLeaving;
				f->waitTicks = 0;
				f->destinationX = Data_Scenario.riverEntryPoint.x;
				f->destinationY = Data_Scenario.riverEntryPoint.y;
			} else if (tradeShipDoneTrading(walkerId)) {
				f->tradeShipFailedDockAttempts = 0;
				f->actionState = FigureActionState_115_TradeShipLeaving;
				f->waitTicks = 0;
				f->destinationX = Data_Scenario.riverEntryPoint.x;
				f->destinationY = Data_Scenario.riverEntryPoint.y;
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
			Data_Message.messageCategoryCount[MessageDelay_BlockedDock] = 0;
			break;
		case FigureActionState_113_TradeShipGoingToDockQueue:
			FigureMovement_walkTicks(walkerId, 1);
			f->heightAdjustedTicks = 0;
			if (f->direction == DirFigure_8_AtDestination) {
				f->actionState = FigureActionState_114_TradeShipAnchored;
			} else if (f->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(walkerId);
			} else if (f->direction == DirFigure_10_Lost) {
				f->state = FigureState_Dead;
			}
			break;
		case FigureActionState_114_TradeShipAnchored:
			f->waitTicks++;
			if (f->waitTicks > 40) {
				int xTile, yTile;
				int dockId = Terrain_Water_getFreeDockDestination(walkerId, &xTile, &yTile);
				if (dockId) {
					f->destinationBuildingId = dockId;
					f->actionState = FigureActionState_111_TradeShipGoingToDock;
					f->destinationX = xTile;
					f->destinationY = yTile;
				} else if (Data_Grid_figureIds[f->gridOffset] != walkerId &&
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
			FigureMovement_walkTicks(walkerId, 1);
			f->heightAdjustedTicks = 0;
			if (f->direction == DirFigure_8_AtDestination) {
				f->actionState = FigureActionState_110_TradeShipCreated;
				f->state = FigureState_Dead;
			} else if (f->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(walkerId);
			} else if (f->direction == DirFigure_10_Lost) {
				f->state = FigureState_Dead;
			}
			break;
	}
	int dir = f->direction < 8 ? f->direction : f->previousTileDirection;
	WalkerActionNormalizeDirection(dir);
	f->graphicId = GraphicId(ID_Graphic_Figure_Ship) + dir;
}

int FigureAction_TradeShip_isBuyingOrSelling(int walkerId)
{
	int buildingId = Data_Walkers[walkerId].destinationBuildingId;
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (!BuildingIsInUse(buildingId) || b->type != Building_Dock) {
		return TradeShipState_Buying;
	}
	for (int i = 0; i < 3; i++) {
		struct Data_Walker *f = &Data_Walkers[b->data.other.dockWalkerIds[i]];
		if (!b->data.other.dockWalkerIds[i] || f->state != FigureState_Alive) {
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
