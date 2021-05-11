#ifndef CITY_TRADE_POLICY_H
#define CITY_TRADE_POLICY_H

#define TRADE_POLICY_COST 500
#define POLICY_1_BONUS_PERCENT 20
#define POLICY_1_MALUS_PERCENT 10
#define POLICY_2_BONUS_PERCENT 20
#define POLICY_2_MALUS_PERCENT 10
#define POLICY_3_BONUS 4
#define POLICY_3_MALUS_PERCENT 20

typedef enum {
    LAND_TRADE_POLICY,
    SEA_TRADE_POLICY
} trade_policy_type;

typedef enum {
    NO_POLICY,
    TRADE_POLICY_1,
    TRADE_POLICY_2,
    TRADE_POLICY_3
} trade_policy;

trade_policy city_trade_policy_get(trade_policy_type type);
void city_trade_policy_set(trade_policy_type type, trade_policy policy);

#endif // CITY_TRADE_POLICY_H
