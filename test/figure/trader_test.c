#include "loki/loki.h"
#include "figure/trader.h"

#include "mocks/buffer.h"

CREATE_BUFFER_MOCKS
CREATE_MOCK1(int, trade_price_buy, resource_type)
CREATE_MOCK1(int, trade_price_sell, resource_type)

void setup()
{
    traders_clear();
}

INIT_MOCKS(
    INIT_BUFFER_MOCKS
    INIT_MOCK(trade_price_buy)
    INIT_MOCK(trade_price_sell)
    SETUP(setup)
)

buffer buf;

void test_traders_clear()
{
    trader_record_bought_resource(1, RESOURCE_WHEAT);

    traders_clear();
    
    assert_eq(0, trader_bought_resources(1, RESOURCE_WHEAT));
}

void test_trader_create()
{
    for (int i = 0; i < 100; i++) {
        assert_eq(i, trader_create());
    }
    assert_eq(0, trader_create());
}

void test_trader_has_traded()
{
    trader_record_bought_resource(1, RESOURCE_CLAY);
    trader_record_sold_resource(2, RESOURCE_FRUIT);
    
    assert_false(trader_has_traded(0));
    assert_true(trader_has_traded(1));
    assert_true(trader_has_traded(2));
}

void test_trader_has_traded_max()
{
    for (int i = 1; i <= 11; i++) {
        trader_record_bought_resource(0, i);
    }
    for (int i = 1; i <= 12; i++) {
        trader_record_bought_resource(1, i);
    }
    for (int i = 1; i <= 12; i++) {
        trader_record_sold_resource(2, i);
    }
    
    assert_false(trader_has_traded_max(0));
    assert_true(trader_has_traded_max(1));
    assert_true(trader_has_traded_max(2));
}

void test_trader_record_resource()
{
    trader_record_bought_resource(1, RESOURCE_FURNITURE);
    trader_record_bought_resource(1, RESOURCE_FURNITURE);
    trader_record_sold_resource(1, RESOURCE_POTTERY);
    
    assert_eq(2, trader_bought_resources(1, RESOURCE_FURNITURE));
    assert_eq(1, trader_sold_resources(1, RESOURCE_POTTERY));
}

void test_traders_save_state()
{
    traders_save_state(&buf);

    verify_buffer_write_u8_times(3200); // 100x 32 resources
    verify_buffer_write_i32_times(401); // 100x 4 fields + index
}

void test_traders_load_state()
{
    traders_load_state(&buf);
    
    verify_buffer_read_u8_times(3200);
    verify_buffer_read_i32_times(401);
}

RUN_TESTS(figure.trader,
    ADD_TEST(test_traders_clear)
    ADD_TEST(test_trader_create)
    ADD_TEST(test_trader_record_resource)
    ADD_TEST(test_trader_has_traded)
    ADD_TEST(test_trader_has_traded_max)
    ADD_TEST(test_traders_save_state)
    ADD_TEST(test_traders_load_state)
)
