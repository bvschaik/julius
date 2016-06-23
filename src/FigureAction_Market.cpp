#include "FigureAction_private.h"

#include "Figure.h"
#include "Resource.h"

static int createDeliveryBoy(int leaderId, struct Data_Walker *f)
{
	int boy = Figure_create(Figure_DeliveryBoy, f->x, f->y, 0);
	Data_Walkers[boy].inFrontFigureId = leaderId;
	Data_Walkers[boy].collectingItemId = f->collectingItemId;
	Data_Walkers[boy].buildingId = f->buildingId;
	return boy;
}

static int marketBuyerTakeFoodFromGranary(int walkerId, int marketId, int granaryId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	int resource;
	switch (f->collectingItemId) {
		case Inventory_Wheat: resource = Resource_Wheat; break;
		case Inventory_Vegetables: resource = Resource_Vegetables; break;
		case Inventory_Fruit: resource = Resource_Fruit; break;
		case Inventory_Meat: resource = Resource_Meat; break;
		default: return 0;
	}
	int marketUnits = Data_Buildings[marketId].data.market.inventory[f->collectingItemId];
	int maxUnits = (f->collectingItemId == Inventory_Wheat ? 800 : 600) - marketUnits;
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
		previousBoy = createDeliveryBoy(previousBoy, f);
	}
	return 1;
}

static int marketBuyerTakeResourceFromWarehouse(int walkerId, int marketId, int warehouseId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	int resource;
	switch (f->collectingItemId) {
		case Inventory_Pottery: resource = Resource_Pottery; break;
		case Inventory_Furniture: resource = Resource_Furniture; break;
		case Inventory_Oil: resource = Resource_Oil; break;
		case Inventory_Wine: resource = Resource_Wine; break;
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
	int boy1 = createDeliveryBoy(walkerId, f);
	if (numLoads > 1) {
		createDeliveryBoy(boy1, f);
	}
	return 1;
}

void FigureAction_marketBuyer(int walkerId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	f->terrainUsage = FigureTerrainUsage_Roads;
	f->useCrossCountry = 0;
	f->maxRoamLength = 800;
	
	if (!BuildingIsInUse(f->buildingId) || Data_Buildings[f->buildingId].figureId2 != walkerId) {
		f->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(f, 12);
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(walkerId);
			break;
		case FigureActionState_145_MarketBuyerGoingToStorage:
			FigureMovement_walkTicks(walkerId, 1);
			if (f->direction == DirFigure_8_AtDestination) {
				if (f->collectingItemId > 3) {
					if (!marketBuyerTakeResourceFromWarehouse(walkerId, f->buildingId, f->destinationBuildingId)) {
						f->state = FigureState_Dead;
					}
				} else {
					if (!marketBuyerTakeFoodFromGranary(walkerId, f->buildingId, f->destinationBuildingId)) {
						f->state = FigureState_Dead;
					}
				}
				f->actionState = FigureActionState_146_MarketBuyerReturning;
				f->destinationX = f->sourceX;
				f->destinationY = f->sourceY;
			} else if (f->direction == DirFigure_9_Reroute || f->direction == DirFigure_10_Lost) {
				f->actionState = FigureActionState_146_MarketBuyerReturning;
				f->destinationX = f->sourceX;
				f->destinationY = f->sourceY;
				FigureRoute_remove(walkerId);
			}
			break;
		case FigureActionState_146_MarketBuyerReturning:
			FigureMovement_walkTicks(walkerId, 1);
			if (f->direction == DirFigure_8_AtDestination || f->direction == DirFigure_10_Lost) {
				f->state = FigureState_Dead;
			} else if (f->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(walkerId);
			}
			break;
	}
	WalkerActionUpdateGraphic(f, GraphicId(ID_Graphic_Figure_MarketLady));
}

void FigureAction_deliveryBoy(int walkerId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	f->isGhost = 0;
	f->terrainUsage = FigureTerrainUsage_Roads;
	FigureActionIncreaseGraphicOffset(f, 12);
	f->cartGraphicId = 0;
	
	struct Data_Walker *leader = &Data_Walkers[f->inFrontFigureId];
	if (f->inFrontFigureId <= 0 || leader->actionState == FigureActionState_149_Corpse) {
		f->state = FigureState_Dead;
	} else {
		if (leader->state == FigureState_Alive) {
			if (leader->type == Figure_MarketBuyer || leader->type == Figure_DeliveryBoy) {
				FigureMovement_followTicks(walkerId, f->inFrontFigureId, 1);
			} else {
				f->state = FigureState_Dead;
			}
		} else { // leader arrived at market, drop resource at market
			Data_Buildings[f->buildingId].data.market.inventory[f->collectingItemId] += 100;
			f->state = FigureState_Dead;
		}
	}
	if (leader->isGhost) {
		f->isGhost = 1;
	}
	int dir = (f->direction < 8) ? f->direction : f->previousTileDirection;
	FigureActionNormalizeDirection(dir);
	if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = GraphicId(ID_Graphic_Figure_DeliveryBoy) + 96 +
			FigureActionCorpseGraphicOffset(f);
	} else {
		f->graphicId = GraphicId(ID_Graphic_Figure_DeliveryBoy) +
			dir + 8 * f->graphicOffset;
	}
}
