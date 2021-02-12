#include "tavern.h"

#include "building/distribution.h"
#include "game/resource.h"

#define INFINITE 10000
#define MAX_FOOD 600

int building_tavern_get_storage_destination(building *tavern)
{
    int allowed_inventory = INVENTORY_FLAG_NONE;
    inventory_set(&allowed_inventory, INVENTORY_MEAT);
    inventory_set(&allowed_inventory, INVENTORY_WINE);

    inventory_data *data = building_distribution_get_inventory_data(tavern, allowed_inventory, INFINITE);
    if (!data) {
        return 0;
    }
    if (data[INVENTORY_WINE].building_id && tavern->data.market.inventory[INVENTORY_WINE] < BASELINE_STOCK) {
        tavern->data.market.fetch_inventory_id = INVENTORY_WINE;
        return data[INVENTORY_WINE].building_id;
    }
    if (tavern->data.market.inventory[INVENTORY_WINE] >= BASELINE_STOCK && 
        data[INVENTORY_MEAT].building_id && tavern->data.market.inventory[INVENTORY_MEAT] < MAX_FOOD) {
        tavern->data.market.fetch_inventory_id = INVENTORY_MEAT;
        return data[INVENTORY_MEAT].building_id;
    }
    return 0;
}
