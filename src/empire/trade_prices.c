#include "building/caravanserai.h"
#include "building/model.h"
#include "building/monument.h"
#include "city/buildings.h"
#include "city/resource.h"
#include "core/calc.h"
#include "trade_prices.h"

struct trade_price {
    int32_t buy;
    int32_t sell;
};

static const struct trade_price DEFAULT_PRICES[RESOURCE_MAX] = {
    {0, 0}, {28, 22}, {38, 30}, {38, 30}, // wheat, vegetables, fruit
    {42, 34}, {44, 36}, {44, 36}, {215, 160}, // olives, vines, meat, wine
    {180, 140}, {60, 40}, {50, 35}, {40, 30}, // oil, iron, timber, clay
    {200, 140}, {250, 180}, {200, 150}, {180, 140} // marble, weapons, furniture, pottery
};

static struct trade_price prices[RESOURCE_MAX];

static int trade_get_caravanserai_factor(int percent) {
    int caravanserai_percent = 0;

    if (building_monument_working(BUILDING_CARAVANSERAI)) {
        building *b = building_get(city_buildings_get_caravanserai());

        // caravanserai has enough food for the month
        if (building_caravanserai_enough_foods(b)) {
            // get workers percentage
            int pct_workers = calc_percentage(b->num_workers, model_get_building(b->type)->laborers);
            if (pct_workers >= 100) { // full laborers
                caravanserai_percent = percent;
            } else if (pct_workers > 0) {
                caravanserai_percent = caravanserai_percent / 2;
            }
        }
    }

    return caravanserai_percent;
}

static int trade_factor_sell(int land_trader)
{
    int percent = 0;
    if (land_trader && city_buildings_has_caravanserai()) {
        int policy = building_monument_module_type(BUILDING_CARAVANSERAI);

        if (policy == TRADE_POLICY_1) {
            percent = trade_get_caravanserai_factor(POLICY_1_BONUS_PERCENT); // trader buy 20% more
        } else if (policy == TRADE_POLICY_2) {
            percent = - trade_get_caravanserai_factor(POLICY_2_MALUS_PERCENT); // trader buy 10% less
        }
    }
    return percent;
}

static int trade_factor_buy(int land_trader)
{
    int percent = 0;
    if (land_trader && city_buildings_has_caravanserai()) {
        int policy = building_monument_module_type(BUILDING_CARAVANSERAI);

        if (policy == TRADE_POLICY_1) {
            percent = - trade_get_caravanserai_factor(POLICY_1_MALUS_PERCENT); // player buy 10% more
        } else if (policy == TRADE_POLICY_2) {
            percent = trade_get_caravanserai_factor(POLICY_2_BONUS_PERCENT); // player buy 20% less
        }
    }
    return percent;
}

void trade_prices_reset(void)
{
    for (int i = 0; i < RESOURCE_MAX; i++) {
        prices[i] = DEFAULT_PRICES[i];
    }
}

int trade_price_buy(resource_type resource, int land_trader)
{
    return calc_adjust_with_percentage(prices[resource].buy, 100 + trade_factor_buy(land_trader));
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
    for (int i = 0; i < RESOURCE_MAX; i++) {
        prices[i].buy = buffer_read_i32(buf);
        prices[i].sell = buffer_read_i32(buf);
    }
}
