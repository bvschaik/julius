#ifndef FIGURETYPE_MARKET_H
#define FIGURETYPE_MARKET_H

#include "figure/figure.h"

#define MAX_FOOD_STOCKED_MARKET 800
#define MAX_FOOD_STOCKED_MESS_HALL 1600

int figure_market_create_delivery_boy(int leader_id, figure* f, int type);

void figure_market_buyer_action(figure *f);

void figure_delivery_boy_action(figure *f);

#endif // FIGURETYPE_MARKET_H
