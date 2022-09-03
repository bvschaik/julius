#ifndef FIGURETYPE_TRADER_H
#define FIGURETYPE_TRADER_H

#include "figure/figure.h"

enum {
    TRADE_SHIP_NONE = 0,
    TRADE_SHIP_BUYING = 1,
    TRADE_SHIP_SELLING = 2,
};

int figure_create_trade_caravan(int x, int y, int city_id);

int figure_create_trade_ship(int x, int y, int city_id);

int figure_trade_caravan_can_buy(figure *trader, int warehouse_id, int city_id);

int figure_trade_caravan_can_sell(figure *trader, int warehouse_id, int city_id);

void figure_trade_caravan_action(figure *f);

void figure_trade_caravan_donkey_action(figure *f);

void figure_native_trader_action(figure *f);

int figure_trade_ship_is_trading(figure *ship);

void figure_trade_ship_action(figure *f);

int figure_trade_land_trade_units();

int figure_trade_sea_trade_units();

int figure_trader_ship_docked_once_at_dock(figure *ship, int dock_id);

int figure_trader_ship_already_docked_at(figure *ship, int dock_num);

int figure_trader_ship_can_queue_for_import(figure *ship);

int figure_trader_ship_can_queue_for_export(figure *ship);

int figure_trader_ship_get_distance_to_dock(const figure *ship, int dock_id);

int figure_trader_ship_other_ship_closer_to_dock(int ship_id, int dock_id, int distance);

#define IMAGE_CAMEL 4922

#endif // FIGURETYPE_TRADER_H
