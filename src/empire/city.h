#ifndef EMPIRE_CITY_H
#define EMPIRE_CITY_H

#include "core/buffer.h"
#include "game/resource.h"
#include "type.h"

typedef struct
{
    int in_use;
    int type;
    int name_id;
    int route_id;
    int is_open;
    int buys_resource[RESOURCE_MAX];
    int sells_resource[RESOURCE_MAX];
    int cost_to_open;
    int trader_entry_delay;
    int empire_object_id;
    int is_sea_trade;
    int trader_figure_ids[3];
} empire_city;

void empire_city_clear_all();

empire_city * empire_city_get(int city_id);

int empire_city_get_route_id(int city_id);

int empire_can_import_resource(int resource);

int empire_can_import_resource_potentially(int resource);

int empire_city_can_export_resource(int resource);

int empire_city_can_produce_ourselves(int resource);

int empire_city_get_for_object(int empire_object_id);

int empire_city_get_for_trade_route(int route_id);

int empire_city_is_trade_route_open(int route_id);

void empire_city_reset_yearly_trade_amounts();

int empire_city_count_wine_sources();

int empire_city_get_vulnerable_roman();

void empire_city_expand_empire();

void empire_city_set_vulnerable(int city_id);

void empire_city_set_foreign(int city_id);

void empire_city_foreach_open_until(int (*until_func)(int, empire_city*));

void empire_city_remove_trader(int city_id, int figure_id);

void empire_city_save_state(buffer *buf);

void empire_city_load_state(buffer *buf);

int empire_can_produce_resource(int resource);

int empire_can_produce_resource_potentially(int resource);

#endif // EMPIRE_CITY_H
