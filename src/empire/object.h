#ifndef EMPIRE_OBJECT_H
#define EMPIRE_OBJECT_H

#include "core/buffer.h"

typedef struct {
    int id;
    int in_use;
    int type;
    int animation_index;
    int x;
    int y;
    int width;
    int height;
    int image_id;
    struct {
        int x;
        int y;
        int image_id;
    } expanded;
    int distant_battle_travel_months;
    // city-related
    int city_type;
    int city_name_id;
    int trade_route_id;
    int trade_route_open;
    int trade_route_cost;
    int city_sells_resource[10];
    int city_buys_resource[8];
    int owner_city_object;
    int invasion_path_id;
    int invasion_years;
    int trade40;
    int trade25;
    int trade15;
} empire_object;

void empire_object_load(buffer *buf);

void empire_object_init_trade_amounts();

void empire_object_init_cities();

int empire_object_init_distant_battle_travel_months(int object_type);

const empire_object *empire_object_get(int object_id);

void empire_object_foreach(void (*callback)(const empire_object *));

const empire_object *empire_object_get_battle_icon(int path_id, int year);

int empire_object_get_max_invasion_path();

int empire_object_get_closest(int x, int y);

void empire_object_set_expanded(int object_id, int new_city_type);

void empire_object_update_animation(int object_id, int new_animation_index);

#endif // EMPIRE_OBJECT_H
