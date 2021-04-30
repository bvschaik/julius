#ifndef CITY_TRADE_POLICY_H
#define CITY_TRADE_POLICY_H

#include "window/building/common.h"

#define TRADE_POLICY_COST 500
#define POLICY_1_BONUS_PERCENT 20
#define POLICY_1_MALUS_PERCENT 10
#define POLICY_2_BONUS_PERCENT 20
#define POLICY_2_MALUS_PERCENT 10
#define POLICY_3_BONUS_PERCENT 50
#define POLICY_3_MALUS_PERCENT 15

typedef enum {
    NO_POLICY,
    TRADE_POLICY_1,
    TRADE_POLICY_2,
    TRADE_POLICY_3
} trade_policy;

void window_building_draw_policy_action(building_info_context *c, int land_policy);
void window_policy_show(int land_policy);

#endif //CITY_TRADE_POLICY_H
