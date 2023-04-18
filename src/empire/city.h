#ifndef EMPIRE_CITY_H
#define EMPIRE_CITY_H

#include "core/buffer.h"
#include "empire/type.h"
#include "game/resource.h"

#define EMPIRE_CITY_MAX_TRADERS 3

typedef struct {
    int in_use;
    empire_city_type type;
    int name_id;
    int route_id;
    int is_open;
    int buys_resource[RESOURCE_MAX];
    int sells_resource[RESOURCE_MAX];
    int cost_to_open;
    int trader_entry_delay;
    int empire_object_id;
    int is_sea_trade;
    int trader_figure_ids[EMPIRE_CITY_MAX_TRADERS];
} empire_city;

void empire_city_clear_all(void);

empire_city *empire_city_get(int city_id);

empire_city *empire_city_get_new(void);

int empire_city_get_route_id(int city_id);

int empire_city_get_id_by_name(const uint8_t *city_name);

int empire_can_import_resource(int resource);

int empire_can_import_resource_potentially(int resource);

int empire_has_access_to_resource(int resource);

int empire_can_export_resource_potentially(int resource);

int empire_can_export_resource(int resource);

int can_produce_resource(int resource);

int empire_can_produce_resource(int resource);

int empire_can_produce_resource_potentially(int resource);

int empire_city_get_for_object(int empire_object_id);

int empire_city_get_for_trade_route(int route_id);

int empire_city_is_trade_route_open(int route_id);

void empire_city_reset_yearly_trade_amounts(void);

int empire_city_count_wine_sources(void);

int empire_city_get_vulnerable_roman(void);

void empire_city_expand_empire(void);

void empire_city_force_sell(int route, int resource);

void empire_city_set_vulnerable(int city_id);

void empire_city_set_foreign(int city_id);

void empire_city_open_trade(int city_id);

void empire_city_generate_trader(void);

void empire_city_remove_trader(int city_id, int figure_id);

int empire_unlock_all_resources(void);

int empire_city_change_own_resource_availability(resource_type resource, int is_available);

const uint8_t *empire_city_get_name(const empire_city *city);

void empire_city_save_state(buffer *buf);

int empire_city_can_mine_gold(int city_name_id);

void empire_city_update_gold_trading(void);

void empire_city_load_state(buffer *buf, int version);

int empire_city_get_array_size(void);

#endif // EMPIRE_CITY_H
