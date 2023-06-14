#include "city.h"

#include "building/monument.h"
#include "core/array.h"
#include "core/calc.h"
#include "core/lang.h"
#include "core/log.h"
#include "core/string.h"
#include "city/buildings.h"
#include "city/finance.h"
#include "city/map.h"
#include "city/message.h"
#include "city/trade.h"
#include "city/resource.h"
#include "empire/object.h"
#include "empire/trade_route.h"
#include "empire/type.h"
#include "figuretype/trader.h"
#include "game/resource.h"
#include "game/save_version.h"
#include "scenario/building.h"
#include "scenario/empire.h"
#include "scenario/map.h"

#include <string.h>

#define RESOURCES_TO_TRADER_RATIO 60
#define LAND_TRADER_DELAY_TICKS 4
#define SEA_TRADER_DELAY_TICKS 30
#define LEGACY_MAX_CITIES 41
#define CITIES_ARRAY_SIZE_STEP 50

#define EMPIRE_CITY_CURRENT_BUF_SIZE (18 + 2 * RESOURCE_MAX)

static array(empire_city) cities;

static int city_in_use(const empire_city *city)
{
    return city->in_use;
}

void empire_city_clear_all(void)
{
    array_init(cities, CITIES_ARRAY_SIZE_STEP, 0, city_in_use);
    // Discard city 0
    array_advance(cities);
}

empire_city *empire_city_get(int city_id)
{
    return array_item(cities, city_id);
}

empire_city *empire_city_get_new(void)
{
    return array_advance(cities);
}

int empire_city_get_route_id(int city_id)
{
    return array_item(cities, city_id)->route_id;
}

int empire_city_get_id_by_name(const uint8_t *city_name)
{
    empire_city *city;
    array_foreach(cities, city) {
        const uint8_t *current_name = empire_city_get_name(city);
        if (string_equals(current_name, city_name)) {
            return i;
        }
    }
    return 0;
}

int empire_can_import_resource(int resource)
{
    empire_city *city;
    array_foreach(cities, city) {
        if (city->in_use && city->type == EMPIRE_CITY_TRADE && city->is_open && city->sells_resource[resource] == 1) {
            return 1;
        }
    }
    return 0;
}

int empire_can_import_resource_potentially(int resource)
{
    empire_city *city;
    array_foreach(cities, city) {
        if (city->in_use && city->type == EMPIRE_CITY_TRADE && city->sells_resource[resource] == 1) {
            return 1;
        }
    }
    return 0;
}

int empire_has_access_to_resource(int resource)
{
    empire_city *city;
    array_foreach(cities, city) {
        if (city->in_use && (city->type == EMPIRE_CITY_OURS || city->type == EMPIRE_CITY_TRADE) &&
            city->sells_resource[resource] == 1) {
            return 1;
        }
    }
    return 0;
}

int empire_can_export_resource_potentially(int resource)
{
    empire_city *city;
    array_foreach(cities, city) {
        if (city->in_use && city->type == EMPIRE_CITY_TRADE && city->buys_resource[resource] == 1) {
            return 1;
        }
    }
    return 0;
}

int empire_can_export_resource(int resource)
{
    empire_city *city;
    array_foreach(cities, city) {
        if (city->in_use && city->type == EMPIRE_CITY_TRADE && city->is_open && city->buys_resource[resource] == 1) {
            return 1;
        }
    }
    return 0;
}

static int can_produce_resource_naturally(resource_type resource)
{
    empire_city *city;
    array_foreach(cities, city) {
        if (city->in_use && city->type == EMPIRE_CITY_OURS && city->sells_resource[resource] == 1) {
            return 1;
        }
    }
    return 0;
}

int empire_can_produce_resource_locally(int resource)
{
    if (can_produce_resource_naturally(resource)) {
        return 1;
    }

    // Wine can also be produced via Venus Grand Temple
    if (resource == RESOURCE_WINE) {
        return !building_monument_requires_resource(BUILDING_GRAND_TEMPLE_VENUS, RESOURCE_WINE) &&
            scenario_building_allowed(BUILDING_MENU_GRAND_TEMPLES) &&
            scenario_building_allowed(BUILDING_GRAND_TEMPLE_VENUS) &&
            building_monument_has_required_resources_to_build(BUILDING_GRAND_TEMPLE_VENUS);
    }
    // Gold can also be produced via City Mint
    if (resource == RESOURCE_GOLD) {
        return !building_monument_requires_resource(BUILDING_CITY_MINT, RESOURCE_GOLD) &&
            scenario_building_allowed(BUILDING_CITY_MINT) &&
            building_monument_has_required_resources_to_build(BUILDING_CITY_MINT);
    }
    return 0;
}

int empire_can_produce_resource(int resource)
{
    resource_supply_chain chain[RESOURCE_SUPPLY_CHAIN_MAX_SIZE];
    int num_raw_materials = resource_get_supply_chain_for_good(chain, resource);
    // finished goods: check imports of raw materials
    if (num_raw_materials > 0) {
        for (int i = 0; i < num_raw_materials; i++) {
            if (resource_is_raw_material(chain[i].raw_material)) {
                if (!empire_can_import_resource(chain[i].raw_material) &&
                    !empire_can_produce_resource_locally(chain[i].raw_material)) {
                    return 0;
                }
            } else if (!empire_can_produce_resource(chain[i].raw_material)) {
                return 0;
            }
        }
        return 1;
    }
    // check if we can produce the raw materials
    return empire_can_produce_resource_locally(resource);
}

int empire_can_produce_resource_naturally(int resource)
{
    resource_supply_chain chain[RESOURCE_SUPPLY_CHAIN_MAX_SIZE];
    int num_raw_materials = resource_get_supply_chain_for_good(chain, resource);
    // finished goods: check imports of raw materials
    if (num_raw_materials > 0) {
        for (int i = 0; i < num_raw_materials; i++) {
            if (resource_is_raw_material(chain[i].raw_material)) {
                if (!empire_can_import_resource(chain[i].raw_material) &&
                    !can_produce_resource_naturally(chain[i].raw_material)) {
                    return 0;
                }
            } else if (!empire_can_produce_resource_naturally(chain[i].raw_material)) {
                return 0;
            }
        }
        return 1;
    }
    // check if we can produce the raw materials
    return can_produce_resource_naturally(resource);
}

int empire_can_produce_resource_potentially(int resource)
{
    resource_supply_chain chain[RESOURCE_SUPPLY_CHAIN_MAX_SIZE];
    int num_raw_materials = resource_get_supply_chain_for_good(chain, resource);
    // finished goods: check imports of raw materials
    if (num_raw_materials > 0) {
        for (int i = 0; i < num_raw_materials; i++) {
            if (resource_is_raw_material(chain[i].raw_material)) {
                if (!empire_can_import_resource_potentially(chain[i].raw_material) &&
                    !empire_can_produce_resource_locally(chain[i].raw_material)) {
                    return 0;
                }
            } else if (!empire_can_produce_resource_potentially(chain[i].raw_material)) {
                return 0;
            }
        }
        return 1;
    }
    // check if we can produce the raw materials
    return empire_can_produce_resource_locally(resource);
}

int empire_city_get_for_object(int empire_object_id)
{
    empire_city *city;
    array_foreach(cities, city) {
        if (city->in_use && city->empire_object_id == empire_object_id) {
            return i;
        }
    }
    return 0;
}

int empire_city_get_for_trade_route(int route_id)
{
    empire_city *city;
    array_foreach(cities, city) {
        if (city->in_use && city->route_id == route_id) {
            return i;
        }
    }
    return -1;
}

int empire_city_is_trade_route_open(int route_id)
{
    empire_city *city;
    array_foreach(cities, city) {
        if (city->in_use && city->route_id == route_id) {
            return city->is_open ? 1 : 0;
        }
    }
    return 0;
}

void empire_city_reset_yearly_trade_amounts(void)
{
    empire_city *city;
    array_foreach(cities, city) {
        if (city->in_use && city->is_open) {
            trade_route_reset_traded(city->route_id);
        }
    }
}

int empire_city_count_wine_sources(void)
{
    int sources = 0;
    empire_city *city;
    array_foreach(cities, city) {
        if (city->in_use && city->is_open && city->sells_resource[RESOURCE_WINE]) {
            sources++;
        }
    }
    return sources;
}

int empire_city_get_vulnerable_roman(void)
{
    int city_id = 0;
    empire_city *city;
    array_foreach(cities, city) {
        if (city->in_use && city->type == EMPIRE_CITY_VULNERABLE_ROMAN) {
            city_id = i;
        }
    }
    return city_id;
}

void empire_city_expand_empire(void)
{
    empire_city *city;
    array_foreach(cities, city) {
        if (!city->in_use) {
            continue;
        }
        if (city->type == EMPIRE_CITY_FUTURE_TRADE) {
            city->type = EMPIRE_CITY_TRADE;
        } else if (city->type == EMPIRE_CITY_FUTURE_ROMAN) {
            city->type = EMPIRE_CITY_DISTANT_ROMAN;
        } else {
            continue;
        }
        empire_object_set_expanded(city->empire_object_id, city->type);
    }
}

// Override hardcoded empire data to allow new trade
void empire_city_force_sell(int route, int resource)
{
    empire_city *city;
    array_foreach(cities, city) {
        if (city->route_id == route) {
            city->sells_resource[resource] = 1;
            empire_object_city_force_sell_resource(city->empire_object_id, resource);
            break;
        }        
    }
}

static int generate_trader(int city_id, empire_city *city)
{
    // Check timeout before city can send another trader
    if (city->trader_entry_delay > 0) {
        city->trader_entry_delay--;
        return 0;
    }
    city->trader_entry_delay = city->is_sea_trade ? SEA_TRADER_DELAY_TICKS : LAND_TRADER_DELAY_TICKS;

    // Check that we have space to trade
    int trade_potential = 0;
    for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        if (city->buys_resource[r] || city->sells_resource[r]) {
            trade_potential += trade_route_limit(city->route_id, r);
        }
    }
    if (trade_potential <= 0) {
        return 0;
    }

    // Find a slot to hold a trader
    int max_traders = calc_bound(trade_potential / RESOURCES_TO_TRADER_RATIO + 1, 1, EMPIRE_CITY_MAX_TRADERS);
    int index = -1;
    for (int i = 0; i < max_traders; i++) {
        if (!city->trader_figure_ids[i]) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        return 0;
    }

    if (city->is_sea_trade) {
        // generate ship
        if (city_buildings_has_working_dock() && scenario_map_has_river_entry()
            && !city_trade_has_sea_trade_problems()) {
            map_point river_entry = scenario_map_river_entry();
            city->trader_figure_ids[index] = figure_create_trade_ship(river_entry.x, river_entry.y, city_id);
            return 1;
        }
    } else {
        // generate caravan and donkeys
        if (!city_trade_has_land_trade_problems()) {
            // caravan head
            const map_tile *entry = city_map_entry_point();
            city->trader_figure_ids[index] = figure_create_trade_caravan(entry->x, entry->y, city_id);
            return 1;
        }
    }
    return 0;
}

void empire_city_open_trade(int city_id)
{
    empire_city *city = array_item(cities, city_id);
    city_finance_process_construction(city->cost_to_open);
    city->is_open = 1;
}

void empire_city_generate_trader(void)
{
    empire_city *city;
    array_foreach(cities, city) {
        if (!city->in_use || !city->is_open) {
            continue;
        }
        if (city->is_sea_trade) {
            if (!city_buildings_has_working_dock()) {
                // delay of 384 = 1 year
                city_message_post_with_message_delay(MESSAGE_CAT_NO_WORKING_DOCK, 1, MESSAGE_NO_WORKING_DOCK, 384);
                continue;
            }
            if (!scenario_map_has_river_entry()) {
                continue;
            }
            city_trade_add_sea_trade_route();
        } else {
            city_trade_add_land_trade_route();
        }
        if (generate_trader(i, city)) {
            break;
        }
    }
}

void empire_city_remove_trader(int city_id, int figure_id)
{
    empire_city *city = array_item(cities, city_id);
    for (int i = 0; i < EMPIRE_CITY_MAX_TRADERS; i++) {
        if (city->trader_figure_ids[i] == figure_id) {
            city->trader_figure_ids[i] = 0;
        }
    }
}

void empire_city_set_vulnerable(int city_id)
{
    array_item(cities, city_id)->type = EMPIRE_CITY_VULNERABLE_ROMAN;
}

void empire_city_set_foreign(int city_id)
{
    array_item(cities, city_id)->type = EMPIRE_CITY_DISTANT_FOREIGN;
}

int empire_unlock_all_resources(void)
{
    empire_city *city;
    array_foreach(cities, city) {
        if (city->in_use && (city->type == EMPIRE_CITY_OURS)) {
            for (int resource = RESOURCE_MIN; resource < RESOURCE_MAX; resource++) {
                city->sells_resource[resource] = 1;
            }
            return 1;
        }
    }
    return 0;
}

int empire_city_change_own_resource_availability(resource_type resource, int is_available)
{
    empire_city *city;
    array_foreach(cities, city) {
        if (city->in_use && (city->type == EMPIRE_CITY_OURS)) {
            city->sells_resource[resource] = is_available;
            return 1;
        }
    }
    return 0;
}

const uint8_t *empire_city_get_name(const empire_city *city)
{
    full_empire_object *full = empire_object_get_full(city->empire_object_id);
    if (string_length(full->city_custom_name)) {
        return full->city_custom_name;
    }
    return lang_get_string(21, city->name_id);
}

void empire_city_save_state(buffer *buf)
{
    int buf_size = sizeof(int32_t) + cities.size * EMPIRE_CITY_CURRENT_BUF_SIZE;
    uint8_t *buf_data = malloc(buf_size);
    buffer_init(buf, buf_data, buf_size);
    buffer_write_i32(buf, EMPIRE_CITY_CURRENT_BUF_SIZE);

    empire_city *city;
    array_foreach(cities, city) {
        buffer_write_u8(buf, city->in_use);
        buffer_write_u8(buf, city->type);
        buffer_write_u8(buf, city->name_id);
        buffer_write_u8(buf, city->route_id);
        buffer_write_u8(buf, city->is_open);
        for (int r = 0; r < RESOURCE_MAX; r++) {
            buffer_write_u8(buf, city->buys_resource[r]);
        }
        for (int r = 0; r < RESOURCE_MAX; r++) {
            buffer_write_u8(buf, city->sells_resource[r]);
        }
        buffer_write_i16(buf, city->cost_to_open);
        buffer_write_i16(buf, city->trader_entry_delay);
        buffer_write_i16(buf, city->empire_object_id);
        buffer_write_u8(buf, city->is_sea_trade);
        for (int f = 0; f < EMPIRE_CITY_MAX_TRADERS; f++) {
            buffer_write_i16(buf, city->trader_figure_ids[f]);
        }
    }
}

int empire_city_can_mine_gold(int city_name_id)
{
    switch (city_name_id) {
        case 4:  // Mediolanum
        case 6:  // Carthago
        case 7:  // Tarraco
        case 30: // Lugdunum
        case 36: // Londinium
        case 38: // Sarmizegetusa
            return 1;
        default:
            return 0;
    }
}

static void set_new_monument_elements_production(empire_city *city)
{
    if (city->sells_resource[RESOURCE_IRON] || city->sells_resource[RESOURCE_MARBLE]) {
        city->sells_resource[RESOURCE_STONE] = 1;
        if (city->type != EMPIRE_CITY_OURS) {
            trade_route_set_limit(city->route_id, RESOURCE_STONE, 25);
        }
    }
    if (city->sells_resource[RESOURCE_CLAY]) {
        city->sells_resource[RESOURCE_SAND] = 1;
        if (city->type != EMPIRE_CITY_OURS) {
            trade_route_set_limit(city->route_id, RESOURCE_SAND, 25);
        }
    }
    // If a city sells both sand and stone and also sells more than four items,
    // make land routes sell stone and water routes sell sand
    if (city->type != EMPIRE_CITY_OURS && city->sells_resource[RESOURCE_SAND] && city->sells_resource[RESOURCE_STONE]) {
        int resources_sold = 0;
        for (resource_type r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
            if (city->sells_resource[r]) {
                resources_sold++;
                if (resources_sold > 4) {
                    if (city->is_sea_trade) {
                        city->sells_resource[RESOURCE_STONE] = 0;
                        trade_route_set_limit(city->route_id, RESOURCE_STONE, 0);
                    } else {
                        city->sells_resource[RESOURCE_SAND] = 0;
                        trade_route_set_limit(city->route_id, RESOURCE_SAND, 0);
                    }
                    break;
                }
            }
        }
    }
}

static void set_gold_production(empire_city *city)
{
    if (empire_city_can_mine_gold(city->name_id) && city->sells_resource[RESOURCE_IRON]) {
        city->sells_resource[RESOURCE_GOLD] = 1;
        if (city->type != EMPIRE_CITY_OURS) {
            trade_route_set_limit(city->route_id, RESOURCE_GOLD, 5);
        }
    }
}

void empire_city_update_trading_data(int version)
{
    if (scenario_empire_id() == SCENARIO_CUSTOM_EMPIRE) {
        return;
    }
    empire_city *city;
    array_foreach(cities, city) {
        if (version < SAVE_GAME_LAST_NO_NEW_MONUMENT_RESOURCES) {
            set_new_monument_elements_production(city);
        }
        if (version < SAVE_GAME_LAST_NO_GOLD_AND_MINTING) {
            set_gold_production(city);
        }
    }
}

void empire_city_load_state(buffer *buf, int version)
{
    int cities_to_load;
    if (version <= SAVE_GAME_LAST_STATIC_SCENARIO_OBJECTS) {
        cities_to_load = LEGACY_MAX_CITIES;
    } else {
        cities_to_load = (int) (buf->size - sizeof(int32_t)) / buffer_read_i32(buf);
    }

    if (!array_init(cities, CITIES_ARRAY_SIZE_STEP, 0, city_in_use) ||
        !array_expand(cities, cities_to_load)) {
        log_error("Unable to allocate enough memory for the empire city array. The game will now crash.", 0, 0);
    }

    if (!trade_route_init()) {
        return;
    }

    int highest_id_in_use = 0;

    for (int i = 0; i < cities_to_load; i++) {
        empire_city *city = array_next(cities);
        city->in_use = buffer_read_u8(buf);
        if (city->in_use) {
            highest_id_in_use = i;
        }
        if (version <= SAVE_GAME_LAST_STATIC_SCENARIO_OBJECTS) {
            buffer_skip(buf, 1);
        }
        city->type = buffer_read_u8(buf);
        city->name_id = buffer_read_u8(buf);
        city->route_id = buffer_read_u8(buf);
        city->is_open = buffer_read_u8(buf);
        for (int r = 0; r < resource_total_mapped(); r++) {
            city->buys_resource[resource_remap(r)] = buffer_read_u8(buf);
        }
        for (int r = 0; r < resource_total_mapped(); r++) {
            city->sells_resource[resource_remap(r)] = buffer_read_u8(buf);
        }
        city->cost_to_open = buffer_read_i16(buf);
        if (version <= SAVE_GAME_LAST_STATIC_SCENARIO_OBJECTS) {
            buffer_skip(buf, 2);
        }
        city->trader_entry_delay = buffer_read_i16(buf);
        if (version <= SAVE_GAME_LAST_STATIC_SCENARIO_OBJECTS) {
            buffer_skip(buf, 2);
        }
        city->empire_object_id = buffer_read_i16(buf);
        city->is_sea_trade = buffer_read_u8(buf);
        if (version <= SAVE_GAME_LAST_STATIC_SCENARIO_OBJECTS) {
            buffer_skip(buf, 1);
        }
        for (int f = 0; f < EMPIRE_CITY_MAX_TRADERS; f++) {
            city->trader_figure_ids[f] = buffer_read_i16(buf);
        }
        if (version <= SAVE_GAME_LAST_STATIC_SCENARIO_OBJECTS) {
            buffer_skip(buf, 10);
        }
        if (resource_mapping_get_version() < RESOURCE_SEPARATE_FISH_AND_MEAT_VERSION) {
            if (city->type == EMPIRE_CITY_OURS) {
                if (city->sells_resource[RESOURCE_FISH]) {
                    city->sells_resource[RESOURCE_MEAT] = 1;
                } else if (scenario_building_allowed(BUILDING_WHARF)) {
                    city->sells_resource[RESOURCE_FISH] = 1;
                }
            }
        }
    }
    cities.size = highest_id_in_use + 1;
}

int empire_city_get_array_size(void)
{
    return cities.size;
}
