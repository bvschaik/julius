#include "figure/trader.h"

#include "empire/trade_prices.h"
#include "figuretype/trader.h"
#include "trader.h"
#include <string.h>

#define MAX_TRADERS 100

struct trader {
    int32_t bought_amount;
    int32_t bought_value;
    uint8_t bought_resources[RESOURCE_MAX];

    int32_t sold_amount;
    int32_t sold_value;
    uint8_t sold_resources[RESOURCE_MAX];
};

static struct {
    struct trader traders[MAX_TRADERS];
    int next_index;
} data;

void traders_clear(void)
{
    memset(&data, 0, sizeof(data));
}

int trader_create(void)
{
    int trader_id = data.next_index++;
    if (data.next_index >= MAX_TRADERS) {
        data.next_index = 0;
    }

    memset(&data.traders[trader_id], 0, sizeof(struct trader));
    return trader_id;
}

void trader_record_bought_resource(int trader_id, resource_type resource)
{
    figure *f = figure_get(trader_id);
    int is_land_trader = f->type == FIGURE_TRADE_CARAVAN || f->type == FIGURE_TRADE_CARAVAN_DONKEY || f->type == FIGURE_NATIVE_TRADER;

    data.traders[trader_id].bought_amount++;
    data.traders[trader_id].bought_resources[resource]++;
    data.traders[trader_id].bought_value += trade_price_sell(resource, is_land_trader);
}

void trader_record_sold_resource(int trader_id, resource_type resource)
{
    figure *f = figure_get(trader_id);
    int is_land_trader = f->type == FIGURE_TRADE_CARAVAN || f->type == FIGURE_TRADE_CARAVAN_DONKEY || f->type == FIGURE_NATIVE_TRADER;

    data.traders[trader_id].sold_amount++;
    data.traders[trader_id].sold_resources[resource]++;
    data.traders[trader_id].sold_value += trade_price_buy(resource, is_land_trader);
}

int trader_bought_resources(int trader_id, resource_type resource)
{
    return data.traders[trader_id].bought_resources[resource];
}

int trader_sold_resources(int trader_id, resource_type resource)
{
    return data.traders[trader_id].sold_resources[resource];
}

int trader_has_traded(int trader_id)
{
    return data.traders[trader_id].bought_amount || data.traders[trader_id].sold_amount;
}

int trader_has_traded_max(int trader_id)
{
    return trader_has_bought_max(trader_id) || trader_has_sold_max(trader_id);
}

int trader_has_bought_max(int trader_id)
{
    return data.traders[trader_id].bought_amount >= figure_trade_sea_trade_units();
}

int trader_has_sold_max(int trader_id)
{
    return data.traders[trader_id].sold_amount >= figure_trade_sea_trade_units();
}

void traders_save_state(buffer *buf)
{
    for (int i = 0; i < MAX_TRADERS; i++) {
        struct trader *t = &data.traders[i];
        buffer_write_i32(buf, t->bought_amount);
        buffer_write_i32(buf, t->sold_amount);
        for (int r = 0; r < RESOURCE_MAX; r++) {
            buffer_write_u8(buf, t->bought_resources[r]);
        }
        for (int r = 0; r < RESOURCE_MAX; r++) {
            buffer_write_u8(buf, t->sold_resources[r]);
        }
        buffer_write_i32(buf, t->bought_value);
        buffer_write_i32(buf, t->sold_value);
    }
    buffer_write_i32(buf, data.next_index);
}

void traders_load_state(buffer *buf)
{
    traders_clear();
    for (int i = 0; i < MAX_TRADERS; i++) {
        struct trader *t = &data.traders[i];
        t->bought_amount = buffer_read_i32(buf);
        t->sold_amount = buffer_read_i32(buf);
        for (int r = 0; r < resource_total_mapped(); r++) {
            t->bought_resources[resource_remap(r)] = buffer_read_u8(buf);
        }
        for (int r = 0; r < resource_total_mapped(); r++) {
            t->sold_resources[resource_remap(r)] = buffer_read_u8(buf);
        }
        t->bought_value = buffer_read_i32(buf);
        t->sold_value = buffer_read_i32(buf);
    }
    data.next_index = buffer_read_i32(buf);
}
