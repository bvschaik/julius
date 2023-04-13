#include "building/caravanserai.h"
#include "building/lighthouse.h"
#include "building/model.h"
#include "building/monument.h"
#include "city/buildings.h"
#include "city/resource.h"
#include "city/trade_policy.h"
#include "core/calc.h"
#include "trade_prices.h"

struct trade_price {
    int32_t buy;
    int32_t sell;
};

static struct trade_price prices[RESOURCE_MAX];

static int trade_percentage_from_laborers(int percent, building *b)
{
    int percent_laborers = 0;
    // get workers percentage
    int pct_workers = calc_percentage(b->num_workers, model_get_building(b->type)->laborers);
    if (pct_workers >= 100) { // full laborers
        percent_laborers = percent;
    } else if (pct_workers > 0) {
        percent_laborers = percent / 2;
    }
    return percent_laborers;
}

static int trade_get_caravanserai_factor(int percent)
{
    int caravanserai_percent = 0;
    if (building_caravanserai_is_fully_functional()) {
        building *b = building_get(city_buildings_get_caravanserai());
        caravanserai_percent = trade_percentage_from_laborers(percent, b);
    }
    return caravanserai_percent;
}

static int trade_get_lighthouse_factor(int percent)
{
    int lighthouse_percent = 0;

    if (building_lighthouse_is_fully_functional()) {
        building *b = building_get(building_find(BUILDING_LIGHTHOUSE));
        lighthouse_percent = trade_percentage_from_laborers(percent, b);
    }
    return lighthouse_percent;
}

static int trade_factor_sell(int land_trader)
{
    int percent = 0;
    if (land_trader && city_buildings_has_caravanserai()) {
        trade_policy policy = city_trade_policy_get(LAND_TRADE_POLICY);

        if (policy == TRADE_POLICY_1) {
            percent = trade_get_caravanserai_factor(POLICY_1_BONUS_PERCENT); // trader buy 20% more
        } else if (policy == TRADE_POLICY_2) {
            percent -= trade_get_caravanserai_factor(POLICY_2_MALUS_PERCENT); // trader buy 10% less
        }
    } else if (!land_trader && building_find(BUILDING_LIGHTHOUSE)) {
        trade_policy policy = city_trade_policy_get(SEA_TRADE_POLICY);

        if (policy == TRADE_POLICY_1) {
            percent = trade_get_lighthouse_factor(POLICY_1_BONUS_PERCENT); // trader buy 20% more
        } else if (policy == TRADE_POLICY_2) {
            percent -= trade_get_lighthouse_factor(POLICY_2_MALUS_PERCENT); // trader buy 10% less
        }
    }
    return percent;
}

static int trade_factor_buy(int land_trader)
{
    int percent = 0;
    if (land_trader && city_buildings_has_caravanserai()) {
        trade_policy policy = city_trade_policy_get(LAND_TRADE_POLICY);

        if (policy == TRADE_POLICY_1) {
            percent = trade_get_caravanserai_factor(POLICY_1_MALUS_PERCENT); // player buy 10% more
        } else if (policy == TRADE_POLICY_2) {
            percent -= trade_get_caravanserai_factor(POLICY_2_BONUS_PERCENT); // player buy 20% less
        }
    } else if (!land_trader && building_find(BUILDING_LIGHTHOUSE)) {
        trade_policy policy = city_trade_policy_get(SEA_TRADE_POLICY);

        if (policy == TRADE_POLICY_1) {
            percent = trade_get_lighthouse_factor(POLICY_1_MALUS_PERCENT); // player buy 10% more
        } else if (policy == TRADE_POLICY_2) {
            percent -= trade_get_lighthouse_factor(POLICY_2_BONUS_PERCENT); // player buy 20% less
        }
    }
    return percent;
}

void trade_prices_reset(void)
{
    for (int i = 0; i < RESOURCE_MAX; i++) {
        prices[i].buy = resource_get_data(i)->default_trade_price.buy;
        prices[i].sell = resource_get_data(i)->default_trade_price.sell;
    }
}

int trade_price_base_buy(resource_type resource)
{
    return prices[resource].buy;
}

int trade_price_buy(resource_type resource, int land_trader)
{
    return calc_adjust_with_percentage(prices[resource].buy, 100 + trade_factor_buy(land_trader));
}

int trade_price_base_sell(resource_type resource)
{
    return prices[resource].sell;
}

int trade_price_sell(resource_type resource, int land_trader)
{
    return calc_adjust_with_percentage(prices[resource].sell, 100 + trade_factor_sell(land_trader));
}

int trade_price_change(resource_type resource, int amount)
{
    if (amount < 0 && prices[resource].sell <= 0) {
        // cannot lower the price to negative
        return 0;
    }
    if (amount < 0 && prices[resource].sell <= -amount) {
        prices[resource].buy = 2;
        prices[resource].sell = 0;
    } else {
        prices[resource].buy += amount;
        prices[resource].sell += amount;
    }
    return 1;
}

void trade_prices_save_state(buffer *buf)
{
    for (int i = 0; i < RESOURCE_MAX; i++) {
        buffer_write_i32(buf, prices[i].buy);
        buffer_write_i32(buf, prices[i].sell);
    }
}

void trade_prices_load_state(buffer *buf)
{
    trade_prices_reset();
    for (int i = 0; i < resource_total_mapped(); i++) {
        prices[resource_remap(i)].buy = buffer_read_i32(buf);
        prices[resource_remap(i)].sell = buffer_read_i32(buf);
    }
}
