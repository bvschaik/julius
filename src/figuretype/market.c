#include "market.h"

#include "building/building.h"
#include "building/granary.h"
#include "building/warehouse.h"
#include "figure/image.h"
#include "figure/route.h"
#include "game/resource.h"
#include "graphics/image.h"

#include "FigureAction.h"
#include "FigureMovement.h"

static int createDeliveryBoy(int leader_id, figure *f)
{
    figure *boy = figure_create(FIGURE_DELIVERY_BOY, f->x, f->y, 0);
    boy->inFrontFigureId = leader_id;
    boy->collectingItemId = f->collectingItemId;
    boy->buildingId = f->buildingId;
    return boy->id;
}

static int marketBuyerTakeFoodFromGranary(figure *f, int market_id, int granary_id)
{
    int resource;
    switch (f->collectingItemId) {
        case INVENTORY_WHEAT: resource = RESOURCE_WHEAT; break;
        case INVENTORY_VEGETABLES: resource = RESOURCE_VEGETABLES; break;
        case INVENTORY_FRUIT: resource = RESOURCE_FRUIT; break;
        case INVENTORY_MEAT: resource = RESOURCE_MEAT; break;
        default: return 0;
    }
    building *granary = building_get(granary_id);
    int market_units = building_get(market_id)->data.market.inventory[f->collectingItemId];
    int max_units = (f->collectingItemId == INVENTORY_WHEAT ? 800 : 600) - market_units;
    int granary_units = granary->data.storage.resourceStored[resource];
    int num_loads;
    if (granary_units >= 800) {
        num_loads = 8;
    } else if (granary_units >= 700) {
        num_loads = 7;
    } else if (granary_units >= 600) {
        num_loads = 6;
    } else if (granary_units >= 500) {
        num_loads = 5;
    } else if (granary_units >= 400) {
        num_loads = 4;
    } else if (granary_units >= 300) {
        num_loads = 3;
    } else if (granary_units >= 200) {
        num_loads = 2;
    } else if (granary_units >= 100) {
        num_loads = 1;
    } else {
        num_loads = 0;
    }
    if (num_loads > max_units / 100) {
        num_loads = max_units / 100;
    }
    if (num_loads <= 0) {
        return 0;
    }
    building_granary_remove_resource(granary, resource, 100 * num_loads);
    // create delivery boys
    int previous_boy = f->id;
    for (int i = 0; i < num_loads; i++) {
        previous_boy = createDeliveryBoy(previous_boy, f);
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
    building *warehouse = building_get(warehouseId);
    int numLoads;
    int stored = building_warehouse_get_amount(warehouse, resource);
    if (stored < 2) {
        numLoads = stored;
    } else {
        numLoads = 2;
    }
    if (numLoads <= 0) {
        return 0;
    }
    building_warehouse_remove_resource(warehouse, resource, numLoads);
    
    // create delivery boys
    int boy1 = createDeliveryBoy(f->id, f);
    if (numLoads > 1) {
        createDeliveryBoy(boy1, f);
    }
    return 1;
}

void figure_market_buyer_action(figure *f)
{
    f->terrainUsage = FigureTerrainUsage_Roads;
    f->useCrossCountry = 0;
    f->maxRoamLength = 800;
    
    building *b = building_get(f->buildingId);
    if (!BuildingIsInUse(b) || b->figureId2 != f->id) {
        f->state = FigureState_Dead;
    }
    figure_image_increase_offset(f, 12);
    switch (f->actionState) {
        case FigureActionState_150_Attack:
            FigureAction_Common_handleAttack(f);
            break;
        case FigureActionState_149_Corpse:
            FigureAction_Common_handleCorpse(f);
            break;
        case FIGURE_ACTION_145_MARKET_BUYER_GOING_TO_STORAGE:
            FigureMovement_walkTicks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                if (f->collectingItemId > 3) {
                    if (!marketBuyerTakeResourceFromWarehouse(f, f->buildingId, f->destinationBuildingId)) {
                        f->state = FigureState_Dead;
                    }
                } else {
                    if (!marketBuyerTakeFoodFromGranary(f, f->buildingId, f->destinationBuildingId)) {
                        f->state = FigureState_Dead;
                    }
                }
                f->actionState = FIGURE_ACTION_146_MARKET_BUYER_RETURNING;
                f->destinationX = f->sourceX;
                f->destinationY = f->sourceY;
            } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->actionState = FIGURE_ACTION_146_MARKET_BUYER_RETURNING;
                f->destinationX = f->sourceX;
                f->destinationY = f->sourceY;
                figure_route_remove(f);
            }
            break;
        case FIGURE_ACTION_146_MARKET_BUYER_RETURNING:
            FigureMovement_walkTicks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION || f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            }
            break;
    }
    figure_image_update(f, image_group(GROUP_FIGURE_MARKET_LADY));
}

void figure_delivery_boy_action(figure *f)
{
    f->isGhost = 0;
    f->terrainUsage = FigureTerrainUsage_Roads;
    figure_image_increase_offset(f, 12);
    f->cartGraphicId = 0;
    
    figure *leader = figure_get(f->inFrontFigureId);
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
            building_get(f->buildingId)->data.market.inventory[f->collectingItemId] += 100;
            f->state = FigureState_Dead;
        }
    }
    if (leader->isGhost) {
        f->isGhost = 1;
    }
    int dir = figure_image_normalize_direction(f->direction < 8 ? f->direction : f->previousTileDirection);
    if (f->actionState == FigureActionState_149_Corpse) {
        f->graphicId = image_group(GROUP_FIGURE_DELIVERY_BOY) + 96 +
            figure_image_corpse_offset(f);
    } else {
        f->graphicId = image_group(GROUP_FIGURE_DELIVERY_BOY) +
            dir + 8 * f->graphicOffset;
    }
}
