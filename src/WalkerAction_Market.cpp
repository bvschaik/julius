#include "WalkerAction_private.h"

#include "Resource.h"
#include "Walker.h"

#define CREATE_DELIVERY_BOY(boy, leader) \
	int boy = Walker_create(Walker_DeliveryBoy, w->x, w->y, 0);\
	Data_Walkers[boy].inFrontWalkerId = leader;\
	Data_Walkers[boy].collectingItemId = w->collectingItemId;\
	Data_Walkers[boy].buildingId = w->buildingId;

static int marketBuyerTakeFoodFromGranary(int walkerId, int marketId, int granaryId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	int resource;
	switch (w->collectingItemId) {
		case MarketInventory_Wheat: resource = Resource_Wheat; break;
		case MarketInventory_Vegetables: resource = Resource_Vegetables; break;
		case MarketInventory_Fruit: resource = Resource_Fruit; break;
		case MarketInventory_Meat: resource = Resource_Meat; break;
		default: return 0;
	}
	int marketUnits = Data_Buildings[marketId].data.market.inventory.all[w->collectingItemId];
	int maxUnits = (w->collectingItemId == MarketInventory_Wheat) ? 800 - marketUnits : 600 - marketUnits;
	int granaryUnits = Data_Buildings[granaryId].data.storage.resourceStored[resource];
	int numLoads;
	if (granaryUnits >= 800) {
		numLoads = 8;
	} else if (granaryUnits >= 700) {
		numLoads = 7;
	} else if (granaryUnits >= 600) {
		numLoads = 6;
	} else if (granaryUnits >= 500) {
		numLoads = 5;
	} else if (granaryUnits >= 400) {
		numLoads = 4;
	} else if (granaryUnits >= 300) {
		numLoads = 3;
	} else if (granaryUnits >= 200) {
		numLoads = 2;
	} else if (granaryUnits >= 100) {
		numLoads = 1;
	} else {
		numLoads = 0;
	}
	if (numLoads > maxUnits / 100) {
		numLoads = maxUnits / 100;
	}
	if (numLoads <= 0) {
		return 0;
	}
	Resource_removeFromGranary(granaryId, resource, 100 * numLoads);
	// create delivery boys
	int previousBoy = walkerId;
	for (int i = 0; i < numLoads; i++) {
		CREATE_DELIVERY_BOY(nextBoy, previousBoy);
		previousBoy = nextBoy;
	}
	return 1;
}

static int marketBuyerTakeResourceFromWarehouse(int walkerId, int marketId, int warehouseId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	int resource;
	switch (w->collectingItemId) {
		case MarketInventory_Pottery: resource = Resource_Pottery; break;
		case MarketInventory_Furniture: resource = Resource_Furniture; break;
		case MarketInventory_Oil: resource = Resource_Oil; break;
		case MarketInventory_Wine: resource = Resource_Wine; break;
		default: return 0;
	}
	int numLoads;
	int stored = Resource_getAmountStoredInWarehouse(warehouseId, resource);
	if (stored < 2) {
		numLoads = stored;
	} else {
		numLoads = 2;
	}
	if (numLoads <= 0) {
		return 0;
	}
	Resource_removeFromWarehouse(warehouseId, resource, numLoads);
	
	// create delivery boys
	CREATE_DELIVERY_BOY(boy1, walkerId);
	if (numLoads > 1) {
		CREATE_DELIVERY_BOY(boy2, boy1);
	}
	return 1;
}

void WalkerAction_marketBuyer(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = 1;
	w->useCrossCountry = 0;
	w->maxRoamLength = 800;
	
	struct Data_Building *b = &Data_Buildings[w->buildingId];
	if (b->inUse != 1 || b->walkerId2 != walkerId) {
		w->state = WalkerState_Dead;
	}
	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_145_MarketBuyerGoingToStorage:
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == 8) {
				if (w->collectingItemId > 3) {
					if (!marketBuyerTakeResourceFromWarehouse(walkerId, w->buildingId, w->destinationBuildingId)) {
						w->state = WalkerState_Dead;
					}
				} else {
					if (!marketBuyerTakeFoodFromGranary(walkerId, w->buildingId, w->destinationBuildingId)) {
						w->state = WalkerState_Dead;
					}
				}
				w->actionState = WalkerActionState_146_MarketBuyerReturning;
				w->destinationX = w->sourceX;
				w->destinationY = w->sourceY;
			} else if (w->direction == 9 || w->direction == 10) {
				w->actionState = WalkerActionState_146_MarketBuyerReturning;
				w->destinationX = w->sourceX;
				w->destinationY = w->sourceY;
				WalkerRoute_remove(walkerId);
			}
			break;
		case WalkerActionState_146_MarketBuyerReturning:
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == 8 || w->direction == 10) {
				w->state = WalkerState_Dead;
			} else if (w->direction == 9) {
				WalkerRoute_remove(walkerId);
			}
			break;
	}
	WalkerActionUpdateGraphic(w, GraphicId(ID_Graphic_Walker_MarketLady));
}

void WalkerAction_deliveryBoy(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->isGhost = 0;
	w->terrainUsage = 1;
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->cartGraphicId = 0;
	
	struct Data_Walker *leader = &Data_Walkers[w->inFrontWalkerId];
	if (w->inFrontWalkerId <= 0 || w->actionState == WalkerActionState_149_Corpse) {
		w->state = WalkerState_Dead;
	} else {
		if (leader->state == WalkerState_Alive) {
			if (leader->type == Walker_MarketBuyer || w->type == Walker_DeliveryBoy) {
				WalkerMovement_followTicks(walkerId, w->inFrontWalkerId, 1);
			} else {
				w->state = WalkerState_Dead;
			}
		} else { // leader died, drop resource at market
			Data_Buildings[w->buildingId].data.market.inventory.all[w->collectingItemId] += 100;
			w->state = WalkerState_Dead;
		}
	}
	if (leader->isGhost) {
		w->isGhost = 1;
	}
	int dir = (w->direction < 8) ? w->direction : w->previousTileDirection;
	dir = (8 + dir - Data_Settings_Map.orientation) % 8;
	if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Walker_DeliveryBoy) + 96 +
			WalkerActionCorpseGraphicOffset(w);
	} else {
		w->graphicId = GraphicId(ID_Graphic_Walker_DeliveryBoy) +
			dir + 8 * w->graphicOffset;
	}
}
