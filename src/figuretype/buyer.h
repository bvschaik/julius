#ifndef FIGURETYPE_BUYER_H
#define FIGURETYPE_BUYER_H

#include "figure/figure.h"

#define MAX_FOOD_STOCKED_MARKET 800
#define MAX_FOOD_STOCKED_MESS_HALL 1600

int figure_buyer_create_delivery_boy(int leader_id, figure *f, int type);

void figure_buyer_action(figure *f);

void figure_delivery_boy_action(figure *f);

#endif // FIGURETYPE_BUYER_H
