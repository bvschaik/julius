#include "granary.h"

#include "building/destruction.h"
#include "building/model.h"
#include "building/storage.h"
#include "building/warehouse.h"
#include "city/finance.h"
#include "city/map.h"
#include "city/message.h"
#include "city/resource.h"
#include "core/calc.h"
#include "core/config.h"
#include "empire/trade_prices.h"
#include "figure/figure.h"
#include "map/road_access.h"
#include "map/routing_terrain.h"
#include "scenario/property.h"
#include "sound/effect.h"

#define MAX_GRANARIES 100
#define UNITS_PER_LOAD 100
#define CURSE_LOADS 16
#define INFINITE 10000

static struct {
    int building_ids[MAX_GRANARIES];
    int num_items;
    int total_storage[RESOURCE_MAX_FOOD];
} non_getting_granaries;

static int get_amount(building *granary, int resource)
{
    if (!resource_is_food(resource)) {
        return 0;
    }
    if (granary->type != BUILDING_GRANARY) {
        return 0;
    }
    return granary->resources[resource];
}

int building_granary_is_accepting(int resource, building *b)
{
    const building_storage *s = building_storage_get(b->storage_id);
    int amount = get_amount(b, resource);
    if (!b->has_plague &&
        ((s->resource_state[resource] == BUILDING_STORAGE_STATE_ACCEPTING) ||
        (s->resource_state[resource] == BUILDING_STORAGE_STATE_ACCEPTING_3QUARTERS && amount < THREEQUARTERS_GRANARY) ||
        (s->resource_state[resource] == BUILDING_STORAGE_STATE_ACCEPTING_HALF && amount < HALF_GRANARY) ||
        (s->resource_state[resource] == BUILDING_STORAGE_STATE_ACCEPTING_QUARTER && amount < QUARTER_GRANARY))) {
        return 1;
    } else {
        return 0;
    }
}

int building_granary_is_getting(int resource, building *b)
{
    const building_storage *s = building_storage_get(b->storage_id);
    int amount = get_amount(b, resource);
    return !b->has_plague &&
        ((s->resource_state[resource] == BUILDING_STORAGE_STATE_GETTING) ||
        (s->resource_state[resource] == BUILDING_STORAGE_STATE_GETTING_3QUARTERS && amount < THREEQUARTERS_GRANARY) ||
        (s->resource_state[resource] == BUILDING_STORAGE_STATE_GETTING_HALF && amount < HALF_GRANARY) ||
        (s->resource_state[resource] == BUILDING_STORAGE_STATE_GETTING_QUARTER && amount < QUARTER_GRANARY));
}

int building_granary_is_gettable(int resource, building *b)
{
    const building_storage *s = building_storage_get(b->storage_id);
    return !b->has_plague &&
        ((s->resource_state[resource] == BUILDING_STORAGE_STATE_GETTING) ||
        (s->resource_state[resource] == BUILDING_STORAGE_STATE_GETTING_3QUARTERS) ||
        (s->resource_state[resource] == BUILDING_STORAGE_STATE_GETTING_HALF) ||
        (s->resource_state[resource] == BUILDING_STORAGE_STATE_GETTING_QUARTER));
}

int building_granary_is_not_accepting(int resource, building *b)
{
    return !(building_granary_is_accepting(resource, b) || building_granary_is_getting(resource, b));
}

int building_granary_is_full(int resource, building *b)
{
    return b->resources[RESOURCE_NONE] <= 0;
}

int building_granary_resource_amount(int resource, building *b)
{
    return b->resources[resource];
}

int building_granary_add_import(building *granary, int resource, int land_trader)
{
    if (!building_granary_add_resource(granary, resource, 0)) {
        return 0;
    }
    int price = trade_price_buy(resource, land_trader);
    city_finance_process_import(price);
    return 1;
}

int building_granary_remove_export(building *granary, int resource, int land_trader)
{
    if (building_granary_remove_resource(granary, resource, RESOURCE_ONE_LOAD) == RESOURCE_ONE_LOAD) {
        return 0;
    }
    int price = trade_price_sell(resource, land_trader);
    city_finance_process_export(price);
    return 1;
}

int building_granary_add_resource(building *granary, int resource, int is_produced)
{
    if (granary->id <= 0) {
        return 1;
    }
    if (!resource_is_food(resource)) {
        return 0;
    }
    if (granary->type != BUILDING_GRANARY) {
        return 0;
    }
    if (granary->resources[RESOURCE_NONE] <= 0) {
        return 0; // no space
    }
    if (building_granary_is_not_accepting(resource, granary)) {
        return 0;
    }
    if (is_produced) {
        city_resource_add_produced_to_granary(RESOURCE_ONE_LOAD);
    }
    city_resource_add_to_granary(resource, RESOURCE_ONE_LOAD);
    if (granary->resources[RESOURCE_NONE] <= RESOURCE_ONE_LOAD) {
        granary->resources[resource] += granary->resources[RESOURCE_NONE];
        granary->resources[RESOURCE_NONE] = 0;
    } else {
        granary->resources[resource] += RESOURCE_ONE_LOAD;
        granary->resources[RESOURCE_NONE] -= RESOURCE_ONE_LOAD;
    }
    return 1;
}

int building_granary_remove_resource(building *granary, int resource, int amount)
{
    if (amount <= 0) {
        return 0;
    }
    if (granary->has_plague) {
        return 0;
    }

    int removed;
    if (granary->resources[resource] >= amount) {
        removed = amount;
    } else {
        removed = granary->resources[resource];
    }
    city_resource_remove_from_granary(resource, removed);
    granary->resources[resource] -= removed;
    granary->resources[RESOURCE_NONE] += removed;
    return amount - removed;
}

int building_granaries_remove_resource(int resource, int amount)
{
    // first go for non-getting granaries
    for (building *b = building_first_of_type(BUILDING_GRANARY); b && amount; b = b->next_of_type) {
        if (b->state == BUILDING_STATE_IN_USE) {
            if (!building_granary_is_getting(resource, b)) {
                amount = building_granary_remove_resource(b, resource, amount);
            }
        }
    }
    // if that doesn't work, take it anyway
    for (building *b = building_first_of_type(BUILDING_GRANARY); b && amount; b = b->next_of_type) {
        if (b->state == BUILDING_STATE_IN_USE) {
            amount = building_granary_remove_resource(b, resource, amount);
        }
    }
    return amount;
}

int building_granaries_send_resources_to_rome(int resource, int amount)
{
    // first go for non-getting granaries
    for (building *b = building_first_of_type(BUILDING_GRANARY); b && amount; b = b->next_of_type) {
        if (b->state == BUILDING_STATE_IN_USE) {
            if (!building_granary_is_getting(resource, b)) {
                int remaining = building_granary_remove_resource(b, resource, amount);
                if (remaining < amount) {
                    int loads = amount - remaining;
                    amount = remaining;
                    map_point road;
                    if (map_has_road_access(b->x, b->y, b->size, &road)) {
                        figure *f = figure_create(FIGURE_CART_PUSHER, road.x, road.y, DIR_4_BOTTOM);
                        f->action_state = FIGURE_ACTION_234_CARTPUSHER_GOING_TO_ROME_CREATED;
                        f->resource_id = resource;
                        f->loads_sold_or_carrying = loads / UNITS_PER_LOAD;
                        f->building_id = b->id;
                    }
                }
            }
        }
    }
    // if that doesn't work, take it anyway
    for (building *b = building_first_of_type(BUILDING_GRANARY); b && amount; b = b->next_of_type) {
        if (b->state == BUILDING_STATE_IN_USE) {
            int remaining = building_granary_remove_resource(b, resource, amount);
            if (remaining < amount) {
                int loads = amount - remaining;
                amount = remaining;
                map_point road;
                if (map_has_road_access(b->x, b->y, b->size, &road)) {
                    figure *f = figure_create(FIGURE_CART_PUSHER, road.x, road.y, DIR_4_BOTTOM);
                    f->action_state = FIGURE_ACTION_234_CARTPUSHER_GOING_TO_ROME_CREATED;
                    f->resource_id = resource;
                    f->loads_sold_or_carrying = loads / UNITS_PER_LOAD;
                    f->building_id = b->id;
                }
            }
        }
    }
    return amount;
}

static int maximum_receptible_amount(int resource, building *b)
{
    if (b->has_plague) {
        return 0;
    }
    int stored_amount = b->resources[resource];
    int max_amount;
    switch (building_storage_get(b->storage_id)->resource_state[resource]) {
        case BUILDING_STORAGE_STATE_GETTING:
            max_amount = FULL_GRANARY;
            break;
        case BUILDING_STORAGE_STATE_GETTING_3QUARTERS:
            max_amount = THREEQUARTERS_GRANARY;
            break;
        case BUILDING_STORAGE_STATE_GETTING_HALF:
            max_amount = HALF_GRANARY;
            break;
        case BUILDING_STORAGE_STATE_GETTING_QUARTER:
            max_amount = QUARTER_GRANARY;
            break;
        default:
            return 0;
    }
    return (max_amount > stored_amount) ? (max_amount - stored_amount) : 0;
}

int building_granary_remove_for_getting_deliveryman(building *src, building *dst, int *resource)
{
    int max_amount = 0;
    int max_resource = 0;

    for (resource_type food = RESOURCE_MIN_FOOD; food < RESOURCE_MAX_FOOD; food++) {
        if (building_granary_is_getting(food, dst) && !building_granary_is_gettable(food, src)) {
            if (src->resources[food] > max_amount) {
                max_amount = src->resources[food];
                max_resource = food;
            }
        }
    }

    if (config_get(CONFIG_GP_CH_GRANARIES_GET_DOUBLE)) {
        if (max_amount > 1600) {
            max_amount = 1600;
        }
    } else {
        if (max_amount > 800) {
            max_amount = 800;
        }
    }
    if (max_amount > dst->resources[RESOURCE_NONE]) {
        max_amount = dst->resources[RESOURCE_NONE];
    }
    const int receptible_amount = maximum_receptible_amount(max_resource, dst);
    if (max_amount > receptible_amount) {
        max_amount = receptible_amount;
    }
    building_granary_remove_resource(src, max_resource, max_amount);
    *resource = max_resource;
    return max_amount / UNITS_PER_LOAD;
}

int building_granary_determine_worker_task(building *granary)
{
    int pct_workers = calc_percentage(granary->num_workers, model_get_building(granary->type)->laborers);
    if (pct_workers < 50) {
        return GRANARY_TASK_NONE;
    }
    const building_storage *s = building_storage_get(granary->storage_id);
    if (s->empty_all) {
        // bring food to another granary
        for (int i = RESOURCE_MIN_FOOD; i < RESOURCE_MAX_FOOD; i++) {
            if (granary->resources[i]) {
                return i;
            }
        }
        return GRANARY_TASK_NONE;
    }
    if (granary->resources[RESOURCE_NONE] <= 0) {
        return GRANARY_TASK_NONE; // granary full, nothing to get
    }

    for (resource_type food = RESOURCE_MIN_FOOD; food < RESOURCE_MAX_FOOD; food++) {
        if (building_granary_is_getting(food, granary) &&
            non_getting_granaries.total_storage[food] > RESOURCE_ONE_LOAD) {
            return GRANARY_TASK_GETTING;
        }
    }
    return GRANARY_TASK_NONE;
}

void building_granaries_calculate_stocks(void)
{
    non_getting_granaries.num_items = 0;
    for (int i = 0; i < MAX_GRANARIES; i++) {
        non_getting_granaries.building_ids[i] = 0;
    }
    for (int i = 0; i < RESOURCE_MAX_FOOD; i++) {
        non_getting_granaries.total_storage[i] = 0;     
    }

    for (building *b = building_first_of_type(BUILDING_GRANARY); b; b = b->next_of_type) {
        if (b->state != BUILDING_STATE_IN_USE || !b->has_road_access ||
            b->distance_from_entry <= 0 || b->has_plague) {
            continue;
        }
        int total_non_getting = 0;

        for (resource_type food = RESOURCE_MIN_FOOD; food < RESOURCE_MAX_FOOD; food++) {
            if (!building_granary_is_gettable(food, b)) {
                total_non_getting += b->resources[food];
                non_getting_granaries.total_storage[food] += b->resources[food];
            }
        }
        if (total_non_getting > RESOURCE_ONE_LOAD) {
            non_getting_granaries.building_ids[non_getting_granaries.num_items] = b->id;
            if (non_getting_granaries.num_items < MAX_GRANARIES - 2) {
                non_getting_granaries.num_items++;
            }
        }
    }
}

int building_granary_accepts_storage(building *b, int resource, int *understaffed)
{
    if (b->state != BUILDING_STATE_IN_USE || b->type != BUILDING_GRANARY ||
        !b->has_road_access || b->distance_from_entry <= 0 || b->has_plague) {
        return 0;
    }
    int pct_workers = calc_percentage(b->num_workers, model_get_building(b->type)->laborers);
    if (pct_workers < 100) {
        if (understaffed) {
            *understaffed += 1;
        }
        return 0;
    }
    const building_storage *s = building_storage_get(b->storage_id);
    if (building_granary_is_not_accepting(resource, b) || s->empty_all) {
        return 0;
    }

    if (config_get(CONFIG_GP_CH_DELIVER_ONLY_TO_ACCEPTING_GRANARIES)) {
        if (building_granary_is_getting(resource, b)) {
            return 0;
        }
    }
    return b->resources[RESOURCE_NONE] >= RESOURCE_ONE_LOAD;
}

int building_granary_for_storing(int x, int y, int resource, int road_network_id,
    int force_on_stockpile, int *understaffed, map_point *dst)
{
    if (scenario_property_rome_supplies_wheat()) {
        return 0;
    }
    if (!resource_is_food(resource)) {
        return 0;
    }
    if (city_resource_is_stockpiled(resource) && !force_on_stockpile) {
        return 0;
    }
    int min_dist = INFINITE;
    int min_building_id = 0;
    for (building *b = building_first_of_type(BUILDING_GRANARY); b; b = b->next_of_type) {
        if (b->road_network_id != road_network_id ||
            !building_granary_accepts_storage(b, resource, understaffed)) {
            continue;
        }
        // there is room
        int dist = calc_maximum_distance(b->x + 1, b->y + 1, x, y);
        if (dist < min_dist) {
            min_dist = dist;
            min_building_id = b->id;
        }
    }
    // deliver to center of granary
    building *min = building_get(min_building_id);
    map_point_store_result(min->x + 1, min->y + 1, dst);
    return min_building_id;
}

int building_getting_granary_for_storing(int x, int y, int resource, int road_network_id, map_point *dst)
{
    if (scenario_property_rome_supplies_wheat()) {
        return 0;
    }
    if (!resource_is_food(resource)) {
        return 0;
    }
    
    if (city_resource_is_stockpiled(resource)) {
        return 0;
    }
    int min_dist = INFINITE;
    int min_building_id = 0;
    for (building *b = building_first_of_type(BUILDING_GRANARY); b; b = b->next_of_type) {
        if (b->state != BUILDING_STATE_IN_USE || b->has_plague) {
            continue;
        }
        if (!b->has_road_access || b->distance_from_entry <= 0 || b->road_network_id != road_network_id) {
            continue;
        }
        int pct_workers = calc_percentage(b->num_workers, model_get_building(b->type)->laborers);
        if (pct_workers < 100) {
            continue;
        }
        const building_storage *s = building_storage_get(b->storage_id);
        if (!building_granary_is_getting(resource, b) || s->empty_all) {
            continue;
        }
        if (b->resources[RESOURCE_NONE] > RESOURCE_ONE_LOAD) {
            // there is room
            int dist = calc_maximum_distance(b->x + 1, b->y + 1, x, y);
            if (dist < min_dist) {
                min_dist = dist;
                min_building_id = b->id;
            }
        }
    }
    building *min = building_get(min_building_id);
    map_point_store_result(min->x + 1, min->y + 1, dst);
    return min_building_id;
}

int building_granary_amount_can_get_from(building *destination, building *origin)
{
    int amount_gettable = 0;
    for (resource_type food = RESOURCE_MIN_FOOD; food < RESOURCE_MAX_FOOD; food++) {
        if (building_granary_is_getting(food, origin) &&
            !building_granary_is_gettable(food, destination)) {
            amount_gettable += destination->resources[food];
        }
    }
    return amount_gettable;
}

int building_granary_for_getting(building *src, map_point *dst, int min_amount)
{
    const building_storage *s_src = building_storage_get(src->storage_id);
    if (s_src->empty_all) {
        return 0;
    }
    if (scenario_property_rome_supplies_wheat()) {
        return 0;
    }
    int getting_something = 0;
    for (resource_type food = RESOURCE_MIN_FOOD; food < RESOURCE_MAX_FOOD && !getting_something; food++) {
        getting_something |= building_granary_is_getting(food, src);
    }
    if (!getting_something) {
        return 0;
    }
    int min_dist = INFINITE;
    int min_building_id = 0;
    for (int i = 0; i < non_getting_granaries.num_items; i++) {
        building *b = building_get(non_getting_granaries.building_ids[i]);
        if (!config_get(CONFIG_GP_CH_GETTING_GRANARIES_GO_OFFROAD)) {
            if (b->road_network_id != src->road_network_id) {
                continue;
            }
        }
        if (building_granary_amount_can_get_from(b, src) >= min_amount) {
            int dist = calc_maximum_distance(b->x + 1, b->y + 1, src->x + 1, src->y + 1);
            if (dist < min_dist) {
                min_dist = dist;
                min_building_id = b->id;
            }
        }
    }
    building *min = building_get(min_building_id);
    map_point_store_result(min->x + 1, min->y + 1, dst);
    return min_building_id;
}

void building_granary_bless(void)
{
    int min_stored = INFINITE;
    building *min_building = 0;
    for (building *b = building_first_of_type(BUILDING_GRANARY); b; b = b->next_of_type) {
        if (b->state != BUILDING_STATE_IN_USE || b->has_plague) {
            continue;
        }
        int total_stored = 0;
        for (int r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
            total_stored += get_amount(b, r);
        }
        if (total_stored < min_stored) {
            min_stored = total_stored;
            min_building = b;
        }
    }
    if (min_building) {
        city_resource_determine_available();

        const resource_list *list = city_resource_get_available_foods();

        for (int i = 0; i < list->size;  i++) {
            for (int n = 0; n < 6; n++) {
                building_granary_add_resource(min_building, list->items[i], 0);
            }
        }
    }
}

void building_granary_warehouse_curse(int big)
{
    int max_stored = 0;
    building *max_building = 0;
    for (building *b = building_first_of_type(BUILDING_GRANARY); b; b = b->next_of_type) {
        if (b->state != BUILDING_STATE_IN_USE || b->has_plague) {
            continue;
        }
        int total_stored = 0;
        for (int r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
            total_stored += get_amount(b, r);
        }
        total_stored /= UNITS_PER_LOAD;
        if (total_stored > max_stored) {
            max_stored = total_stored;
            max_building = b;
        }
    }
    for (building *b = building_first_of_type(BUILDING_WAREHOUSE); b; b = b->next_of_type) {
        if (b->state != BUILDING_STATE_IN_USE || b->has_plague) {
            continue;
        }
        int total_stored = 0;
        for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
            total_stored += building_warehouse_get_amount(b, r);
        }
        if (total_stored > max_stored) {
            max_stored = total_stored;
            max_building = b;
        }
    }
    if (!max_building) {
        return;
    }
    if (big) {
        city_message_disable_sound_for_next_message();
        city_message_post(0, MESSAGE_FIRE, max_building->type, max_building->grid_offset);
        building_destroy_by_fire(max_building);
        sound_effect_play(SOUND_EFFECT_EXPLOSION);
        map_routing_update_land();
    } else {
        if (max_building->type == BUILDING_WAREHOUSE) {
            building_warehouse_remove_resource_curse(max_building, CURSE_LOADS);
        } else if (max_building->type == BUILDING_GRANARY) {
            int amount = CURSE_LOADS * UNITS_PER_LOAD;
            for (resource_type food = RESOURCE_MIN_FOOD; food < RESOURCE_MAX_FOOD; food++) {
                amount = building_granary_remove_resource(max_building, food, amount);
            }
        }
    }
}

void building_granary_update_built_granaries_capacity(void)
{
    for (building *b = building_first_of_type(BUILDING_GRANARY); b; b = b->next_of_type) {
        int total_units = 0;
        for (int resource = RESOURCE_MIN_FOOD; resource < RESOURCE_MAX_FOOD; resource++) {
            total_units += b->resources[resource];
        }
        total_units += b->resources[RESOURCE_NONE];

        if (total_units < 3200) {
            b->resources[RESOURCE_NONE] += 3200 - total_units;
        }
        // for now, we don't handle the case where we decrease granary capacity
    }
}
