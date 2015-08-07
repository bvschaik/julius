#include "WalkerAction_private.h"

#include "Resource.h"
#include "Walker.h"

#include "Data/CityInfo.h"
#include "Data/Grid.h"

static const int cartResourceOffsetMultipleLoadsFood[] = {0, 0, 8, 16, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const int cartResourceOffsetMultipleLoadsNonFood[] = {0, 0, 0, 0, 0, 8, 0, 16, 24, 32, 40, 48, 56, 64, 72, 80};
static const int cartResourceOffset8PlusLoadsFood[] = {0, 40, 48, 56, 0, 0, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static void setCartGraphic(struct Data_Walker *w)
{
	w->cartGraphicId = GraphicId(ID_Graphic_Walker_CartpusherCart) +
		8 * w->resourceId + Resource_getGraphicIdOffset(w->resourceId, 1);
}

static void setDestination(struct Data_Walker *w, int actionState, int buildingId, int xDst, int yDst)
{
	w->destinationBuildingId = buildingId;
	w->actionState = actionState;
	w->waitTicks = 0;
	w->destinationX = xDst;
	w->destinationY = yDst;
}

static void determineCartpusherDestination(struct Data_Walker *w, struct Data_Building *b, int roadNetworkId)
{
	int xDst, yDst;
	int understaffedStorages = 0;
	
	// priority 1: warehouse if resource is on stockpile
	int dstBuildingId = Resource_getWarehouseForStoringResource(0, w->x, w->y,
		b->outputResourceId, b->distanceFromEntry, roadNetworkId,
		&understaffedStorages, &xDst, &yDst);
	if (!Data_CityInfo.resourceStockpiled[b->outputResourceId]) {
		dstBuildingId = 0;
	}
	if (dstBuildingId) {
		setDestination(w, WalkerActionState_21_CartpusherDeliveringToWarehouse, dstBuildingId, xDst, yDst);
		return;
	}
	// priority 2: accepting granary for food
	dstBuildingId = Resource_getGranaryForStoringFood(0, w->x, w->y,
		b->outputResourceId, b->distanceFromEntry, roadNetworkId,
		&understaffedStorages, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(w, WalkerActionState_22_CartpusherDeliveringToGranary, dstBuildingId, xDst, yDst);
		return;
	}
	// priority 3: workshop for raw material
	dstBuildingId = Resource_getWorkshopWithRoomForRawMaterial(w->x, w->y,
		b->outputResourceId, b->distanceFromEntry, roadNetworkId, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(w, WalkerActionState_23_CartpusherDeliveringToWorkshop, dstBuildingId, xDst, yDst);
		return;
	}
	// priority 4: warehouse
	dstBuildingId = Resource_getWarehouseForStoringResource(0, w->x, w->y,
		b->outputResourceId, b->distanceFromEntry, roadNetworkId,
		&understaffedStorages, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(w, WalkerActionState_21_CartpusherDeliveringToWarehouse, dstBuildingId, xDst, yDst);
		return;
	}
	// priority 5: granary forced when on stockpile
	dstBuildingId = Resource_getGranaryForStoringFood(1, w->x, w->y,
		b->outputResourceId, b->distanceFromEntry, roadNetworkId,
		&understaffedStorages, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(w, WalkerActionState_22_CartpusherDeliveringToGranary, dstBuildingId, xDst, yDst);
		return;
	}
	// no one will accept
	w->waitTicks = 0;
	// set cartpusher text
	w->minMaxSeen = understaffedStorages ? 2 : 1;
}

static void determineCartpusherDestinationFood(struct Data_Walker *w, int roadNetworkId)
{
	struct Data_Building *b = &Data_Buildings[w->buildingId];
	int xDst, yDst, dstBuildingId;
	// priority 1: accepting granary for food
	dstBuildingId = Resource_getGranaryForStoringFood(0, w->x, w->y,
		b->outputResourceId, b->distanceFromEntry, roadNetworkId,
		0, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(w, WalkerActionState_22_CartpusherDeliveringToGranary, dstBuildingId, xDst, yDst);
		return;
	}
	// priority 2: warehouse
	dstBuildingId = Resource_getWarehouseForStoringResource(0, w->x, w->y,
		b->outputResourceId, b->distanceFromEntry, roadNetworkId,
		0, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(w, WalkerActionState_21_CartpusherDeliveringToWarehouse, dstBuildingId, xDst, yDst);
		return;
	}
	// priority 3: granary
	dstBuildingId = Resource_getGranaryForStoringFood(1, w->x, w->y,
		b->outputResourceId, b->distanceFromEntry, roadNetworkId,
		0, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(w, WalkerActionState_22_CartpusherDeliveringToGranary, dstBuildingId, xDst, yDst);
		return;
	}
	// no one will accept, stand idle
	w->waitTicks = 0;
}

static void updateGraphic(int walkerId, struct Data_Walker *w)
{
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
		if (w->loadsSoldOrCarrying >= 8) {
			w->yOffsetCart -= 40;
		}
	}
}

void WalkerAction_cartpusher(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->cartGraphicId = 0;
	int roadNetworkId = Data_Grid_roadNetworks[w->gridOffset];
	w->terrainUsage = WalkerTerrainUsage_Roads;
	int buildingId = w->buildingId;
	struct Data_Building *b = &Data_Buildings[buildingId];
	
	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_20_CartpusherInitial:
			setCartGraphic(w);
			if (Data_Grid_routingLandCitizen[w->gridOffset] < 0 ||
				Data_Grid_routingLandCitizen[w->gridOffset] > 2) {
				w->state = WalkerState_Dead;
			}
			if (!BuildingIsInUse(buildingId) || b->walkerId != walkerId) {
				w->state = WalkerState_Dead;
			}
			w->waitTicks++;
			if (w->waitTicks > 30) {
				determineCartpusherDestination(w, b, roadNetworkId);
			}
			w->graphicOffset = 0;
			break;
		case WalkerActionState_21_CartpusherDeliveringToWarehouse:
			setCartGraphic(w);
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == DirWalker_8_AtDestination) {
				w->actionState = WalkerActionState_24_CartpusherAtWarehouse;
			} else if (w->direction == DirWalker_9_Reroute) {
				WalkerRoute_remove(walkerId);
				if (Data_Grid_routingLandCitizen[w->gridOffset] != 2) {
					w->actionState = WalkerActionState_20_CartpusherInitial;
				}
				w->waitTicks = 0;
			} else if (w->direction == DirWalker_10_Lost) {
				w->state = WalkerState_Dead;
			}
			if (!BuildingIsInUse(w->destinationBuildingId)) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_22_CartpusherDeliveringToGranary:
			setCartGraphic(w);
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == DirWalker_8_AtDestination) {
				w->actionState = WalkerActionState_25_CartpusherAtGranary;
			} else if (w->direction == DirWalker_9_Reroute) {
				WalkerRoute_remove(walkerId);
				if (Data_Grid_routingLandCitizen[w->gridOffset] != 2) {
					w->actionState = WalkerActionState_20_CartpusherInitial;
				}
				w->waitTicks = 0;
			} else if (w->direction == DirWalker_10_Lost) {
				w->actionState = WalkerActionState_20_CartpusherInitial;
				w->waitTicks = 0;
			}
			if (!BuildingIsInUse(w->destinationBuildingId)) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_23_CartpusherDeliveringToWorkshop:
			setCartGraphic(w);
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == DirWalker_8_AtDestination) {
				w->actionState = WalkerActionState_26_CartpusherAtWorkshop;
			} else if (w->direction == DirWalker_9_Reroute) {
				WalkerRoute_remove(walkerId);
				if (Data_Grid_routingLandCitizen[w->gridOffset] != 2) {
					w->actionState = WalkerActionState_20_CartpusherInitial;
				}
				w->waitTicks = 0;
			} else if (w->direction == DirWalker_10_Lost) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_24_CartpusherAtWarehouse:
			w->waitTicks++;
			if (w->waitTicks > 10) {
				if (Resource_addToWarehouse(w->destinationBuildingId, w->resourceId)) {
					w->actionState = WalkerActionState_27_CartpusherReturning;
					w->waitTicks = 0;
					w->destinationX = w->sourceX;
					w->destinationY = w->sourceY;
				} else {
					WalkerRoute_remove(walkerId);
					w->actionState = WalkerActionState_20_CartpusherInitial;
					w->waitTicks = 0;
				}
			}
			w->graphicOffset = 0;
			break;
		case WalkerActionState_25_CartpusherAtGranary:
			w->waitTicks++;
			if (w->waitTicks > 5) {
				if (Resource_addToGranary(w->destinationBuildingId, w->resourceId, 1)) {
					w->actionState = WalkerActionState_27_CartpusherReturning;
					w->waitTicks = 0;
					w->destinationX = w->sourceX;
					w->destinationY = w->sourceY;
				} else {
					determineCartpusherDestinationFood(w, roadNetworkId);
				}
			}
			w->graphicOffset = 0;
			break;
		case WalkerActionState_26_CartpusherAtWorkshop:
			w->waitTicks++;
			if (w->waitTicks > 5) {
				Resource_addRawMaterialToWorkshop(w->destinationBuildingId);
				w->actionState = WalkerActionState_27_CartpusherReturning;
				w->waitTicks = 0;
				w->destinationX = w->sourceX;
				w->destinationY = w->sourceY;
			}
			w->graphicOffset = 0;
			break;
		case WalkerActionState_27_CartpusherReturning:
			w->cartGraphicId = GraphicId(ID_Graphic_Walker_CartpusherCart);
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == DirWalker_8_AtDestination) {
				w->actionState = WalkerActionState_20_CartpusherInitial;
				w->state = WalkerState_Dead;
			} else if (w->direction == DirWalker_9_Reroute) {
				WalkerRoute_remove(walkerId);
			} else if (w->direction == DirWalker_10_Lost) {
				w->state = WalkerState_Dead;
			}
			break;
	}
	updateGraphic(walkerId, w);
}

static void determineGranarymanDestination(int walkerId, struct Data_Walker *w, int roadNetworkId)
{
	int dstBuildingId, xDst, yDst;
	if (!w->resourceId) {
		// getting granaryman
		dstBuildingId = Resource_getGranaryForGettingFood(w->buildingId, &xDst, &yDst);
		if (dstBuildingId) {
			w->loadsSoldOrCarrying = 0;
			setDestination(w, WalkerActionState_54_WarehousemanGettingFood, dstBuildingId, xDst, yDst);
		} else {
			w->state = WalkerState_Dead;
		}
		return;
	}
	// delivering resource
	// priority 1: another granary
	dstBuildingId = Resource_getGranaryForStoringFood(0, w->x, w->y,
		w->resourceId, Data_Buildings[w->buildingId].distanceFromEntry,
		roadNetworkId, 0, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(w, WalkerActionState_51_WarehousemanDeliveringResource, dstBuildingId, xDst, yDst);
		Resource_removeFromGranary(w->buildingId, w->resourceId, 100);
		return;
	}
	// priority 2: warehouse
	dstBuildingId = Resource_getWarehouseForStoringResource(0, w->x, w->y,
		w->resourceId, Data_Buildings[w->buildingId].distanceFromEntry,
		roadNetworkId, 0, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(w, WalkerActionState_51_WarehousemanDeliveringResource, dstBuildingId, xDst, yDst);
		Resource_removeFromGranary(w->buildingId, w->resourceId, 100);
		return;
	}
	// priority 3: granary even though resource is on stockpile
	dstBuildingId = Resource_getGranaryForStoringFood(1, w->x, w->y,
		w->resourceId, Data_Buildings[w->buildingId].distanceFromEntry,
		roadNetworkId, 0, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(w, WalkerActionState_51_WarehousemanDeliveringResource, dstBuildingId, xDst, yDst);
		Resource_removeFromGranary(w->buildingId, w->resourceId, 100);
		return;
	}
	// nowhere to go to: kill walker
	w->state = WalkerState_Dead;
}

static void removeResourceFromWarehouse(struct Data_Walker *w)
{
	if (w->state != WalkerState_Dead) {
		int err = Resource_removeFromWarehouse(w->buildingId, w->resourceId, 1);
		if (err) {
			w->state = WalkerState_Dead;
		}
	}
}

static void determineWarehousemanDestination(int walkerId, struct Data_Walker *w, int roadNetworkId)
{
	int dstBuildingId, xDst, yDst;
	if (!w->resourceId) {
		// getting warehouseman
		dstBuildingId = Resource_getWarehouseForGettingResource(
			w->buildingId, w->collectingItemId, &xDst, &yDst);
		if (dstBuildingId) {
			w->loadsSoldOrCarrying = 0;
			setDestination(w, WalkerActionState_57_WarehousemanGettingResource, dstBuildingId, xDst, yDst);
			w->terrainUsage = WalkerTerrainUsage_AnyLand;
		} else {
			w->state = WalkerState_Dead;
		}
		return;
	}
	// delivering resource
	// priority 1: weapons to barracks
	dstBuildingId = Resource_getBarracksForWeapon(w->x, w->y, w->resourceId,
		roadNetworkId, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(w, WalkerActionState_51_WarehousemanDeliveringResource, dstBuildingId, xDst, yDst);
		removeResourceFromWarehouse(w);
		return;
	}
	// priority 2: raw materials to workshop
	dstBuildingId = Resource_getWorkshopWithRoomForRawMaterial(w->x, w->y, w->resourceId,
		Data_Buildings[w->buildingId].distanceFromEntry, roadNetworkId, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(w, WalkerActionState_51_WarehousemanDeliveringResource, dstBuildingId, xDst, yDst);
		removeResourceFromWarehouse(w);
		return;
	}
	// priority 3: food to granary
	dstBuildingId = Resource_getGranaryForStoringFood(0, w->x, w->y, w->resourceId,
		Data_Buildings[w->buildingId].distanceFromEntry, roadNetworkId, 0, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(w, WalkerActionState_51_WarehousemanDeliveringResource, dstBuildingId, xDst, yDst);
		removeResourceFromWarehouse(w);
		return;
	}
	// priority 4: food to getting granary
	dstBuildingId = Resource_getGettingGranaryForStoringFood(w->x, w->y, w->resourceId,
		Data_Buildings[w->buildingId].distanceFromEntry, roadNetworkId, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(w, WalkerActionState_51_WarehousemanDeliveringResource, dstBuildingId, xDst, yDst);
		removeResourceFromWarehouse(w);
		return;
	}
	// priority 5: resource to other warehouse
	dstBuildingId = Resource_getWarehouseForStoringResource(w->buildingId, w->x, w->y, w->resourceId,
		Data_Buildings[w->buildingId].distanceFromEntry, roadNetworkId, 0, &xDst, &yDst);
	if (dstBuildingId) {
		if (dstBuildingId == w->buildingId) {
			w->state = WalkerState_Dead;
		} else {
			setDestination(w, WalkerActionState_51_WarehousemanDeliveringResource, dstBuildingId, xDst, yDst);
			removeResourceFromWarehouse(w);
		}
		return;
	}
	// priority 6: raw material to well-stocked workshop
	dstBuildingId = Resource_getWorkshopForRawMaterial(w->x, w->y, w->resourceId,
		Data_Buildings[w->buildingId].distanceFromEntry, roadNetworkId, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(w, WalkerActionState_51_WarehousemanDeliveringResource, dstBuildingId, xDst, yDst);
		removeResourceFromWarehouse(w);
		return;
	}
	// no destination: kill walker
	w->state = WalkerState_Dead;
}

void WalkerAction_warehouseman(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = WalkerTerrainUsage_Roads;
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->cartGraphicId = 0;
	int roadNetworkId = Data_Grid_roadNetworks[w->gridOffset];
	
	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_50_WarehousemanCreated:
			if (!BuildingIsInUse(w->buildingId) ||
				Data_Buildings[w->buildingId].walkerId != walkerId) {
				w->state = WalkerState_Dead;
			}
			w->waitTicks++;
			if (w->waitTicks > 2) {
				if (Data_Buildings[w->buildingId].type == Building_Granary) {
					determineGranarymanDestination(walkerId, w, roadNetworkId);
				} else {
					determineWarehousemanDestination(walkerId, w, roadNetworkId);
				}
			}
			w->graphicOffset = 0;
			break;
		case WalkerActionState_51_WarehousemanDeliveringResource:
			if (w->loadsSoldOrCarrying == 1) {
				w->cartGraphicId = GraphicId(ID_Graphic_Walker_CartpusherCartMultipleFood) +
					8 * w->resourceId - 8 + Resource_getGraphicIdOffset(w->resourceId, 2);
			} else {
				setCartGraphic(w);
			}
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == DirWalker_8_AtDestination) {
				w->actionState = WalkerActionState_52_WarehousemanAtDeliveryBuilding;
			} else if (w->direction == DirWalker_9_Reroute) {
				WalkerRoute_remove(walkerId);
			} else if (w->direction == DirWalker_10_Lost) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_52_WarehousemanAtDeliveryBuilding:
			w->waitTicks++;
			if (w->waitTicks > 4) {
				int buildingId = w->destinationBuildingId;
				switch (Data_Buildings[buildingId].type) {
					case Building_Granary:
						Resource_addToGranary(buildingId, w->resourceId, 0);
						break;
					case Building_Barracks:
						Resource_addWeaponToBarracks(buildingId);
						break;
					case Building_Warehouse:
					case Building_WarehouseSpace:
						Resource_addToWarehouse(buildingId, w->resourceId);
						break;
					default: // workshop
						Resource_addRawMaterialToWorkshop(buildingId);
						break;
				}
				// BUG: what if warehouse/granary is full and returns false?
				w->actionState = WalkerActionState_53_WarehousemanReturningEmpty;
				w->waitTicks = 0;
				w->destinationX = w->sourceX;
				w->destinationY = w->sourceY;
			}
			w->graphicOffset = 0;
			break;
		case WalkerActionState_53_WarehousemanReturningEmpty:
			w->cartGraphicId = GraphicId(ID_Graphic_Walker_CartpusherCart); // empty
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == DirWalker_8_AtDestination || w->direction == DirWalker_10_Lost) {
				w->state = WalkerState_Dead;
			} else if (w->direction == DirWalker_9_Reroute) {
				WalkerRoute_remove(walkerId);
			}
			break;
		case WalkerActionState_54_WarehousemanGettingFood:
			w->cartGraphicId = GraphicId(ID_Graphic_Walker_CartpusherCart); // empty
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == DirWalker_8_AtDestination) {
				w->actionState = WalkerActionState_55_WarehousemanAtGranaryGettingFood;
			} else if (w->direction == DirWalker_9_Reroute) {
				WalkerRoute_remove(walkerId);
			} else if (w->direction == DirWalker_10_Lost) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_55_WarehousemanAtGranaryGettingFood:
			w->waitTicks++;
			if (w->waitTicks > 4) {
				int resource;
				w->loadsSoldOrCarrying = Resource_takeFoodFromGranaryForGettingDeliveryman(
					w->buildingId, w->destinationBuildingId, &resource);
				w->resourceId = resource;
				w->actionState = WalkerActionState_56_WarehousemanReturningWithFood;
				w->waitTicks = 0;
				w->destinationX = w->sourceX;
				w->destinationY = w->sourceY;
				WalkerRoute_remove(walkerId);
			}
			w->graphicOffset = 0;
			break;
		case WalkerActionState_56_WarehousemanReturningWithFood:
			// update graphic
			if (w->loadsSoldOrCarrying <= 0) {
				w->cartGraphicId = GraphicId(ID_Graphic_Walker_CartpusherCart); // empty
			} else if (w->loadsSoldOrCarrying == 1) {
				setCartGraphic(w);
			} else {
				if (w->loadsSoldOrCarrying >= 8) {
					w->cartGraphicId = GraphicId(ID_Graphic_Walker_CartpusherCartMultipleFood) +
						cartResourceOffset8PlusLoadsFood[w->resourceId];
				} else {
					w->cartGraphicId = GraphicId(ID_Graphic_Walker_CartpusherCartMultipleFood) +
						cartResourceOffsetMultipleLoadsFood[w->resourceId];
				}
				w->cartGraphicId += Resource_getGraphicIdOffset(w->resourceId, 2);
			}
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == DirWalker_8_AtDestination) {
				for (int i = 0; i < w->loadsSoldOrCarrying; i++) {
					Resource_addToGranary(w->buildingId, w->resourceId, 0);
				}
				w->state = WalkerState_Dead;
			} else if (w->direction == DirWalker_9_Reroute) {
				WalkerRoute_remove(walkerId);
			} else if (w->direction == DirWalker_10_Lost) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_57_WarehousemanGettingResource:
			w->terrainUsage = WalkerTerrainUsage_AnyLand;
			w->cartGraphicId = GraphicId(ID_Graphic_Walker_CartpusherCart); // empty
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == DirWalker_8_AtDestination) {
				w->actionState = WalkerActionState_58_WarehousemanAtWarehouseGettingResource;
			} else if (w->direction == DirWalker_9_Reroute) {
				WalkerRoute_remove(walkerId);
			} else if (w->direction == DirWalker_10_Lost) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_58_WarehousemanAtWarehouseGettingResource:
			w->terrainUsage = WalkerTerrainUsage_AnyLand;
			w->waitTicks++;
			if (w->waitTicks > 4) {
				w->loadsSoldOrCarrying = 0;
				while (w->loadsSoldOrCarrying < 4 && 0 == Resource_removeFromWarehouse(w->destinationBuildingId, w->collectingItemId, 1)) {
					w->loadsSoldOrCarrying++;
				}
				w->resourceId = w->collectingItemId;
				w->actionState = WalkerActionState_59_WarehousemanReturningWithResource;
				w->waitTicks = 0;
				w->destinationX = w->sourceX;
				w->destinationY = w->sourceY;
				WalkerRoute_remove(walkerId);
			}
			w->graphicOffset = 0;
			break;
		case WalkerActionState_59_WarehousemanReturningWithResource:
			w->terrainUsage = WalkerTerrainUsage_AnyLand;
			// update graphic
			if (w->loadsSoldOrCarrying <= 0) {
				w->cartGraphicId = GraphicId(ID_Graphic_Walker_CartpusherCart); // empty
			} else if (w->loadsSoldOrCarrying == 1) {
				setCartGraphic(w);
			} else {
				if (ResourceIsFood(w->resourceId)) {
					w->cartGraphicId = GraphicId(ID_Graphic_Walker_CartpusherCartMultipleFood) +
						cartResourceOffsetMultipleLoadsFood[w->resourceId];
				} else {
					w->cartGraphicId = GraphicId(ID_Graphic_Walker_CartpusherCartMultipleResource) +
						cartResourceOffsetMultipleLoadsNonFood[w->resourceId];
				}
				w->cartGraphicId += Resource_getGraphicIdOffset(w->resourceId, 2);
			}
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == DirWalker_8_AtDestination) {
				for (int i = 0; i < w->loadsSoldOrCarrying; i++) {
					Resource_addToWarehouse(w->buildingId, w->resourceId);
				}
				w->state = WalkerState_Dead;
			} else if (w->direction == DirWalker_9_Reroute) {
				WalkerRoute_remove(walkerId);
			} else if (w->direction == DirWalker_10_Lost) {
				w->state = WalkerState_Dead;
			}
			break;
	}
	updateGraphic(walkerId, w);
}
