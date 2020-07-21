#include "city.h"

#include "city/buildings.h"
#include "city/finance.h"
#include "city/map.h"
#include "city/message.h"
#include "city/trade.h"
#include "empire/object.h"
#include "empire/trade_route.h"
#include "empire/type.h"
#include "figuretype/trader.h"
#include "scenario/map.h"

#include <string.h>

#define MAX_CITIES 41

static empire_city cities[MAX_CITIES];

void empire_city_clear_all(void)
{
    memset(cities, 0, sizeof(cities));
}

empire_city *empire_city_get(int city_id)
{
    if (city_id >= 0 && city_id < MAX_CITIES) {
        return &cities[city_id];
    } else {
        return 0;
    }
}

int empire_city_get_route_id(int city_id)
{
    return cities[city_id].route_id;
}

int empire_can_import_resource(int resource)
{
    for (int i = 0; i < MAX_CITIES; i++) {
        if (cities[i].in_use &&
            cities[i].type == EMPIRE_CITY_TRADE &&
            cities[i].is_open &&
            cities[i].sells_resource[resource] == 1) {
            return 1;
        }
    }
    return 0;
}

int empire_can_import_resource_potentially(int resource)
{
    for (int i = 0; i < MAX_CITIES; i++) {
        if (cities[i].in_use &&
            cities[i].type == EMPIRE_CITY_TRADE &&
            cities[i].sells_resource[resource] == 1) {
            return 1;
        }
    }
    return 0;
}

int empire_can_export_resource(int resource)
{
    for (int i = 0; i < MAX_CITIES; i++) {
        if (cities[i].in_use &&
            cities[i].type == EMPIRE_CITY_TRADE &&
            cities[i].is_open &&
            cities[i].buys_resource[resource] == 1) {
            return 1;
        }
    }
    return 0;
}

static int can_produce_resource(int resource)
{
    for (int i = 0; i < MAX_CITIES; i++) {
        if (cities[i].in_use &&
            cities[i].type == EMPIRE_CITY_OURS &&
            cities[i].sells_resource[resource] == 1) {
            return 1;
        }
    }
    return 0;
}

static int get_raw_resource(int resource)
{
    switch (resource) {
        case RESOURCE_POTTERY:
            return RESOURCE_CLAY;
        case RESOURCE_FURNITURE:
            return RESOURCE_TIMBER;
        case RESOURCE_OIL:
            return RESOURCE_OLIVES;
        case RESOURCE_WINE:
            return RESOURCE_VINES;
        case RESOURCE_WEAPONS:
            return RESOURCE_IRON;
        default:
            return resource;
    }
}

int empire_can_produce_resource(int resource)
{
    int raw_resource = get_raw_resource(resource);
    // finished goods: check imports of raw materials
    if (raw_resource != resource && empire_can_import_resource(raw_resource)) {
        return 1;
    }
    // check if we can produce the raw materials
    return can_produce_resource(raw_resource);
}

int empire_can_produce_resource_potentially(int resource)
{
    int raw_resource = get_raw_resource(resource);
    // finished goods: check imports of raw materials
    if (raw_resource != resource && empire_can_import_resource_potentially(raw_resource)) {
        return 1;
    }
    // check if we can produce the raw materials
    return can_produce_resource(raw_resource);
}

int empire_city_get_for_object(int empire_object_id)
{
    for (int i = 0; i < MAX_CITIES; i++) {
        if (cities[i].in_use && cities[i].empire_object_id == empire_object_id) {
            return i;
        }
    }
    return 0;
}

int empire_city_get_for_trade_route(int route_id)
{
    for (int i = 0; i < MAX_CITIES; i++) {
        if (cities[i].in_use && cities[i].route_id == route_id) {
            return i;
        }
    }
    return -1;
}

int empire_city_is_trade_route_open(int route_id)
{
    for (int i = 0; i < MAX_CITIES; i++) {
        if (cities[i].in_use && cities[i].route_id == route_id) {
            return cities[i].is_open ? 1 : 0;
        }
    }
    return 0;
}

void empire_city_reset_yearly_trade_amounts(void)
{
    for (int i = 0; i < MAX_CITIES; i++) {
        if (cities[i].in_use && cities[i].is_open) {
            trade_route_reset_traded(cities[i].route_id);
        }
    }
}

int empire_city_count_wine_sources(void)
{
    int sources = 0;
    for (int i = 1; i < MAX_CITIES; i++) {
        if (cities[i].in_use &&
            cities[i].is_open &&
            cities[i].sells_resource[RESOURCE_WINE]) {
            sources++;
        }
    }
    return sources;
}

int empire_city_get_vulnerable_roman(void)
{
    int city = 0;
    for (int i = 0; i < MAX_CITIES; i++) {
        if (cities[i].in_use) {
            if (cities[i].type == EMPIRE_CITY_VULNERABLE_ROMAN) {
                city = i;
            }
        }
    }
    return city;
}

void empire_city_expand_empire(void)
{
    for (int i = 0; i < MAX_CITIES; i++) {
        if (!cities[i].in_use) {
            continue;
        }
        if (cities[i].type == EMPIRE_CITY_FUTURE_TRADE) {
            cities[i].type = EMPIRE_CITY_TRADE;
        } else if (cities[i].type == EMPIRE_CITY_FUTURE_ROMAN) {
            cities[i].type = EMPIRE_CITY_DISTANT_ROMAN;
        } else {
            continue;
        }
        empire_object_set_expanded(cities[i].empire_object_id, cities[i].type);
    }
}

static int generate_trader(int city_id, empire_city *city)
{
    int max_traders = 0;
    int num_resources = 0;
    for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        if (city->buys_resource[r] || city->sells_resource[r]) {
            ++num_resources;
            switch (trade_route_limit(city->route_id, r)) {
                case 15: max_traders += 1; break;
                case 25: max_traders += 2; break;
                case 40: max_traders += 3; break;
            }
        }
    }
    if (num_resources > 1) {
        if (max_traders % num_resources) {
            max_traders = max_traders / num_resources + 1;
        } else {
            max_traders = max_traders / num_resources;
        }
    }
    if (max_traders <= 0) {
        return 0;
    }

    int index;
    if (max_traders == 1) {
        if (!city->trader_figure_ids[0]) {
            index = 0;
        } else {
            return 0;
        }
    } else if (max_traders == 2) {
        if (!city->trader_figure_ids[0]) {
            index = 0;
        } else if (!city->trader_figure_ids[1]) {
            index = 1;
        } else {
            return 0;
        }
    } else { // 3
        if (!city->trader_figure_ids[0]) {
            index = 0;
        } else if (!city->trader_figure_ids[1]) {
            index = 1;
        } else if (!city->trader_figure_ids[2]) {
            index = 2;
        } else {
            return 0;
        }
    }

    if (city->trader_entry_delay > 0) {
        city->trader_entry_delay--;
        return 0;
    }
    city->trader_entry_delay = city->is_sea_trade ? 30 : 4;

    if (city->is_sea_trade) {
        // generate ship
        if (city_buildings_has_working_dock() && scenario_map_has_river_entry() && !city_trade_has_sea_trade_problems()) {
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
    empire_city *city = &cities[city_id];
    city_finance_process_construction(city->cost_to_open);
    city->is_open = 1;
}

void empire_city_generate_trader(void)
{
    for (int i = 1; i < MAX_CITIES; i++) {
        if (!cities[i].in_use || !cities[i].is_open) {
            continue;
        }
        if (cities[i].is_sea_trade) {
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
        if (generate_trader(i, &cities[i])) {
            break;
        }
    }
}

void empire_city_remove_trader(int city_id, int figure_id)
{
    for (int i = 0; i < 3; i++) {
        if (cities[city_id].trader_figure_ids[i] == figure_id) {
            cities[city_id].trader_figure_ids[i] = 0;
        }
    }
}

void empire_city_set_vulnerable(int city_id)
{
    cities[city_id].type = EMPIRE_CITY_VULNERABLE_ROMAN;
}

void empire_city_set_foreign(int city_id)
{
    cities[city_id].type = EMPIRE_CITY_DISTANT_FOREIGN;
}

void empire_city_save_state(buffer *buf)
{
    for (int i = 0; i < MAX_CITIES; i++) {
        empire_city *city = &cities[i];
        buffer_write_u8(buf, city->in_use);
        buffer_write_u8(buf, 0);
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
        buffer_skip(buf, 2);
        buffer_write_i16(buf, city->trader_entry_delay);
        buffer_write_i16(buf, 0);
        buffer_write_i16(buf, city->empire_object_id);
        buffer_write_u8(buf, city->is_sea_trade);
        buffer_write_u8(buf, 0);
        for (int f = 0; f < 3; f++) {
            buffer_write_i16(buf, city->trader_figure_ids[f]);
        }
        for (int p = 0; p < 10; p++) {
            buffer_write_u8(buf, 0);
        }
    }
}

void empire_city_load_state(buffer *buf)
{
    for (int i = 0; i < MAX_CITIES; i++) {
        empire_city *city = &cities[i];
        city->in_use = buffer_read_u8(buf);
        buffer_skip(buf, 1);
        city->type = buffer_read_u8(buf);
        city->name_id = buffer_read_u8(buf);
        city->route_id = buffer_read_u8(buf);
        city->is_open = buffer_read_u8(buf);
        for (int r = 0; r < RESOURCE_MAX; r++) {
            city->buys_resource[r] = buffer_read_u8(buf);
        }
        for (int r = 0; r < RESOURCE_MAX; r++) {
            city->sells_resource[r] = buffer_read_u8(buf);
        }
        city->cost_to_open = buffer_read_i16(buf);
        buffer_skip(buf, 2);
        city->trader_entry_delay = buffer_read_i16(buf);
        buffer_skip(buf, 2);
        city->empire_object_id = buffer_read_i16(buf);
        city->is_sea_trade = buffer_read_u8(buf);
        buffer_skip(buf, 1);
        for (int f = 0; f < 3; f++) {
            city->trader_figure_ids[f] = buffer_read_i16(buf);
        }
        buffer_skip(buf, 10);
    }
}

