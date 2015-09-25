#include "WalkerAction_private.h"

#include "Resource.h"
#include "Trader.h"
#include "Walker.h"

#include "Data/CityInfo.h"

static int dockerDeliverImportResource(int walkerId, int buildingId)
{
	int shipId = Data_Buildings[buildingId].data.other.boatWalkerId;
	if (!shipId) {
		return 0;
	}
	struct Data_Walker *ship = &Data_Walkers[shipId];
	if (ship->actionState != WalkerActionState_112_TradeShipMoored || ship->loadsSoldOrCarrying <= 0) {
		return 0;
	}
	struct Data_Walker *w = &Data_Walkers[walkerId];
	int x, y;
	if (Data_CityInfo.buildingTradeCenterBuildingId) {
		int tcId = Data_CityInfo.buildingTradeCenterBuildingId;
		x = Data_Buildings[tcId].x;
		y = Data_Buildings[tcId].y;
	} else {
		x = w->x;
		y = w->y;
	}
	int xTile, yTile;
	int warehouseId = Trader_getClosestWarehouseForImportDocker(x, y, ship->empireCityId,
		Data_Buildings[buildingId].distanceFromEntry, Data_Buildings[buildingId].roadNetworkId, &xTile, &yTile);
	if (!warehouseId) {
		return 0;
	}
	ship->loadsSoldOrCarrying--;
	w->destinationBuildingId = warehouseId;
	w->waitTicks = 0;
	w->actionState = WalkerActionState_133_DockerImportQueue;
	w->destinationX = xTile;
	w->destinationY = yTile;
	w->resourceId = Data_CityInfo.tradeNextImportResourceDocker;
	return 1;
}

static int dockerGetExportResource(int walkerId, int buildingId)
{
	int shipId = Data_Buildings[buildingId].data.other.boatWalkerId;
	if (!shipId) {
		return 0;
	}
	struct Data_Walker *ship = &Data_Walkers[shipId];
	if (ship->actionState != WalkerActionState_112_TradeShipMoored || ship->traderAmountBought >= 12) {
		return 0;
	}
	struct Data_Walker *w = &Data_Walkers[walkerId];
	int x, y;
	if (Data_CityInfo.buildingTradeCenterBuildingId) {
		int tcId = Data_CityInfo.buildingTradeCenterBuildingId;
		x = Data_Buildings[tcId].x;
		y = Data_Buildings[tcId].y;
	} else {
		x = w->x;
		y = w->y;
	}
	int xTile, yTile;
	int warehouseId = Trader_getClosestWarehouseForExportDocker(x, y, ship->empireCityId,
		Data_Buildings[buildingId].distanceFromEntry, Data_Buildings[buildingId].roadNetworkId, &xTile, &yTile);
	if (!warehouseId) {
		return 0;
	}
	ship->traderAmountBought++;
	w->destinationBuildingId = warehouseId;
	w->actionState = WalkerActionState_136_DockerExportGoingToWarehouse;
	w->waitTicks = 0;
	w->destinationX = xTile;
	w->destinationY = yTile;
	w->resourceId = Data_CityInfo.tradeNextExportResourceDocker;
	return 1;
}

static void setCartGraphic(struct Data_Walker *w)
{
	w->cartGraphicId = GraphicId(ID_Graphic_Walker_CartpusherCart) + 8 * w->resourceId;
	w->cartGraphicId += Resource_getGraphicIdOffset(w->resourceId, 1);
}

void WalkerAction_docker(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Building *b = &Data_Buildings[w->buildingId];
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->cartGraphicId = 0;
	if (!BuildingIsInUse(w->buildingId)) {
		w->state = WalkerState_Dead;
	}
	if (b->type != Building_Dock && b->type != Building_Wharf) {
		w->state = WalkerState_Dead;
	}
	if (b->data.other.dockNumShips) {
		b->data.other.dockNumShips--;
	}
	if (b->data.other.boatWalkerId) {
		struct Data_Walker *ship = &Data_Walkers[b->data.other.boatWalkerId];
		if (ship->state != WalkerState_Alive || ship->type != Walker_TradeShip) {
			b->data.other.boatWalkerId = 0;
		} else if (Data_Walker_Traders[ship->traderId].totalBought >= 12 ||
				Data_Walker_Traders[ship->traderId].totalSold >= 12) {
			b->data.other.boatWalkerId = 0;
		} else if (ship->actionState == WalkerActionState_115_TradeShipLeaving) {
			b->data.other.boatWalkerId = 0;
		}
	}
	w->terrainUsage = WalkerTerrainUsage_Roads;
	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_132_DockerIdling:
			w->resourceId = 0;
			w->cartGraphicId = 0;
			if (!dockerDeliverImportResource(walkerId, w->buildingId)) {
				dockerGetExportResource(walkerId, w->buildingId);
			}
			w->graphicOffset = 0;
			break;
		case WalkerActionState_133_DockerImportQueue:
			w->cartGraphicId = 0;
			w->graphicOffset = 0;
			if (b->data.other.dockQueuedDockerId <= 0) {
				b->data.other.dockQueuedDockerId = walkerId;
				w->waitTicks = 0;
			}
			if (b->data.other.dockQueuedDockerId == walkerId) {
				b->data.other.dockNumShips = 120;
				w->waitTicks++;
				if (w->waitTicks >= 80) {
					w->actionState = WalkerActionState_135_DockerImportGoingToWarehouse;
					w->waitTicks = 0;
					setCartGraphic(w);
					b->data.other.dockQueuedDockerId = 0;
				}
			} else {
				int hasQueuedDocker = 0;
				for (int i = 0; i < 3; i++) {
					int dockerId = b->data.other.dockWalkerIds[i];
					if (dockerId && b->data.other.dockQueuedDockerId == dockerId &&
							Data_Walkers[dockerId].state == WalkerState_Alive) {
						if (Data_Walkers[dockerId].actionState == WalkerActionState_133_DockerImportQueue ||
							Data_Walkers[dockerId].actionState == WalkerActionState_134_DockerExportQueue) {
							hasQueuedDocker = 1;
						}
					}
				}
				if (!hasQueuedDocker) {
					b->data.other.dockQueuedDockerId = 0;
				}
			}
			break;
		case WalkerActionState_134_DockerExportQueue:
			setCartGraphic(w);
			if (b->data.other.dockQueuedDockerId <= 0) {
				b->data.other.dockQueuedDockerId = walkerId;
				w->waitTicks = 0;
			}
			if (b->data.other.dockQueuedDockerId == walkerId) {
				b->data.other.dockNumShips = 120;
				w->waitTicks++;
				if (w->waitTicks >= 80) {
					w->actionState = WalkerActionState_132_DockerIdling;
					w->waitTicks = 0;
					w->graphicId = 0;
					w->cartGraphicId = 0;
					b->data.other.dockQueuedDockerId = 0;
				}
			}
			w->waitTicks++;
			if (w->waitTicks >= 20) {
				w->actionState = WalkerActionState_132_DockerIdling;
				w->waitTicks = 0;
			}
			w->graphicOffset = 0;
			break;
		case WalkerActionState_135_DockerImportGoingToWarehouse:
			setCartGraphic(w);
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == DirWalker_8_AtDestination) {
				w->actionState = WalkerActionState_139_DockerImportAtWarehouse;
			} else if (w->direction == DirWalker_9_Reroute) {
				WalkerRoute_remove(walkerId);
			} else if (w->direction == DirWalker_10_Lost) {
				w->state = WalkerState_Dead;
			}
			if (!BuildingIsInUse(w->destinationBuildingId)) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_136_DockerExportGoingToWarehouse:
			w->cartGraphicId = GraphicId(ID_Graphic_Walker_CartpusherCart); // empty
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == DirWalker_8_AtDestination) {
				w->actionState = WalkerActionState_140_DockerExportAtWarehouse;
			} else if (w->direction == DirWalker_9_Reroute) {
				WalkerRoute_remove(walkerId);
			} else if (w->direction == DirWalker_10_Lost) {
				w->state = WalkerState_Dead;
			}
			if (!BuildingIsInUse(w->destinationBuildingId)) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_137_DockerExportReturning:
			setCartGraphic(w);
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == DirWalker_8_AtDestination) {
				w->actionState = WalkerActionState_134_DockerExportQueue;
				w->waitTicks = 0;
			} else if (w->direction == DirWalker_9_Reroute) {
				WalkerRoute_remove(walkerId);
			} else if (w->direction == DirWalker_10_Lost) {
				w->state = WalkerState_Dead;
			}
			if (!BuildingIsInUse(w->destinationBuildingId)) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_138_DockerImportReturning:
			setCartGraphic(w);
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == DirWalker_8_AtDestination) {
				w->actionState = WalkerActionState_132_DockerIdling;
			} else if (w->direction == DirWalker_9_Reroute) {
				WalkerRoute_remove(walkerId);
			} else if (w->direction == DirWalker_10_Lost) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_139_DockerImportAtWarehouse:
			setCartGraphic(w);
			w->waitTicks++;
			if (w->waitTicks > 10) {
				int tradeCityId;
				if (b->data.other.boatWalkerId) {
					tradeCityId = Data_Walkers[b->data.other.boatWalkerId].empireCityId;
				} else {
					tradeCityId = 0;
				}
				if (Trader_tryImportResource(w->destinationBuildingId, w->resourceId, tradeCityId)) {
					Trader_sellResource(b->data.other.boatWalkerId, w->resourceId);
					w->actionState = WalkerActionState_138_DockerImportReturning;
					w->waitTicks = 0;
					w->destinationX = w->sourceX;
					w->destinationY = w->sourceY;
					w->resourceId = 0;
					dockerGetExportResource(walkerId, w->buildingId);
				} else {
					w->actionState = WalkerActionState_138_DockerImportReturning;
					w->destinationX = w->sourceX;
					w->destinationY = w->sourceY;
				}
				w->waitTicks = 0;
			}
			w->graphicOffset = 0;
			break;
		case WalkerActionState_140_DockerExportAtWarehouse:
			w->cartGraphicId = GraphicId(ID_Graphic_Walker_CartpusherCart); // empty
			w->waitTicks++;
			if (w->waitTicks > 10) {
				int tradeCityId;
				if (b->data.other.boatWalkerId) {
					tradeCityId = Data_Walkers[b->data.other.boatWalkerId].empireCityId;
				} else {
					tradeCityId = 0;
				}
				w->actionState = WalkerActionState_138_DockerImportReturning;
				w->destinationX = w->sourceX;
				w->destinationY = w->sourceY;
				w->waitTicks = 0;
				if (Trader_tryExportResource(w->destinationBuildingId, w->resourceId, tradeCityId)) {
					Trader_buyResource(b->data.other.boatWalkerId, w->resourceId);
					w->actionState = WalkerActionState_137_DockerExportReturning;
				} else {
					dockerGetExportResource(walkerId, w->buildingId);
				}
			}
			w->graphicOffset = 0;
			break;
	}

	int dir = w->direction < 8 ? w->direction : w->previousTileDirection;
	WalkerActionNormalizeDirection(dir);

	if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Cartpusher) +
			WalkerActionCorpseGraphicOffset(w) + 96;
		w->cartGraphicId = 0;
	} else {
		w->graphicId = GraphicId(ID_Graphic_Walker_Cartpusher) + dir + 8 * w->graphicOffset;
	}
	if (w->cartGraphicId) {
		w->cartGraphicId += dir;
		WalkerAction_Common_setCartOffset(walkerId, dir);
	} else {
		w->graphicId = 0;
	}
}
