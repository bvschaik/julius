#include "supplier.h"

#include "assets/assets.h"
#include "building/building.h"
#include "building/distribution.h"
#include "building/granary.h"
#include "building/market.h"
#include "building/storage.h"
#include "building/warehouse.h"
#include "core/image.h"
#include "figure/combat.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "figuretype/wall.h"
#include "game/resource.h"
#include "map/road_access.h"
#include "map/road_network.h"

#define MAX_DISTANCE 40

int figure_supplier_create_delivery_boy(int leader_id, int first_figure_id, int type)
{
    figure *f = figure_get(first_figure_id);
    figure *boy = figure_create(type, f->x, f->y, 0);
    f = figure_get(first_figure_id);
    boy->leading_figure_id = leader_id;
    boy->collecting_item_id = f->collecting_item_id;
    // deliver to destination instead of origin
    if (f->action_state == FIGURE_ACTION_214_DESTINATION_MARS_PRIEST_CREATED) {
        boy->building_id = f->destination_building_id;
    } else {
        boy->building_id = f->building_id;
    }
    return boy->id;
}

static int take_food_from_granary(figure *f, int market_id, int granary_id)
{
    if (f->collecting_item_id < INVENTORY_MIN_FOOD || f->collecting_item_id >= INVENTORY_MAX_FOOD) {
        return 0;
    }
    int resource = resource_from_inventory(f->collecting_item_id);
    building *granary = building_get(granary_id);
    int market_units = building_get(market_id)->data.market.inventory[f->collecting_item_id];
    int max_units = 0;
    int granary_units = granary->data.granary.resource_stored[resource];
    int num_loads;
    if (building_get(market_id)->data.market.is_mess_hall) {
        max_units = MAX_FOOD_STOCKED_MESS_HALL - market_units;
    } else {
        max_units = MAX_FOOD_STOCKED_MARKET - market_units;
    }
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
    int type = FIGURE_DELIVERY_BOY;
    if (f->type == FIGURE_MESS_HALL_SUPPLIER) {
        type = FIGURE_MESS_HALL_COLLECTOR;
    } else if (f->type == FIGURE_CARAVANSERAI_SUPPLIER) {
        type = FIGURE_CARAVANSERAI_COLLECTOR;
    }
    int leader_id = f->id;
    int previous_boy = f->id;
    for (int i = 0; i < num_loads; i++) {
        previous_boy = figure_supplier_create_delivery_boy(previous_boy, leader_id, type);
    }
    return 1;
}

// Venus Grand Temple wine
static int take_resource_from_generic_building(figure *f, int building_id)
{
    building *b = building_get(building_id);
    int num_loads;
    int stored = b->loads_stored;
    if (stored < 2) {
        num_loads = stored;
    } else {
        num_loads = 2;
    }
    if (num_loads <= 0) {
        return 0;
    }
    b->loads_stored -= num_loads;

    // create delivery boys
    int priest_id = f->id;
    int boy1 = figure_supplier_create_delivery_boy(priest_id, priest_id, FIGURE_DELIVERY_BOY);
    if (num_loads > 1) {
        figure_supplier_create_delivery_boy(boy1, priest_id, FIGURE_DELIVERY_BOY);
    }
    return 1;
}

static int take_resource_from_warehouse(figure *f, int warehouse_id, int max_amount)
{
    int lighthouse_supplier = f->type == FIGURE_LIGHTHOUSE_SUPPLIER;
    int resource;
    if (lighthouse_supplier) {
        resource = f->collecting_item_id;
        if (f->collecting_item_id < RESOURCE_MIN_RAW || f->collecting_item_id >= RESOURCE_MAX_RAW) {
            return 0;
        }
    } else {
        resource = resource_from_inventory(f->collecting_item_id);
        if (f->collecting_item_id < INVENTORY_MIN_GOOD || f->collecting_item_id >= INVENTORY_MAX_GOOD) {
            return 0;
        }
    }

    building *warehouse = building_get(warehouse_id);
    if (warehouse->type != BUILDING_WAREHOUSE) {
        return take_resource_from_generic_building(f, warehouse_id);
    }
    int num_loads;
    int stored = building_warehouse_get_amount(warehouse, resource);
    if (stored < max_amount) {
        num_loads = stored;
    } else {
        num_loads = max_amount;
    }
    if (num_loads <= 0) {
        return 0;
    }
    building_warehouse_remove_resource(warehouse, resource, num_loads);

    // create delivery boys
    if (!lighthouse_supplier) {
        int supplier_id = f->id;
        int boy1 = figure_supplier_create_delivery_boy(supplier_id, supplier_id, FIGURE_DELIVERY_BOY);
        if (num_loads > 1) {
            figure_supplier_create_delivery_boy(boy1, supplier_id, FIGURE_DELIVERY_BOY);
        }
    }
    return 1;
}

static int change_market_supplier_destination(figure *f, int dst_building_id)
{
    figure_route_remove(f);
    f->destination_building_id = dst_building_id;
    building *b_dst = building_get(dst_building_id);
    map_point road;
    if (!map_has_road_access_rotation(b_dst->subtype.orientation, b_dst->x, b_dst->y, b_dst->size, &road) &&
        !map_has_road_access_rotation(b_dst->subtype.orientation, b_dst->x, b_dst->y, 3, &road)) {
        return 0;
    }
    f->action_state = FIGURE_ACTION_145_SUPPLIER_GOING_TO_STORAGE;
    f->destination_x = road.x;
    f->destination_y = road.y;
    return 1;
}

static int recalculate_market_supplier_destination(figure *f)
{
    int item = f->collecting_item_id;
    building *market = building_get(f->building_id);
    inventory_storage_info info[INVENTORY_MAX];

    int road_network = map_road_network_get(f->grid_offset);
    if (!road_network) {
        return 1;
    }
    if (!building_distribution_get_inventory_storages_for_figure(info, BUILDING_MARKET, road_network, f, MAX_DISTANCE)) {
        return 0;
    }

    if (f->building_id == info[item].building_id) {
        return 1;
    }
    if (info[item].building_id) {
        return change_market_supplier_destination(f, info[item].building_id);
    }
    int needed_inventory = building_market_get_needed_inventory(market);
    if (needed_inventory == INVENTORY_FLAG_NONE) {
        return 0;
    }
    int fetch_inventory = building_market_fetch_inventory(market, info, needed_inventory);
    if (fetch_inventory == INVENTORY_NONE) {
        return 0;
    }
    market->data.market.fetch_inventory_id = fetch_inventory;
    f->collecting_item_id = fetch_inventory;
    return change_market_supplier_destination(f, info[item].building_id);
}

void figure_supplier_action(figure *f)
{

    f->terrain_usage = TERRAIN_USAGE_ROADS_HIGHWAY;
    f->use_cross_country = 0;
    f->max_roam_length = 800;

    building *b = building_get(f->building_id);
    if (b->state != BUILDING_STATE_IN_USE ||
        (b->figure_id2 != f->id && b->figure_id != f->id && b->figure_id4 != f->id)) {
        f->state = FIGURE_STATE_DEAD;
    }
    figure_image_increase_offset(f, 12);
    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_145_SUPPLIER_GOING_TO_STORAGE:
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->wait_ticks = 0;
                int id = f->id;
                if (f->collecting_item_id > 3) {
                    int max_amount = f->type == FIGURE_LIGHTHOUSE_SUPPLIER ? 1 : 2;
                    if (!take_resource_from_warehouse(f, f->destination_building_id, max_amount)) {
                        f->state = FIGURE_STATE_DEAD;
                    }
                } else {
                    if (!take_food_from_granary(f, f->building_id, f->destination_building_id)) {
                        f->state = FIGURE_STATE_DEAD;
                    }
                }
                f = figure_get(id);
                f->action_state = FIGURE_ACTION_146_SUPPLIER_RETURNING;
                f->destination_x = f->source_x;
                f->destination_y = f->source_y;
            } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->action_state = FIGURE_ACTION_146_SUPPLIER_RETURNING;
                f->destination_x = f->source_x;
                f->destination_y = f->source_y;
                figure_route_remove(f);
            } else if (f->type == FIGURE_MARKET_SUPPLIER && f->wait_ticks++ > FIGURE_REROUTE_DESTINATION_TICKS) {
                f->wait_ticks = 0;
                if (!recalculate_market_supplier_destination(f)) {
                    f->action_state = FIGURE_ACTION_146_SUPPLIER_RETURNING;
                    f->collecting_item_id = INVENTORY_NONE;
                    f->destination_x = f->source_x;
                    f->destination_y = f->source_y;
                    figure_route_remove(f);
                }
            }
            break;
        case FIGURE_ACTION_146_SUPPLIER_RETURNING:
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION || f->direction == DIR_FIGURE_LOST) {
                if (f->direction == DIR_FIGURE_AT_DESTINATION && f->type == FIGURE_LIGHTHOUSE_SUPPLIER) {
                    building_get(f->building_id)->loads_stored += 100;
                }
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            }
            break;
    }
    if (f->type == FIGURE_MESS_HALL_SUPPLIER) {
        figure_tower_sentry_set_image(f);
    } else if (f->type == FIGURE_PRIEST_SUPPLIER) {
        figure_image_update(f, image_group(GROUP_FIGURE_PRIEST));
    } else if (f->type == FIGURE_BARKEEP_SUPPLIER) {
        int dir = figure_image_normalize_direction(f->direction < 8 ? f->direction : f->previous_tile_direction);
        if (f->action_state == FIGURE_ACTION_149_CORPSE) {
            f->image_id = assets_get_image_id("Entertainment", "Barkeep Death 01") +
                figure_image_corpse_offset(f);
        } else {
            f->image_id = assets_get_image_id("Entertainment", "Barkeep NE 01") +
                dir * 12 + f->image_offset;
        }
    } else if (f->type == FIGURE_LIGHTHOUSE_SUPPLIER || f->type == FIGURE_CARAVANSERAI_SUPPLIER) {
        int dir = figure_image_normalize_direction(f->direction < 8 ? f->direction : f->previous_tile_direction);
        if (f->action_state == FIGURE_ACTION_149_CORPSE) {
            f->image_id = assets_get_image_id("Logistics", "Slave death 01") +
                figure_image_corpse_offset(f);
        } else {
            f->image_id = assets_get_image_id("Logistics", "Slave NE 01") +
                dir * 12 + f->image_offset;
        }
    } else {
        figure_image_update(f, image_group(GROUP_FIGURE_MARKET_LADY));
    }
}

void figure_delivery_boy_action(figure *f)
{
    f->is_ghost = 0;
    f->terrain_usage = TERRAIN_USAGE_ROADS_HIGHWAY;
    figure_image_increase_offset(f, 12);
    f->cart_image_id = 0;

    figure *leader = figure_get(f->leading_figure_id);
    if (f->leading_figure_id <= 0 || leader->action_state == FIGURE_ACTION_149_CORPSE) {
        f->state = FIGURE_STATE_DEAD;
    } else {
        if (leader->state == FIGURE_STATE_ALIVE) {
            if (leader->type == FIGURE_MARKET_SUPPLIER || leader->type == FIGURE_DELIVERY_BOY ||
                leader->type == FIGURE_MESS_HALL_SUPPLIER || leader->type == FIGURE_MESS_HALL_COLLECTOR ||
                leader->type == FIGURE_PRIEST_SUPPLIER || leader->type == FIGURE_PRIEST ||
                leader->type == FIGURE_BARKEEP_SUPPLIER || leader->type == FIGURE_CARAVANSERAI_SUPPLIER ||
                leader->type == FIGURE_CARAVANSERAI_COLLECTOR) {
                figure_movement_follow_ticks(f, 1);
            } else {
                f->state = FIGURE_STATE_DEAD;
            }
        } else { // leader arrived at market, drop resource at market
            building_get(f->building_id)->data.market.inventory[f->collecting_item_id] += 100;
            f->state = FIGURE_STATE_DEAD;
        }
    }
    if (leader->is_ghost && !leader->height_adjusted_ticks) {
        f->is_ghost = 1;
    }
    int dir = figure_image_normalize_direction(f->direction < 8 ? f->direction : f->previous_tile_direction);

    if (f->type == FIGURE_MESS_HALL_COLLECTOR) {
        if (f->action_state == FIGURE_ACTION_149_CORPSE) {
            f->image_id = assets_get_image_id("Military", "M Hall death 01") +
                figure_image_corpse_offset(f);
        } else {
            f->image_id = assets_get_image_id("Military", "M Hall NE 01") +
                dir * 12 + f->image_offset;
        }
    } else if (f->type == FIGURE_CARAVANSERAI_COLLECTOR) {
        if (f->action_state == FIGURE_ACTION_149_CORPSE) {
            f->image_id = assets_get_image_id("Logistics", "Slave death 01") + figure_image_corpse_offset(f);
        } else {
            f->image_id = assets_get_image_id("Logistics", "Slave NE 01")
                + dir * 12 + f->image_offset;
        }
    } else {
        if (f->action_state == FIGURE_ACTION_149_CORPSE) {
            f->image_id = image_group(GROUP_FIGURE_DELIVERY_BOY) + 96 +
                figure_image_corpse_offset(f);
        } else {
            f->image_id = image_group(GROUP_FIGURE_DELIVERY_BOY) +
                dir + 8 * f->image_offset;
        }
    }
}

void figure_fort_supplier_action(figure *f)
{
    f->is_ghost = 0;
    f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS_HIGHWAY;
    figure_image_increase_offset(f, 12);

    building *b = building_get(f->building_id);
    if (!b || b->state != BUILDING_STATE_IN_USE || b->type != BUILDING_MESS_HALL) {
        f->state = FIGURE_STATE_DEAD;
    }

    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_236_SUPPLY_POST_GOING_TO_FORT:
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_237_SUPPLY_POST_RETURNING_FROM_FORT;
                f->destination_x = f->source_x;
                f->destination_y = f->source_y;
                f->wait_ticks = 20;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_237_SUPPLY_POST_RETURNING_FROM_FORT:
            if (f->wait_ticks) {
                f->wait_ticks--;
            } else {
                figure_movement_move_ticks(f, 1);
                if (f->direction == DIR_FIGURE_REROUTE) {
                    figure_route_remove(f);
                } else if (f->direction == DIR_FIGURE_AT_DESTINATION || f->direction == DIR_FIGURE_LOST) {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            break;
    }

    int dir = figure_image_normalize_direction(f->direction < 8 ? f->direction : f->previous_tile_direction);
    if (f->action_state == FIGURE_ACTION_149_CORPSE) {
        f->image_id = assets_get_image_id("Military", "M Hall death 01") +
            figure_image_corpse_offset(f);
    } else {
        f->image_id = assets_get_image_id("Military", "M Hall NE 01") +
            dir * 12 + f->image_offset;
    }
}
