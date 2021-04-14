#ifndef BUILDING_CARANVASERAI_H
#define BUILDING_CARANVASERAI_H

#include "building/building.h"

#define TRADE_POLICY_COST 500
#define MAX_FOOD 1200
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

int building_caravanserai_enough_foods(building *caravanserai);
int building_caravanserai_get_storage_destination(building *caravanserai);

#endif // BUILDING_CARANVASERAI_H
