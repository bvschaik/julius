#include "resource.h"

#include "building/building.h"
#include "building/count.h"
#include "building/industry.h"
#include "building/model.h"
#include "building/monument.h"
#include "city/buildings.h"
#include "city/data_private.h"
#include "city/message.h"
#include "city/military.h"
#include "city/trade.h"
#include "city/trade_policy.h"
#include "core/calc.h"
#include "empire/city.h"
#include "figure/figure.h"
#include "figure/formation.h"
#include "game/difficulty.h"
#include "game/tutorial.h"
#include "map/road_access.h"
#include "scenario/building.h"
#include "scenario/property.h"

#include <math.h>

static struct {
    resource_list resource_list;
    resource_list food_list;
} available;

static struct {
    resource_list resource_list;
    resource_list food_list;
} potential;

int city_resource_count_food_on_granaries(resource_type food)
{
    return city_data.resource.granary_food_stored[food];
}

int city_resource_count(resource_type resource)
{
    return city_data.resource.stored_in_warehouses[resource];
}

int city_resource_get_amount_including_granaries(resource_type resource, int amount, int *checked_granaries)
{
    if (checked_granaries) {
        *checked_granaries = 0;
    }
    int amount_stored = city_data.resource.stored_in_warehouses[resource];
    if (amount_stored < amount && resource_is_food(resource)) {
        amount_stored += city_data.resource.granary_food_stored[resource] / 100;
        if (checked_granaries) {
            *checked_granaries = 1;
        }
    }
    return amount_stored;
}

const resource_list *city_resource_get_available(void)
{
    return &available.resource_list;
}

const resource_list *city_resource_get_available_foods(void)
{
    return &available.food_list;
}

const resource_list *city_resource_get_potential(void)
{
    return &potential.resource_list;
}

const resource_list *city_resource_get_potential_foods(void)
{
    return &potential.food_list;
}

int city_resource_multiple_wine_available(void)
{
    return city_data.resource.wine_types_available >= 2;
}

int city_resource_food_types_available(void)
{
    return city_data.resource.food_types_available;
}

int city_resource_food_stored(void)
{
    return city_data.resource.granary_total_stored;
}

int city_resource_food_needed(void)
{
    return city_data.resource.food_needed_per_month;
}

int city_resource_food_supply_months(void)
{
    return city_data.resource.food_supply_months;
}

int city_resource_food_percentage_produced(void)
{
    return calc_percentage(city_data.resource.food_produced_last_month, city_data.resource.food_consumed_last_month);
}

int city_resource_operating_granaries(void)
{
    return city_data.resource.granaries.operating;
}

int city_resource_last_used_warehouse(void)
{
    return city_data.resource.last_used_warehouse;
}

void city_resource_set_last_used_warehouse(int warehouse_id)
{
    city_data.resource.last_used_warehouse = warehouse_id;
}

resource_trade_status city_resource_trade_status(resource_type resource)
{
    return city_data.resource.trade_status[resource];
}

void city_resource_cycle_trade_status(resource_type resource, resource_trade_status status)
{
    if (status == TRADE_STATUS_IMPORT && !empire_can_import_resource(resource)) {
        city_data.resource.trade_status[resource] &= ~TRADE_STATUS_IMPORT;
        return;
    }
    if (status == TRADE_STATUS_EXPORT && !empire_can_export_resource(resource)) {
        city_data.resource.trade_status[resource] &= ~TRADE_STATUS_EXPORT;
        return;
    }
    city_data.resource.trade_status[resource] ^= status;

    if (city_data.resource.trade_status[resource] & TRADE_STATUS_EXPORT) {
        city_data.resource.stockpiled[resource] = 0;
    }
}

int city_resource_import_over(resource_type resource)
{
    return city_data.resource.import_over[resource];
}

void city_resource_change_import_over(resource_type resource, int change)
{
    city_data.resource.import_over[resource] = calc_bound(city_data.resource.import_over[resource] + change, 0, 100);
}

int city_resource_export_over(resource_type resource)
{
    return city_data.resource.export_over[resource];
}

void city_resource_change_export_over(resource_type resource, int change)
{
    city_data.resource.export_over[resource] = calc_bound(city_data.resource.export_over[resource] + change, 0, 100);
}

int city_resource_is_stockpiled(resource_type resource)
{
    return city_data.resource.stockpiled[resource];
}

void city_resource_toggle_stockpiled(resource_type resource)
{
    if (city_data.resource.stockpiled[resource]) {
        city_data.resource.stockpiled[resource] = 0;
        city_data.resource.trade_status[resource] |= city_data.resource.export_status_before_stockpiling[resource];
    } else {
        city_data.resource.stockpiled[resource] = 1;
        city_data.resource.export_status_before_stockpiling[resource] = city_data.resource.trade_status[resource] & TRADE_STATUS_EXPORT;
        city_data.resource.trade_status[resource] &= ~TRADE_STATUS_EXPORT;
    }
}

int city_resource_is_mothballed(resource_type resource)
{
    return city_data.resource.mothballed[resource];
}

void city_resource_toggle_mothballed(resource_type resource)
{
    city_data.resource.mothballed[resource] = city_data.resource.mothballed[resource] ? 0 : 1;
}

void city_resource_add_produced_to_granary(int amount)
{
    city_data.resource.food_produced_this_month += amount;
}

void city_resource_add_to_granary(resource_type food, int amount)
{
    city_data.resource.granary_food_stored[food] += amount;
}

void city_resource_remove_from_granary(resource_type food, int amount)
{
    city_data.resource.granary_food_stored[food] -= amount;
}

void city_resource_add_to_warehouse(resource_type resource, int amount)
{
    city_data.resource.space_in_warehouses[resource] -= amount;
    city_data.resource.stored_in_warehouses[resource] += amount;
}

void city_resource_remove_from_warehouse(resource_type resource, int amount)
{
    city_data.resource.space_in_warehouses[resource] += amount;
    city_data.resource.stored_in_warehouses[resource] -= amount;
}

void city_resource_calculate_warehouse_stocks(void)
{
    for (int i = 0; i < RESOURCE_MAX; i++) {
        city_data.resource.space_in_warehouses[i] = 0;
        city_data.resource.stored_in_warehouses[i] = 0;
    }
    for (building *b = building_first_of_type(BUILDING_WAREHOUSE); b; b = b->next_of_type) {
        if (b->state == BUILDING_STATE_IN_USE) {
            b->has_road_access = 0;
            if (map_has_road_access_rotation(b->subtype.orientation, b->x, b->y, b->size, 0)) {
                b->has_road_access = 1;
            } else if (map_has_road_access_rotation(b->subtype.orientation, b->x, b->y, 3, 0)) {
                b->has_road_access = 2;
            }
        }
    }
    for (building *b = building_first_of_type(BUILDING_WAREHOUSE_SPACE); b; b = b->next_of_type) {
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        building *warehouse = building_main(b);
        if (warehouse->state != BUILDING_STATE_IN_USE || warehouse->type != BUILDING_WAREHOUSE) {
            continue;
        }
        if (warehouse->has_road_access) {
            b->has_road_access = warehouse->has_road_access;
            if (b->subtype.warehouse_resource_id) {
                int loads = b->loads_stored;
                int resource = b->subtype.warehouse_resource_id;
                city_data.resource.stored_in_warehouses[resource] += loads;
                city_data.resource.space_in_warehouses[resource] += 4 - loads;
            } else {
                city_data.resource.space_in_warehouses[RESOURCE_NONE] += 4;
            }
        }
    }
}

void city_resource_determine_available(void)
{
    for (int i = 0; i < RESOURCE_MAX; i++) {
        available.resource_list.items[i] = 0;
        available.food_list.items[i] = 0;
        potential.resource_list.items[i] = 0;
        potential.food_list.items[i] = 0;
    }
    available.resource_list.size = 0;
    available.food_list.size = 0;
    potential.resource_list.size = 0;
    potential.food_list.size = 0;

    for (resource_type r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        if (empire_can_produce_resource(r) || empire_can_import_resource(r)) {
            available.resource_list.items[available.resource_list.size++] = r;
            potential.resource_list.items[potential.resource_list.size++] = r;
            if (resource_is_food(r)) {
                available.food_list.items[available.food_list.size++] = r;
                potential.food_list.items[potential.food_list.size++] = r;               
            }
        } else if (empire_can_produce_resource_potentially(r) || empire_can_import_resource_potentially(r)) {
            potential.resource_list.items[potential.resource_list.size++] = r;
            if (resource_is_food(r)) {
                potential.food_list.items[potential.food_list.size++] = r;
            }
        }
    }
}

resource_type city_resource_ceres_temple_food(void)
{
    resource_type imported = RESOURCE_NONE;

    // locally produced
    for (resource_type r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
        if (can_produce_resource(r)) {
            return r;
        }
        if (imported == RESOURCE_NONE && empire_can_import_resource_potentially(r)) {
            imported = r;
        }
    }

    // imported, if no food is locally produced
    return imported;
}

static void calculate_available_food(void)
{
    for (resource_type r = 0; r < RESOURCE_MAX_FOOD; r++) {
        city_data.resource.granary_food_stored[r] = 0;
    }
    city_data.resource.granary_total_stored = 0;
    city_data.resource.food_types_available = 0;
    city_data.resource.food_supply_months = 0;
    city_data.resource.granaries.operating = 0;
    city_data.resource.granaries.understaffed = 0;
    city_data.resource.granaries.not_operating = 0;
    city_data.resource.granaries.not_operating_with_food = 0;
    for (building *b = building_first_of_type(BUILDING_GRANARY); b; b = b->next_of_type) {
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        b->has_road_access = 0;
        if (map_has_road_access_granary(b->x, b->y, 0)) {
            b->has_road_access = 1;
            int pct_workers = calc_percentage(
                b->num_workers, model_get_building(b->type)->laborers);
            if (pct_workers < 100) {
                city_data.resource.granaries.understaffed++;
            }
            int amount_stored = 0;
            for (int r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
                amount_stored += b->resources[r];
            }
            if (pct_workers < 50) {
                city_data.resource.granaries.not_operating++;
                if (amount_stored > 0) {
                    city_data.resource.granaries.not_operating_with_food++;
                }
            } else {
                city_data.resource.granaries.operating++;
                for (int r = 0; r < RESOURCE_MAX_FOOD; r++) {
                    city_data.resource.granary_food_stored[r] += b->resources[r];
                }
                if (amount_stored > 400) {
                    tutorial_on_filled_granary();
                }
            }
        }
    }
    for (int i = RESOURCE_MIN_FOOD; i < RESOURCE_MAX_FOOD; i++) {
        if (city_data.resource.granary_food_stored[i]) {
            city_data.resource.granary_total_stored += city_data.resource.granary_food_stored[i];
            city_data.resource.food_types_available++;
        }
    }
    city_data.resource.food_needed_per_month =
        calc_adjust_with_percentage(city_data.population.population, 50);
    if (city_data.resource.food_needed_per_month > 0) {
        city_data.resource.food_supply_months =
            city_data.resource.granary_total_stored / city_data.resource.food_needed_per_month;
    } else {
        city_data.resource.food_supply_months =
            city_data.resource.granary_total_stored > 0 ? 1 : 0;
    }
    if (scenario_property_rome_supplies_wheat()) {
        city_data.resource.food_types_available = 1;
        city_data.resource.food_supply_months = 12;
    }
}

void city_resource_calculate_food_stocks_and_supply_wheat(void)
{
    calculate_available_food();
    if (scenario_property_rome_supplies_wheat()) {
        building_type supplied_buildings[] = { BUILDING_MARKET, BUILDING_MESS_HALL };
        for (int i = 0; i < 2; i++) {
            building_type type = supplied_buildings[i];
            for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
                if (b->state == BUILDING_STATE_IN_USE) {
                    b->resources[RESOURCE_WHEAT] = 200;
                }
            }
        }
    }
}

static int house_consume_food(void)
{
    int total_consumed = 0;
    int ceres_module = (building_monument_gt_module_is_active(CERES_MODULE_1_REDUCE_FOOD));
    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state != BUILDING_STATE_IN_USE || !b->house_size) {
                continue;
            }
            int num_types = model_get_house(b->subtype.house_level)->food_types;
            int amount_per_type;
            if (ceres_module && b->data.house.temple_ceres) {
                amount_per_type = calc_adjust_with_percentage(b->house_population, 40);
            } else {
                amount_per_type = calc_adjust_with_percentage(b->house_population, 50);
            }
            int foodtypes_available = 0;
            for (resource_type r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
                if (b->resources[r] && resource_is_inventory(r)) {
                    foodtypes_available++;
                }
            }
            if (foodtypes_available) {
                amount_per_type /= foodtypes_available;
            }

            b->data.house.num_foods = 0;
            if (scenario_property_rome_supplies_wheat()) {
                city_data.resource.food_types_eaten = 1;
                city_data.resource.food_types_available = 1;
                b->resources[RESOURCE_WHEAT] = amount_per_type;
                b->data.house.num_foods = 1;
            } else if (num_types > 0) {
                for (resource_type r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
                    if (!resource_is_inventory(r)) {
                        continue;
                    }
                    if (b->resources[r] >= amount_per_type) {
                        b->resources[r] -= amount_per_type;
                        b->data.house.num_foods++;
                        total_consumed += amount_per_type;
                    } else if (b->resources[r]) {
                        // has food but not enough
                        b->resources[r] = 0;
                        b->data.house.num_foods++;
                        total_consumed += amount_per_type;
                    }
                    if (b->data.house.num_foods > city_data.resource.food_types_eaten) {
                        city_data.resource.food_types_eaten = b->data.house.num_foods;
                    }
                }
            }
        }
    }
    return total_consumed;
}

static int mess_hall_consume_food(void)
{
    int total_consumed = 0;
    building *b = building_first_of_type(BUILDING_MESS_HALL);
    if (!b || b->state != BUILDING_STATE_IN_USE) {
        return 0;
    };
    int food_required = city_military_total_soldiers_in_city() *
        difficulty_adjust_soldier_food_consumption(FOOD_PER_SOLDIER_MONTHLY);
    int num_foods = 0;
    int total_food_in_mess_hall = 0;
    int proportionate_amount = 0;
    int amount_for_type = 0;

    for (resource_type r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
        total_food_in_mess_hall += b->resources[r];
    }

    city_data.mess_hall.total_food = total_food_in_mess_hall;
    if (!food_required) {
        return 0;
    }

    if (total_food_in_mess_hall > 0) {
        for (resource_type r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
            proportionate_amount = food_required * b->resources[r] / total_food_in_mess_hall;
            if (proportionate_amount > 0) {
                amount_for_type = calc_bound((int) ceil(proportionate_amount), 0, b->resources[r]);
                b->resources[r] -= amount_for_type;
                ++num_foods;
            }
        }
    }

    if (food_required > total_food_in_mess_hall) {
        city_data.mess_hall.food_percentage_missing_this_month = 100 -
            calc_percentage(total_food_in_mess_hall, food_required);
        total_consumed += total_food_in_mess_hall;
        city_data.mess_hall.total_food = 0;
    } else {
        city_data.mess_hall.food_percentage_missing_this_month = 0;
        total_consumed += food_required;
        city_data.mess_hall.total_food -= food_required;
    }

    city_data.mess_hall.food_types = num_foods;

    return total_consumed;
}

static int caravanserai_consume_food(void)
{
    if (!building_monument_working(BUILDING_CARAVANSERAI)) {
        return 0;
    }
    int food_required = trade_caravan_count() * FOOD_PER_TRADER_MONTHLY;

    trade_policy policy = city_trade_policy_get(LAND_TRADE_POLICY);

    if (policy == TRADE_POLICY_3) { // consume 20% more
        food_required = calc_adjust_with_percentage(food_required, 100 + POLICY_3_MALUS_PERCENT);
    }

    int total_consumed = 0;
    building *b = building_first_of_type(BUILDING_CARAVANSERAI);
    if (!b) {
        return 0;
    }

    int total_food_in_caravanserai = 0;
    int proportionate_amount = 0;
    int amount_for_type = 0;

    for (resource_type r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
        total_food_in_caravanserai += b->resources[r];
    }

    city_data.caravanserai.total_food = total_food_in_caravanserai;


    if (!food_required || !total_food_in_caravanserai) {
        return 0;
    }

    for (resource_type r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
        proportionate_amount = food_required * b->resources[r] / total_food_in_caravanserai;
        if (proportionate_amount > 0) {
            amount_for_type = calc_bound(proportionate_amount, 0, b->resources[r]);
            b->resources[r] -= amount_for_type;
        }
    }

    if (food_required > total_food_in_caravanserai) {
        total_consumed += total_food_in_caravanserai;
        city_data.caravanserai.total_food = 0;
    } else {
        total_consumed += food_required;
        city_data.caravanserai.total_food -= food_required;
    }

    return total_consumed;
}

void city_resource_consume_food(void)
{
    calculate_available_food();
    city_data.resource.food_types_eaten = 0;

    int total_consumed = house_consume_food() + mess_hall_consume_food() + caravanserai_consume_food();

    if (city_military_total_soldiers_in_city() > 0 && !city_buildings_has_mess_hall() &&
        !city_data.mess_hall.missing_mess_hall_warning_shown) {
        city_data.mess_hall.food_percentage_missing_this_month = 100;
        city_message_post(1, MESSAGE_SOLDIERS_STARVING_NO_MESS_HALL, 0, 0);
        city_data.mess_hall.missing_mess_hall_warning_shown = 1;
    } else if (city_military_total_soldiers_in_city() > 0 && city_data.mess_hall.food_stress_cumulative > 50 &&
        !city_data.mess_hall.mess_hall_warning_shown) {
        city_message_post(1, MESSAGE_SOLDIERS_STARVING, 0, 0);
        city_data.mess_hall.mess_hall_warning_shown = 1;
    }

    city_data.mess_hall.food_stress_cumulative = ((city_data.mess_hall.food_percentage_missing_this_month +
        city_data.mess_hall.food_stress_cumulative) / 2);
    if (city_data.mess_hall.food_stress_cumulative < 20) {
        city_data.mess_hall.mess_hall_warning_shown = 0;
    }

    if (city_data.mess_hall.food_stress_cumulative > 100) {
        city_data.mess_hall.food_stress_cumulative = 100;
    }

    city_data.resource.food_consumed_last_month = total_consumed;
    city_data.resource.food_produced_last_month = city_data.resource.food_produced_this_month;
    city_data.resource.food_produced_this_month = 0;
}
