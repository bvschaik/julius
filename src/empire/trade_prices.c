#include "trade_prices.h"

struct trade_price {
    int32_t buy;
    int32_t sell;
};

static const struct trade_price DEFAULT_PRICES[16] = {
    {0, 0}, {28, 22}, {38, 30}, {38, 30}, // wheat, vegetables, fruit
    {42, 34}, {44, 36}, {44, 36}, {215, 160}, // olives, vines, meat, wine
    {180, 140}, {60, 40}, {50, 35}, {40, 30}, // oil, iron, timber, clay
    {200, 140}, {250, 180}, {200, 150}, {180, 140} // marble, weapons, furniture, pottery
};

static struct trade_price prices[16];

void trade_prices_reset(void)
{
    for (int i = 0; i < 16; i++) {
        prices[i] = DEFAULT_PRICES[i];
    }
}

int trade_price_buy(resource_type resource)
{
    return prices[resource].buy;
}

int trade_price_sell(resource_type resource)
{
    return prices[resource].sell;
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
    for (int i = 0; i < 16; i++) {
        buffer_write_i32(buf, prices[i].buy);
        buffer_write_i32(buf, prices[i].sell);
    }
}

void trade_prices_load_state(buffer *buf)
{
    for (int i = 0; i < 16; i++) {
        prices[i].buy = buffer_read_i32(buf);
        prices[i].sell = buffer_read_i32(buf);
    }
}

