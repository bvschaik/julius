#include "depot.h"

#include "building/granary.h"
#include "building/industry.h"
#include "building/storage.h"
#include "building/warehouse.h"
#include "city/health.h"
#include "city/resource.h"
#include "core/calc.h"
#include "core/config.h"
#include "core/image.h"
#include "figure/combat.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "game/resource.h"
#include "map/road_access.h"
#include "map/road_network.h"
#include "map/routing.h"
#include "map/routing_terrain.h"

#define DEPOT_CART_PUSHER_SPEED 1

#define DEPOT_CART_PUSHER_FOOD_CAPACITY 16
#define DEPOT_CART_PUSHER_OTHER_CAPACITY 4

#define DEPOT_CART_REROUTE_DELAY 10
#define DEPOT_CART_LOAD_OFFLOAD_DELAY 10

static int cartpusher_carries_food(figure *f)
{
    return resource_is_food(f->resource_id);
}

static void set_cart_graphic(figure *f)
{
    int carried = f->loads_sold_or_carrying;
    if (carried == 0 || f->resource_id == RESOURCE_NONE) {
        f->cart_image_id = image_group(GROUP_FIGURE_CARTPUSHER_CART);
    } else if (carried == 1) {
        f->cart_image_id = resource_get_data(f->resource_id)->image.cart.single_load;
    } else if (cartpusher_carries_food(f) && carried >= 8) {
        f->cart_image_id = resource_get_data(f->resource_id)->image.cart.eight_loads;        
    } else {
        f->cart_image_id = resource_get_data(f->resource_id)->image.cart.multiple_loads;
    }
}

static void update_image(figure *f)
{
    int dir = figure_image_normalize_direction(
        f->direction < 8 ? f->direction : f->previous_tile_direction);

    if (f->action_state == FIGURE_ACTION_149_CORPSE) {
        f->image_id = image_group(GROUP_FIGURE_MIGRANT) + figure_image_corpse_offset(f) + 96;
        f->cart_image_id = 0;
    } else {
        f->image_id = image_group(GROUP_FIGURE_MIGRANT) + dir + 8 * f->image_offset;
    }
    if (f->cart_image_id) {
        f->cart_image_id += dir;
        figure_image_set_cart_offset(f, dir);
        if (f->loads_sold_or_carrying >= 8) {
            f->y_offset_cart -= 40;
        }
    }
}

static int get_storage_road_access(building *b, map_point *point)
{
    if (b->type == BUILDING_GRANARY) {
        map_point_store_result(b->x + 1, b->y + 1, point);
        return 1;
    } else if (b->type == BUILDING_WAREHOUSE) {
        if (b->has_road_access == 1) {
            map_point_store_result(b->x, b->y, point);
            return 1;
        } else {
            return map_has_road_access_rotation(b->subtype.orientation, b->x, b->y, 3, point);
        }
    } else {
        point->x = b->road_access_x;
        point->y = b->road_access_y;
        return b->has_road_access;
    }
}

static int is_order_condition_satisfied(const building *depot, const order *current_order)
{
    if (!current_order->src_storage_id || !current_order->dst_storage_id || !current_order->resource_type) {
        return 0;
    }

    if (current_order->condition.condition_type == ORDER_CONDITION_NEVER) {
        return 0;
    }

    building *src = building_get(current_order->src_storage_id);
    building *dst = building_get(current_order->dst_storage_id);
    if (!src || !dst) {
        return 0;
    }

    if (src->state != BUILDING_STATE_IN_USE || dst->state != BUILDING_STATE_IN_USE) {
        return 0;
    }

    if (src->type != BUILDING_GRANARY && src->type != BUILDING_WAREHOUSE) {
        return 0;
    }
    if (dst->type != BUILDING_GRANARY && dst->type != BUILDING_WAREHOUSE) {
        return 0;
    }

    int src_amount = src->type == BUILDING_GRANARY ?
        building_granary_resource_amount(current_order->resource_type, src) / RESOURCE_ONE_LOAD :
        building_warehouse_get_amount(src, current_order->resource_type);
    int dst_amount = dst->type == BUILDING_GRANARY ?
        building_granary_resource_amount(current_order->resource_type, dst) / RESOURCE_ONE_LOAD :
        building_warehouse_get_amount(dst, current_order->resource_type);
    if (src_amount == 0) {
        return 0;
    }

    switch (current_order->condition.condition_type) {
        case ORDER_CONDITION_SOURCE_HAS_MORE_THAN:
            return src_amount >= current_order->condition.threshold;
        case ORDER_CONDITION_DESTINATION_HAS_LESS_THAN:
            return dst_amount < current_order->condition.threshold;
        default:
            return 1;
    }
}

static int is_current_state_still_valid_for_order(const figure *f, const order *current_order)
{
    building *destination = building_get(f->destination_building_id);
    if (destination->state != BUILDING_STATE_IN_USE) {
        return 0;
    }

    switch (f->action_state) {
        case FIGURE_ACTION_239_DEPOT_CART_PUSHER_HEADING_TO_SOURCE:
        case FIGURE_ACTION_240_DEPOT_CART_PUSHER_AT_SOURCE:
        {
            if (f->destination_building_id == current_order->src_storage_id) {
                return 1;
            } else {
                return 0;
            }
            break;
        }
        case FIGURE_ACTION_241_DEPOT_CART_HEADING_TO_DESTINATION:
        case FIGURE_ACTION_242_DEPOT_CART_PUSHER_AT_DESTINATION:
        {
            if (f->resource_id == current_order->resource_type
                && f->destination_building_id == current_order->dst_storage_id
                && f->loads_sold_or_carrying > 0) {
                return 1;
            } else {
                return 0;
            }
            break;
        }
    }
    return 1;
}

static void dump_goods(figure *f)
{
    if (f->loads_sold_or_carrying > 0) {
        // TODO maybe add a dump goods animation?
        f->loads_sold_or_carrying = 0;
        f->resource_id = RESOURCE_NONE;
        set_cart_graphic(f);
    }
}

static int storage_remove_resource(building *b, int resource, int amount)
{
    if (b->type == BUILDING_GRANARY) {
        return building_granary_try_fullload_remove_resource(b, resource, amount);
    } else if (b->type == BUILDING_WAREHOUSE) {
        return building_warehouse_try_remove_resource(b, resource, amount);
    } else {
        return 0;
    }
}

static int storage_add_resource(building *b, int resource, int amount)
{
    if (b->type == BUILDING_GRANARY) {
        while (amount > 0) {
            if (!building_granary_add_resource(b, resource, 0)) {
                return amount;
            }
            amount--;
        }
    } else if (b->type == BUILDING_WAREHOUSE) {
        while (amount > 0) {
            if (!building_warehouse_add_resource(b, resource)) {
                return amount;
            }
            amount--;
        }
    }
    return amount;
}

void figure_depot_cartpusher_action(figure *f)
{
    figure_image_increase_offset(f, 12);
    f->cart_image_id = 0;
    int speed_factor = DEPOT_CART_PUSHER_SPEED;
    int percentage_speed = 0;
    f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS_HIGHWAY;
    building *b = building_get(f->building_id);

    if (b->state != BUILDING_STATE_IN_USE) {
        f->state = FIGURE_STATE_DEAD;
        update_image(f);
        return;
    }

    int is_linked = 0;
    for (int i = 0; i < 3; i++) {
        if (b->data.distribution.cartpusher_ids[i] == f->id) {
            is_linked = 1;
        }
    }
    if (!is_linked) {
        f->state = FIGURE_STATE_DEAD;
        update_image(f);
        return;
    }

    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_238_DEPOT_CART_PUSHER_INITIAL:
            set_cart_graphic(f);
            if (!map_routing_citizen_is_passable(f->grid_offset)) {
                f->state = FIGURE_STATE_DEAD;
            }

            if (is_order_condition_satisfied(b, &b->data.depot.current_order)) {
                building *src = building_get(b->data.depot.current_order.src_storage_id);
                map_point road_access;
                get_storage_road_access(src, &road_access);
                f->action_state = FIGURE_ACTION_239_DEPOT_CART_PUSHER_HEADING_TO_SOURCE;
                f->wait_ticks = DEPOT_CART_REROUTE_DELAY + 1;
                f->destination_building_id = b->data.depot.current_order.src_storage_id;
                f->destination_x = road_access.x;
                f->destination_y = road_access.y;
            } else {
                f->state = FIGURE_STATE_DEAD;
            }

            f->image_offset = 0;
            break;
        case FIGURE_ACTION_239_DEPOT_CART_PUSHER_HEADING_TO_SOURCE:
            set_cart_graphic(f);
            if (f->wait_ticks > DEPOT_CART_REROUTE_DELAY) {
                figure_movement_move_ticks_with_percentage(f, speed_factor, percentage_speed);
                if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                    f->action_state = FIGURE_ACTION_240_DEPOT_CART_PUSHER_AT_SOURCE;
                    f->wait_ticks = 0;
                } else if (f->direction == DIR_FIGURE_LOST) {
                    f->action_state = FIGURE_ACTION_244_DEPOT_CART_PUSHER_CANCEL_ORDER;
                    f->wait_ticks = 0;
                } else if (f->direction == DIR_FIGURE_REROUTE) {
                    figure_route_remove(f);
                    f->wait_ticks = 0;
                }
            } else {
                f->wait_ticks++;
            }
            break;
        case FIGURE_ACTION_240_DEPOT_CART_PUSHER_AT_SOURCE:
            set_cart_graphic(f);
            f->wait_ticks++;
            if (f->wait_ticks > DEPOT_CART_LOAD_OFFLOAD_DELAY) {
                building *src = building_get(b->data.depot.current_order.src_storage_id);

                // TODO upgradable?
                int capacity = resource_is_food(b->data.depot.current_order.resource_type) ? DEPOT_CART_PUSHER_FOOD_CAPACITY : DEPOT_CART_PUSHER_OTHER_CAPACITY;
                int amount_loaded = storage_remove_resource(src, b->data.depot.current_order.resource_type, capacity);
                if (amount_loaded > 0) {
                    city_health_dispatch_sickness(f);
                    f->resource_id = b->data.depot.current_order.resource_type;
                    f->loads_sold_or_carrying = amount_loaded;

                    building *dst = building_get(b->data.depot.current_order.dst_storage_id);
                    map_point road_access;
                    get_storage_road_access(dst, &road_access);
                    f->action_state = FIGURE_ACTION_241_DEPOT_CART_HEADING_TO_DESTINATION;
                    f->wait_ticks = DEPOT_CART_REROUTE_DELAY + 1;
                    f->destination_building_id = b->data.depot.current_order.dst_storage_id;
                    f->destination_x = road_access.x;
                    f->destination_y = road_access.y;
                    figure_route_remove(f);
                }
            }
            f->image_offset = 0;
            break;
        case FIGURE_ACTION_241_DEPOT_CART_HEADING_TO_DESTINATION:
            set_cart_graphic(f);
            if (f->wait_ticks > DEPOT_CART_REROUTE_DELAY) {
                figure_movement_move_ticks_with_percentage(f, speed_factor, percentage_speed);
                if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                    f->action_state = FIGURE_ACTION_242_DEPOT_CART_PUSHER_AT_DESTINATION;
                    f->wait_ticks = 0;
                } else if (f->direction == DIR_FIGURE_LOST) {
                    f->action_state = DIR_FIGURE_REROUTE;
                    f->wait_ticks = 0;
                } else if (f->direction == DIR_FIGURE_REROUTE) {
                    figure_route_remove(f);
                    f->wait_ticks = 0;
                }
            } else {
                f->wait_ticks++;
            }
            break;
        case FIGURE_ACTION_242_DEPOT_CART_PUSHER_AT_DESTINATION:
            set_cart_graphic(f);
            f->wait_ticks++;
            if (f->wait_ticks > DEPOT_CART_LOAD_OFFLOAD_DELAY) {
                building *dst = building_get(b->data.depot.current_order.dst_storage_id);

                f->loads_sold_or_carrying = storage_add_resource(dst, f->resource_id, f->loads_sold_or_carrying);
                if (f->loads_sold_or_carrying) {
                    // loads remaining
                    set_cart_graphic(f);
                } else {
                    city_health_dispatch_sickness(f);
                    f->action_state = FIGURE_ACTION_243_DEPOT_CART_PUSHER_RETURNING;
                    f->loads_sold_or_carrying = 0;
                    f->resource_id = RESOURCE_NONE;
                    f->destination_building_id = f->building_id;
                    f->destination_x = b->road_access_x;
                    f->destination_y = b->road_access_y;
                    figure_route_remove(f);
                }
                f->wait_ticks = 0;
            }
            break;
        case FIGURE_ACTION_243_DEPOT_CART_PUSHER_RETURNING:
            set_cart_graphic(f);
            figure_movement_move_ticks_with_percentage(f, speed_factor, percentage_speed);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_238_DEPOT_CART_PUSHER_INITIAL;
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
                f->wait_ticks = 0;
            }
            break;
        case FIGURE_ACTION_244_DEPOT_CART_PUSHER_CANCEL_ORDER:
            dump_goods(f);
            f->action_state = FIGURE_ACTION_243_DEPOT_CART_PUSHER_RETURNING;
            f->destination_building_id = f->building_id;
            f->destination_x = b->road_access_x;
            f->destination_y = b->road_access_y;
            figure_route_remove(f);
            break;
    }

    if (!is_current_state_still_valid_for_order(f, &b->data.depot.current_order)) {
        f->action_state = FIGURE_ACTION_244_DEPOT_CART_PUSHER_CANCEL_ORDER;
    }

    update_image(f);
}

void figure_depot_recall(figure *f)
{
    f->action_state = FIGURE_ACTION_244_DEPOT_CART_PUSHER_CANCEL_ORDER;
}
