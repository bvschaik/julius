#include "FigureAction_private.h"

#include "Resource.h"

#include "Data/CityInfo.h"

#include "building/barracks.h"
#include "building/building.h"
#include "building/granary.h"
#include "building/industry.h"
#include "building/warehouse.h"
#include "figure/route.h"
#include "game/resource.h"
#include "map/road_network.h"
#include "map/routing_terrain.h"

static const int cartResourceOffsetMultipleLoadsFood[] = {0, 0, 8, 16, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const int cartResourceOffsetMultipleLoadsNonFood[] = {0, 0, 0, 0, 0, 8, 0, 16, 24, 32, 40, 48, 56, 64, 72, 80};
static const int cartResourceOffset8PlusLoadsFood[] = {0, 40, 48, 56, 0, 0, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static void setCartGraphic(figure *f)
{
	f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART) +
		8 * f->resourceId + resource_image_offset(f->resourceId, RESOURCE_IMAGE_CART);
}

static void setDestination(figure *f, int actionState, int buildingId, int xDst, int yDst)
{
	f->destinationBuildingId = buildingId;
	f->actionState = actionState;
	f->waitTicks = 0;
	f->destinationX = xDst;
	f->destinationY = yDst;
}

static void determineCartpusherDestination(figure *f, building *b, int roadNetworkId)
{
	int xDst, yDst;
	int understaffedStorages = 0;
	
	// priority 1: warehouse if resource is on stockpile
	int dstBuildingId = building_warehouse_for_storing(0, f->x, f->y,
		b->outputResourceId, b->distanceFromEntry, roadNetworkId,
		&understaffedStorages, &xDst, &yDst);
	if (!Data_CityInfo.resourceStockpiled[b->outputResourceId]) {
		dstBuildingId = 0;
	}
	if (dstBuildingId) {
		setDestination(f, FigureActionState_21_CartpusherDeliveringToWarehouse, dstBuildingId, xDst, yDst);
		return;
	}
	// priority 2: accepting granary for food
	dstBuildingId = Resource_getGranaryForStoringFood(0, f->x, f->y,
		b->outputResourceId, b->distanceFromEntry, roadNetworkId,
		&understaffedStorages, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(f, FigureActionState_22_CartpusherDeliveringToGranary, dstBuildingId, xDst, yDst);
		return;
	}
	// priority 3: workshop for raw material
	dstBuildingId = building_get_workshop_for_raw_material_with_room(f->x, f->y,
		b->outputResourceId, b->distanceFromEntry, roadNetworkId, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(f, FigureActionState_23_CartpusherDeliveringToWorkshop, dstBuildingId, xDst, yDst);
		return;
	}
	// priority 4: warehouse
	dstBuildingId = building_warehouse_for_storing(0, f->x, f->y,
		b->outputResourceId, b->distanceFromEntry, roadNetworkId,
		&understaffedStorages, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(f, FigureActionState_21_CartpusherDeliveringToWarehouse, dstBuildingId, xDst, yDst);
		return;
	}
	// priority 5: granary forced when on stockpile
	dstBuildingId = Resource_getGranaryForStoringFood(1, f->x, f->y,
		b->outputResourceId, b->distanceFromEntry, roadNetworkId,
		&understaffedStorages, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(f, FigureActionState_22_CartpusherDeliveringToGranary, dstBuildingId, xDst, yDst);
		return;
	}
	// no one will accept
	f->waitTicks = 0;
	// set cartpusher text
	f->minMaxSeen = understaffedStorages ? 2 : 1;
}

static void determineCartpusherDestinationFood(figure *f, int roadNetworkId)
{
	building *b = building_get(f->buildingId);
	int xDst, yDst, dstBuildingId;
	// priority 1: accepting granary for food
	dstBuildingId = Resource_getGranaryForStoringFood(0, f->x, f->y,
		b->outputResourceId, b->distanceFromEntry, roadNetworkId,
		0, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(f, FigureActionState_22_CartpusherDeliveringToGranary, dstBuildingId, xDst, yDst);
		return;
	}
	// priority 2: warehouse
	dstBuildingId = building_warehouse_for_storing(0, f->x, f->y,
		b->outputResourceId, b->distanceFromEntry, roadNetworkId,
		0, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(f, FigureActionState_21_CartpusherDeliveringToWarehouse, dstBuildingId, xDst, yDst);
		return;
	}
	// priority 3: granary
	dstBuildingId = Resource_getGranaryForStoringFood(1, f->x, f->y,
		b->outputResourceId, b->distanceFromEntry, roadNetworkId,
		0, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(f, FigureActionState_22_CartpusherDeliveringToGranary, dstBuildingId, xDst, yDst);
		return;
	}
	// no one will accept, stand idle
	f->waitTicks = 0;
}

static void updateGraphic(figure *f)
{
	int dir = f->direction < 8 ? f->direction : f->previousTileDirection;
	FigureActionNormalizeDirection(dir);

	if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = image_group(GROUP_FIGURE_CARTPUSHER) +
			FigureActionCorpseGraphicOffset(f) + 96;
		f->cartGraphicId = 0;
	} else {
		f->graphicId = image_group(GROUP_FIGURE_CARTPUSHER) + dir + 8 * f->graphicOffset;
	}
	if (f->cartGraphicId) {
		f->cartGraphicId += dir;
		FigureAction_Common_setCartOffset(f, dir);
		if (f->loadsSoldOrCarrying >= 8) {
			f->yOffsetCart -= 40;
		}
	}
}

static void reroute_cartpusher(figure *f)
{
    figure_route_remove(f);
    if (!map_routing_citizen_is_passable_terrain(f->gridOffset)) {
        f->actionState = FigureActionState_20_CartpusherInitial;
    }
    f->waitTicks = 0;
}

void FigureAction_cartpusher(figure *f)
{
	FigureActionIncreaseGraphicOffset(f, 12);
	f->cartGraphicId = 0;
	int roadNetworkId = map_road_network_get(f->gridOffset);
	f->terrainUsage = FigureTerrainUsage_Roads;
	building *b = building_get(f->buildingId);
	
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(f);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(f);
			break;
		case FigureActionState_20_CartpusherInitial:
			setCartGraphic(f);
			if (!map_routing_citizen_is_passable(f->gridOffset)) {
				f->state = FigureState_Dead;
			}
			if (!BuildingIsInUse(b) || b->figureId != f->id) {
				f->state = FigureState_Dead;
			}
			f->waitTicks++;
			if (f->waitTicks > 30) {
				determineCartpusherDestination(f, b, roadNetworkId);
			}
			f->graphicOffset = 0;
			break;
		case FigureActionState_21_CartpusherDeliveringToWarehouse:
			setCartGraphic(f);
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DIR_FIGURE_AT_DESTINATION) {
				f->actionState = FigureActionState_24_CartpusherAtWarehouse;
			} else if (f->direction == DIR_FIGURE_REROUTE) {
				reroute_cartpusher(f);
			} else if (f->direction == DIR_FIGURE_LOST) {
				f->state = FigureState_Dead;
			}
			if (!BuildingIsInUse(building_get(f->destinationBuildingId))) {
				f->state = FigureState_Dead;
			}
			break;
		case FigureActionState_22_CartpusherDeliveringToGranary:
			setCartGraphic(f);
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DIR_FIGURE_AT_DESTINATION) {
				f->actionState = FigureActionState_25_CartpusherAtGranary;
			} else if (f->direction == DIR_FIGURE_REROUTE) {
				reroute_cartpusher(f);
			} else if (f->direction == DIR_FIGURE_LOST) {
				f->actionState = FigureActionState_20_CartpusherInitial;
				f->waitTicks = 0;
			}
			if (!BuildingIsInUse(building_get(f->destinationBuildingId))) {
				f->state = FigureState_Dead;
			}
			break;
		case FigureActionState_23_CartpusherDeliveringToWorkshop:
			setCartGraphic(f);
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DIR_FIGURE_AT_DESTINATION) {
				f->actionState = FigureActionState_26_CartpusherAtWorkshop;
			} else if (f->direction == DIR_FIGURE_REROUTE) {
				reroute_cartpusher(f);
			} else if (f->direction == DIR_FIGURE_LOST) {
				f->state = FigureState_Dead;
			}
			break;
		case FigureActionState_24_CartpusherAtWarehouse:
			f->waitTicks++;
			if (f->waitTicks > 10) {
				if (building_warehouse_add_resource(building_get(f->destinationBuildingId), f->resourceId)) {
					f->actionState = FigureActionState_27_CartpusherReturning;
					f->waitTicks = 0;
					f->destinationX = f->sourceX;
					f->destinationY = f->sourceY;
				} else {
					figure_route_remove(f);
					f->actionState = FigureActionState_20_CartpusherInitial;
					f->waitTicks = 0;
				}
			}
			f->graphicOffset = 0;
			break;
		case FigureActionState_25_CartpusherAtGranary:
			f->waitTicks++;
			if (f->waitTicks > 5) {
				if (building_granary_add_resource(building_get(f->destinationBuildingId), f->resourceId, 1)) {
					f->actionState = FigureActionState_27_CartpusherReturning;
					f->waitTicks = 0;
					f->destinationX = f->sourceX;
					f->destinationY = f->sourceY;
				} else {
					determineCartpusherDestinationFood(f, roadNetworkId);
				}
			}
			f->graphicOffset = 0;
			break;
		case FigureActionState_26_CartpusherAtWorkshop:
			f->waitTicks++;
			if (f->waitTicks > 5) {
				building_workshop_add_raw_material(building_get(f->destinationBuildingId));
				f->actionState = FigureActionState_27_CartpusherReturning;
				f->waitTicks = 0;
				f->destinationX = f->sourceX;
				f->destinationY = f->sourceY;
			}
			f->graphicOffset = 0;
			break;
		case FigureActionState_27_CartpusherReturning:
			f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART);
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DIR_FIGURE_AT_DESTINATION) {
				f->actionState = FigureActionState_20_CartpusherInitial;
				f->state = FigureState_Dead;
			} else if (f->direction == DIR_FIGURE_REROUTE) {
				figure_route_remove(f);
			} else if (f->direction == DIR_FIGURE_LOST) {
				f->state = FigureState_Dead;
			}
			break;
	}
	updateGraphic(f);
}

static void determineGranarymanDestination(figure *f, int roadNetworkId)
{
	int dstBuildingId, xDst, yDst;
	if (!f->resourceId) {
		// getting granaryman
		dstBuildingId = Resource_getGranaryForGettingFood(f->buildingId, &xDst, &yDst);
		if (dstBuildingId) {
			f->loadsSoldOrCarrying = 0;
			setDestination(f, FigureActionState_54_WarehousemanGettingFood, dstBuildingId, xDst, yDst);
		} else {
			f->state = FigureState_Dead;
		}
		return;
	}
	building *granary = building_get(f->buildingId);
	// delivering resource
	// priority 1: another granary
	dstBuildingId = Resource_getGranaryForStoringFood(0, f->x, f->y,
		f->resourceId, granary->distanceFromEntry,
		roadNetworkId, 0, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(f, FigureActionState_51_WarehousemanDeliveringResource, dstBuildingId, xDst, yDst);
		building_granary_remove_resource(granary, f->resourceId, 100);
		return;
	}
	// priority 2: warehouse
	dstBuildingId = building_warehouse_for_storing(0, f->x, f->y,
		f->resourceId, granary->distanceFromEntry,
		roadNetworkId, 0, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(f, FigureActionState_51_WarehousemanDeliveringResource, dstBuildingId, xDst, yDst);
		building_granary_remove_resource(granary, f->resourceId, 100);
		return;
	}
	// priority 3: granary even though resource is on stockpile
	dstBuildingId = Resource_getGranaryForStoringFood(1, f->x, f->y,
		f->resourceId, granary->distanceFromEntry,
		roadNetworkId, 0, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(f, FigureActionState_51_WarehousemanDeliveringResource, dstBuildingId, xDst, yDst);
		building_granary_remove_resource(granary, f->resourceId, 100);
		return;
	}
	// nowhere to go to: kill figure
	f->state = FigureState_Dead;
}

static void removeResourceFromWarehouse(figure *f)
{
	if (f->state != FigureState_Dead) {
		int err = building_warehouse_remove_resource(building_get(f->buildingId), f->resourceId, 1);
		if (err) {
			f->state = FigureState_Dead;
		}
	}
}

static void determineWarehousemanDestination(figure *f, int roadNetworkId)
{
	int dstBuildingId, xDst, yDst;
	if (!f->resourceId) {
		// getting warehouseman
		dstBuildingId = building_warehouse_for_getting(
			building_get(f->buildingId), f->collectingItemId, &xDst, &yDst);
		if (dstBuildingId) {
			f->loadsSoldOrCarrying = 0;
			setDestination(f, FigureActionState_57_WarehousemanGettingResource, dstBuildingId, xDst, yDst);
			f->terrainUsage = FigureTerrainUsage_PreferRoads;
		} else {
			f->state = FigureState_Dead;
		}
		return;
	}
	building *warehouse = building_get(f->buildingId);
	// delivering resource
	// priority 1: weapons to barracks
	dstBuildingId = building_get_barracks_for_weapon(f->resourceId, roadNetworkId, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(f, FigureActionState_51_WarehousemanDeliveringResource, dstBuildingId, xDst, yDst);
		removeResourceFromWarehouse(f);
		return;
	}
	// priority 2: raw materials to workshop
	dstBuildingId = building_get_workshop_for_raw_material_with_room(f->x, f->y, f->resourceId,
		warehouse->distanceFromEntry, roadNetworkId, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(f, FigureActionState_51_WarehousemanDeliveringResource, dstBuildingId, xDst, yDst);
		removeResourceFromWarehouse(f);
		return;
	}
	// priority 3: food to granary
	dstBuildingId = Resource_getGranaryForStoringFood(0, f->x, f->y, f->resourceId,
		warehouse->distanceFromEntry, roadNetworkId, 0, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(f, FigureActionState_51_WarehousemanDeliveringResource, dstBuildingId, xDst, yDst);
		removeResourceFromWarehouse(f);
		return;
	}
	// priority 4: food to getting granary
	dstBuildingId = Resource_getGettingGranaryForStoringFood(f->x, f->y, f->resourceId,
		warehouse->distanceFromEntry, roadNetworkId, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(f, FigureActionState_51_WarehousemanDeliveringResource, dstBuildingId, xDst, yDst);
		removeResourceFromWarehouse(f);
		return;
	}
	// priority 5: resource to other warehouse
	dstBuildingId = building_warehouse_for_storing(f->buildingId, f->x, f->y, f->resourceId,
		warehouse->distanceFromEntry, roadNetworkId, 0, &xDst, &yDst);
	if (dstBuildingId) {
		if (dstBuildingId == f->buildingId) {
			f->state = FigureState_Dead;
		} else {
			setDestination(f, FigureActionState_51_WarehousemanDeliveringResource, dstBuildingId, xDst, yDst);
			removeResourceFromWarehouse(f);
		}
		return;
	}
	// priority 6: raw material to well-stocked workshop
	dstBuildingId = building_get_workshop_for_raw_material(f->x, f->y, f->resourceId,
		warehouse->distanceFromEntry, roadNetworkId, &xDst, &yDst);
	if (dstBuildingId) {
		setDestination(f, FigureActionState_51_WarehousemanDeliveringResource, dstBuildingId, xDst, yDst);
		removeResourceFromWarehouse(f);
		return;
	}
	// no destination: kill figure
	f->state = FigureState_Dead;
}

void FigureAction_warehouseman(figure *f)
{
	f->terrainUsage = FigureTerrainUsage_Roads;
	FigureActionIncreaseGraphicOffset(f, 12);
	f->cartGraphicId = 0;
	int roadNetworkId = map_road_network_get(f->gridOffset);
	
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(f);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(f);
			break;
		case FigureActionState_50_WarehousemanCreated: {
            building *b = building_get(f->buildingId);
			if (!BuildingIsInUse(b) || b->figureId != f->id) {
				f->state = FigureState_Dead;
			}
			f->waitTicks++;
			if (f->waitTicks > 2) {
				if (building_get(f->buildingId)->type == BUILDING_GRANARY) {
					determineGranarymanDestination(f, roadNetworkId);
				} else {
					determineWarehousemanDestination(f, roadNetworkId);
				}
			}
			f->graphicOffset = 0;
			break;
        }
		case FigureActionState_51_WarehousemanDeliveringResource:
			if (f->loadsSoldOrCarrying == 1) {
				f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART_MULTIPLE_FOOD) +
					8 * f->resourceId - 8 + resource_image_offset(f->resourceId, RESOURCE_IMAGE_FOOD_CART);
			} else {
				setCartGraphic(f);
			}
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DIR_FIGURE_AT_DESTINATION) {
				f->actionState = FigureActionState_52_WarehousemanAtDeliveryBuilding;
			} else if (f->direction == DIR_FIGURE_REROUTE) {
				figure_route_remove(f);
			} else if (f->direction == DIR_FIGURE_LOST) {
				f->state = FigureState_Dead;
			}
			break;
		case FigureActionState_52_WarehousemanAtDeliveryBuilding:
			f->waitTicks++;
			if (f->waitTicks > 4) {
                building *b = building_get(f->destinationBuildingId);
				switch (b->type) {
					case BUILDING_GRANARY:
						building_granary_add_resource(b, f->resourceId, 0);
						break;
					case BUILDING_BARRACKS:
						building_barracks_add_weapon(b);
						break;
					case BUILDING_WAREHOUSE:
					case BUILDING_WAREHOUSE_SPACE:
						building_warehouse_add_resource(b, f->resourceId);
						break;
					default: // workshop
						building_workshop_add_raw_material(b);
						break;
				}
				// BUG: what if warehouse/granary is full and returns false?
				f->actionState = FigureActionState_53_WarehousemanReturningEmpty;
				f->waitTicks = 0;
				f->destinationX = f->sourceX;
				f->destinationY = f->sourceY;
			}
			f->graphicOffset = 0;
			break;
		case FigureActionState_53_WarehousemanReturningEmpty:
			f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART); // empty
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DIR_FIGURE_AT_DESTINATION || f->direction == DIR_FIGURE_LOST) {
				f->state = FigureState_Dead;
			} else if (f->direction == DIR_FIGURE_REROUTE) {
				figure_route_remove(f);
			}
			break;
		case FigureActionState_54_WarehousemanGettingFood:
			f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART); // empty
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DIR_FIGURE_AT_DESTINATION) {
				f->actionState = FigureActionState_55_WarehousemanAtGranaryGettingFood;
			} else if (f->direction == DIR_FIGURE_REROUTE) {
				figure_route_remove(f);
			} else if (f->direction == DIR_FIGURE_LOST) {
				f->state = FigureState_Dead;
			}
			break;
		case FigureActionState_55_WarehousemanAtGranaryGettingFood:
			f->waitTicks++;
			if (f->waitTicks > 4) {
				int resource;
				f->loadsSoldOrCarrying = Resource_takeFoodFromGranaryForGettingDeliveryman(
					f->buildingId, f->destinationBuildingId, &resource);
				f->resourceId = resource;
				f->actionState = FigureActionState_56_WarehousemanReturningWithFood;
				f->waitTicks = 0;
				f->destinationX = f->sourceX;
				f->destinationY = f->sourceY;
				figure_route_remove(f);
			}
			f->graphicOffset = 0;
			break;
		case FigureActionState_56_WarehousemanReturningWithFood:
			// update graphic
			if (f->loadsSoldOrCarrying <= 0) {
				f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART); // empty
			} else if (f->loadsSoldOrCarrying == 1) {
				setCartGraphic(f);
			} else {
				if (f->loadsSoldOrCarrying >= 8) {
					f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART_MULTIPLE_FOOD) +
						cartResourceOffset8PlusLoadsFood[f->resourceId];
				} else {
					f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART_MULTIPLE_FOOD) +
						cartResourceOffsetMultipleLoadsFood[f->resourceId];
				}
				f->cartGraphicId += resource_image_offset(f->resourceId, RESOURCE_IMAGE_FOOD_CART);
			}
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DIR_FIGURE_AT_DESTINATION) {
				for (int i = 0; i < f->loadsSoldOrCarrying; i++) {
					building_granary_add_resource(building_get(f->buildingId), f->resourceId, 0);
				}
				f->state = FigureState_Dead;
			} else if (f->direction == DIR_FIGURE_REROUTE) {
				figure_route_remove(f);
			} else if (f->direction == DIR_FIGURE_LOST) {
				f->state = FigureState_Dead;
			}
			break;
		case FigureActionState_57_WarehousemanGettingResource:
			f->terrainUsage = FigureTerrainUsage_PreferRoads;
			f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART); // empty
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DIR_FIGURE_AT_DESTINATION) {
				f->actionState = FigureActionState_58_WarehousemanAtWarehouseGettingResource;
			} else if (f->direction == DIR_FIGURE_REROUTE) {
				figure_route_remove(f);
			} else if (f->direction == DIR_FIGURE_LOST) {
				f->state = FigureState_Dead;
			}
			break;
		case FigureActionState_58_WarehousemanAtWarehouseGettingResource:
			f->terrainUsage = FigureTerrainUsage_PreferRoads;
			f->waitTicks++;
			if (f->waitTicks > 4) {
				f->loadsSoldOrCarrying = 0;
				while (f->loadsSoldOrCarrying < 4 && 0 == building_warehouse_remove_resource(
                    building_get(f->destinationBuildingId), f->collectingItemId, 1)) {
					f->loadsSoldOrCarrying++;
				}
				f->resourceId = f->collectingItemId;
				f->actionState = FigureActionState_59_WarehousemanReturningWithResource;
				f->waitTicks = 0;
				f->destinationX = f->sourceX;
				f->destinationY = f->sourceY;
				figure_route_remove(f);
			}
			f->graphicOffset = 0;
			break;
		case FigureActionState_59_WarehousemanReturningWithResource:
			f->terrainUsage = FigureTerrainUsage_PreferRoads;
			// update graphic
			if (f->loadsSoldOrCarrying <= 0) {
				f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART); // empty
			} else if (f->loadsSoldOrCarrying == 1) {
				setCartGraphic(f);
			} else {
				if (f->resourceId == RESOURCE_WHEAT || f->resourceId == RESOURCE_VEGETABLES ||
                    f->resourceId == RESOURCE_FRUIT || f->resourceId == RESOURCE_MEAT) {
					f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART_MULTIPLE_FOOD) +
						cartResourceOffsetMultipleLoadsFood[f->resourceId];
				} else {
					f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART_MULTIPLE_RESOURCE) +
						cartResourceOffsetMultipleLoadsNonFood[f->resourceId];
				}
				f->cartGraphicId += resource_image_offset(f->resourceId, RESOURCE_IMAGE_FOOD_CART);
			}
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DIR_FIGURE_AT_DESTINATION) {
				for (int i = 0; i < f->loadsSoldOrCarrying; i++) {
					building_warehouse_add_resource(building_get(f->buildingId), f->resourceId);
				}
				f->state = FigureState_Dead;
			} else if (f->direction == DIR_FIGURE_REROUTE) {
				figure_route_remove(f);
			} else if (f->direction == DIR_FIGURE_LOST) {
				f->state = FigureState_Dead;
			}
			break;
	}
	updateGraphic(f);
}
