#include "FigureAction_private.h"

#include "Figure.h"
#include "Resource.h"

#include "figure/route.h"
#include "figure/type.h"
#include "game/resource.h"

static int createDeliveryBoy(int leaderId, struct Data_Figure *f)
{
	figure *boy = figure_create(FIGURE_DELIVERY_BOY, f->x, f->y, 0);
	boy->inFrontFigureId = leaderId;
	boy->collectingItemId = f->collectingItemId;
	boy->buildingId = f->buildingId;
	return boy->id;
}

static int marketBuyerTakeFoodFromGranary(figure *f, int marketId, int granaryId)
{
	int resource;
	switch (f->collectingItemId) {
		case INVENTORY_WHEAT: resource = RESOURCE_WHEAT; break;
		case INVENTORY_VEGETABLES: resource = RESOURCE_VEGETABLES; break;
		case INVENTORY_FRUIT: resource = RESOURCE_FRUIT; break;
		case INVENTORY_MEAT: resource = RESOURCE_MEAT; break;
		default: return 0;
	}
	int marketUnits = Data_Buildings[marketId].data.market.inventory[f->collectingItemId];
	int maxUnits = (f->collectingItemId == INVENTORY_WHEAT ? 800 : 600) - marketUnits;
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
	int previousBoy = f->id;
	for (int i = 0; i < numLoads; i++) {
		previousBoy = createDeliveryBoy(previousBoy, f);
	}
	return 1;
}

static int marketBuyerTakeResourceFromWarehouse(figure *f, int marketId, int warehouseId)
{
	int resource;
	switch (f->collectingItemId) {
		case INVENTORY_POTTERY: resource = RESOURCE_POTTERY; break;
		case INVENTORY_FURNITURE: resource = RESOURCE_FURNITURE; break;
		case INVENTORY_OIL: resource = RESOURCE_OIL; break;
		case INVENTORY_WINE: resource = RESOURCE_WINE; break;
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
	int boy1 = createDeliveryBoy(f->id, f);
	if (numLoads > 1) {
		createDeliveryBoy(boy1, f);
	}
	return 1;
}

void FigureAction_marketBuyer(figure *f)
{
	f->terrainUsage = FigureTerrainUsage_Roads;
	f->useCrossCountry = 0;
	f->maxRoamLength = 800;
	
	if (!BuildingIsInUse(f->buildingId) || Data_Buildings[f->buildingId].figureId2 != f->id) {
		f->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(f, 12);
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(f);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(f);
			break;
		case FigureActionState_145_MarketBuyerGoingToStorage:
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DirFigure_8_AtDestination) {
				if (f->collectingItemId > 3) {
					if (!marketBuyerTakeResourceFromWarehouse(f, f->buildingId, f->destinationBuildingId)) {
						f->state = FigureState_Dead;
					}
				} else {
					if (!marketBuyerTakeFoodFromGranary(f, f->buildingId, f->destinationBuildingId)) {
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
				figure_route_remove(f);
			}
			break;
		case FigureActionState_146_MarketBuyerReturning:
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DirFigure_8_AtDestination || f->direction == DirFigure_10_Lost) {
				f->state = FigureState_Dead;
			} else if (f->direction == DirFigure_9_Reroute) {
				figure_route_remove(f);
			}
			break;
	}
	FigureActionUpdateGraphic(f, image_group(GROUP_FIGURE_MARKET_LADY));
}

void FigureAction_deliveryBoy(figure *f)
{
	f->isGhost = 0;
	f->terrainUsage = FigureTerrainUsage_Roads;
	FigureActionIncreaseGraphicOffset(f, 12);
	f->cartGraphicId = 0;
	
	struct Data_Figure *leader = &Data_Figures[f->inFrontFigureId];
	if (f->inFrontFigureId <= 0 || leader->actionState == FigureActionState_149_Corpse) {
		f->state = FigureState_Dead;
	} else {
		if (leader->state == FigureState_Alive) {
			if (leader->type == FIGURE_MARKET_BUYER || leader->type == FIGURE_DELIVERY_BOY) {
				FigureMovement_followTicks(f, 1);
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
		f->graphicId = image_group(GROUP_FIGURE_DELIVERY_BOY) + 96 +
			FigureActionCorpseGraphicOffset(f);
	} else {
		f->graphicId = image_group(GROUP_FIGURE_DELIVERY_BOY) +
			dir + 8 * f->graphicOffset;
	}
}
