#include "trade_policy.h"

#include "city/data_private.h"

trade_policy city_trade_policy_get(trade_policy_type type)
{
    switch (type) {
        case LAND_TRADE_POLICY:
            return city_data.trade.land_policy;
        case SEA_TRADE_POLICY:
            return city_data.trade.sea_policy;
    }
    return NO_POLICY;
}

void city_trade_policy_set(trade_policy_type type, trade_policy policy)
{
    switch (type) {
        case LAND_TRADE_POLICY:
            city_data.trade.land_policy = policy;
            break;
        case SEA_TRADE_POLICY:
            city_data.trade.sea_policy = policy;
            break;
    }
}
