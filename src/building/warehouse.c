#include "warehouse.h"

#include "building/building.h"
#include "building/storage.h"
#include "game/resource.h"
#include "game/tutorial.h"
#include "graphics/image.h"
#include "map/image.h"

#include "Data/CityInfo.h"
#include "../Terrain.h"

void building_warehouse_space_set_image(building *space, int resource)
{
    int image_id;
    if (space->loadsStored <= 0) {
        image_id = image_group(GROUP_BUILDING_WAREHOUSE_STORAGE_EMPTY);
    } else {
        image_id = image_group(GROUP_BUILDING_WAREHOUSE_STORAGE_FILLED) +
            4 * (resource - 1) + resource_image_offset(resource, RESOURCE_IMAGE_STORAGE) +
                   space->loadsStored - 1;
    }
    map_image_set(space->gridOffset, image_id);
}

int building_warehouse_add_resource(building *b, int resource)
{
    if (b->id <= 0) {
        return 0;
    }
    // check building itself
    int findSpace = 0;
    if (b->subtype.warehouseResourceId && b->subtype.warehouseResourceId != resource) {
        findSpace = 1;
    } else if (b->loadsStored >= 4) {
        findSpace = 1;
    } else if (b->type == BUILDING_WAREHOUSE) {
        findSpace = 1;
    }
    if (findSpace) {
        int spaceFound = 0;
        building *space = building_main(b);
        for (int i = 0; i < 8; i++) {
            space = building_next(space);
            if (!space->id) {
                return 0;
            }
            if (!space->subtype.warehouseResourceId || space->subtype.warehouseResourceId == resource) {
                if (space->loadsStored < 4) {
                    spaceFound = 1;
                    b = space;
                    break;
                }
            }
        }
        if (!spaceFound) {
            return 0;
        }
    }
    Data_CityInfo.resourceSpaceInWarehouses[resource]--;
    Data_CityInfo.resourceStored[resource]++;
    b->subtype.warehouseResourceId = resource;
    b->loadsStored++;
    tutorial_on_add_to_warehouse();
    building_warehouse_space_set_image(b, resource);
    return 1;
}

int building_warehouse_remove_resource(building *warehouse, int resource, int amount)
{
    // returns amount still needing removal
    if (warehouse->type != BUILDING_WAREHOUSE) {
        return amount;
    }
    building *space = warehouse;
    for (int i = 0; i < 8; i++) {
        if (amount <= 0) {
            return 0;
        }
        space = building_next(space);
        if (space->id <= 0) {
            continue;
        }
        if (space->subtype.warehouseResourceId != resource || space->loadsStored <= 0) {
            continue;
        }
        if (space->loadsStored > amount) {
            Data_CityInfo.resourceSpaceInWarehouses[resource] += amount;
            Data_CityInfo.resourceStored[resource] -= amount;
            space->loadsStored -= amount;
            amount = 0;
        } else {
            Data_CityInfo.resourceSpaceInWarehouses[resource] += space->loadsStored;
            Data_CityInfo.resourceStored[resource] -= space->loadsStored;
            amount -= space->loadsStored;
            space->loadsStored = 0;
            space->subtype.warehouseResourceId = RESOURCE_NONE;
        }
        building_warehouse_space_set_image(space, resource);
    }
    return amount;
}

void building_warehouses_add_resource(int resource, int amount)
{
    int building_id = Data_CityInfo.resourceLastTargetWarehouse;
    for (int i = 1; i < MAX_BUILDINGS && amount > 0; i++) {
        building_id++;
        if (building_id >= MAX_BUILDINGS) {
            building_id = 1;
        }
        building *b = building_get(building_id);
        if (BuildingIsInUse(b) && b->type == BUILDING_WAREHOUSE) {
            Data_CityInfo.resourceLastTargetWarehouse = building_id;
            while (amount && building_warehouse_add_resource(b, resource)) {
                amount--;
            }
        }
    }
}

int building_warehouses_remove_resource(int resource, int amount)
{
    int amount_left = amount;
    int building_id = Data_CityInfo.resourceLastTargetWarehouse;
    // first go for non-getting warehouses
    for (int i = 1; i < MAX_BUILDINGS && amount_left > 0; i++) {
        building_id++;
        if (building_id >= MAX_BUILDINGS) {
            building_id = 1;
        }
        building *b = building_get(building_id);
        if (BuildingIsInUse(b) && b->type == BUILDING_WAREHOUSE) {
            if (building_storage_get(b->storage_id)->resource_state[resource] != BUILDING_STORAGE_STATE_GETTING) {
                Data_CityInfo.resourceLastTargetWarehouse = building_id;
                amount_left = building_warehouse_remove_resource(b, resource, amount_left);
            }
        }
    }
    // if that doesn't work, take it anyway
    for (int i = 1; i < MAX_BUILDINGS && amount_left > 0; i++) {
        building_id++;
        if (building_id >= MAX_BUILDINGS) {
            building_id = 1;
        }
        building *b = building_get(building_id);
        if (BuildingIsInUse(b) && b->type == BUILDING_WAREHOUSE) {
            Data_CityInfo.resourceLastTargetWarehouse = building_id;
            amount_left = building_warehouse_remove_resource(b, resource, amount_left);
        }
    }
    return amount - amount_left;
}

void building_warehouses_calculate_stocks()
{
    for (int i = 0; i < RESOURCE_MAX; i++) {
        Data_CityInfo.resourceSpaceInWarehouses[i] = 0;
        Data_CityInfo.resourceStored[i] = 0;
    }
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (BuildingIsInUse(b) && b->type == BUILDING_WAREHOUSE) {
            b->hasRoadAccess = 0;
            if (Terrain_hasRoadAccess(b->x, b->y, b->size, 0, 0)) {
                b->hasRoadAccess = 1;
            } else if (Terrain_hasRoadAccess(b->x, b->y, 3, 0, 0)) {
                b->hasRoadAccess = 2;
            }
        }
    }
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (!BuildingIsInUse(b) || b->type != BUILDING_WAREHOUSE_SPACE) {
            continue;
        }
        building *warehouse = building_main(b);
        if (warehouse->hasRoadAccess) {
            b->hasRoadAccess = warehouse->hasRoadAccess;
            if (b->subtype.warehouseResourceId) {
                int loads = b->loadsStored;
                int resource = b->subtype.warehouseResourceId;
                Data_CityInfo.resourceStored[resource] += loads;
                Data_CityInfo.resourceSpaceInWarehouses[resource] += 4 - loads;
            } else {
                Data_CityInfo.resourceSpaceInWarehouses[RESOURCE_NONE] += 4;
            }
        }
    }
}
