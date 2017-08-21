#include "loki/loki.h"
#include "empire/trade_prices.h"

#include "mocks/buffer.h"

CREATE_BUFFER_MOCKS

INIT_MOCKS(
    INIT_BUFFER_MOCKS
)

buffer buf;

void test_trade_price_reset()
{
    trade_prices_reset();
    
    assert_eq(180, trade_price_buy(RESOURCE_POTTERY));
}

void test_trade_price_change_up()
{
    trade_prices_reset();
    
    // pottery = 180/140 default
    int changed = trade_price_change(RESOURCE_POTTERY, 20);
    
    assert_true(changed);
    assert_eq(200, trade_price_buy(RESOURCE_POTTERY));
    assert_eq(160, trade_price_sell(RESOURCE_POTTERY));
}

void test_trade_price_change_down()
{
    trade_prices_reset();
    
    // iron = 60/40 default
    int changed = trade_price_change(RESOURCE_IRON, -20);
    
    assert_true(changed);
    assert_eq(40, trade_price_buy(RESOURCE_IRON));
    assert_eq(20, trade_price_sell(RESOURCE_IRON));
}

void test_trade_price_change_down_to_zero()
{
    trade_prices_reset();
    
    // iron = 60/40 default
    int changed = trade_price_change(RESOURCE_IRON, -40);
    
    assert_true(changed);
    assert_eq(2, trade_price_buy(RESOURCE_IRON));
    assert_eq(0, trade_price_sell(RESOURCE_IRON));
}

void test_trade_price_change_down_too_much()
{
    trade_prices_reset();
    
    // iron = 60/40 default
    trade_price_change(RESOURCE_IRON, -40);
    int changed = trade_price_change(RESOURCE_IRON, -1);
    
    assert_false(changed);
    assert_eq(2, trade_price_buy(RESOURCE_IRON));
    assert_eq(0, trade_price_sell(RESOURCE_IRON));
}

void test_trade_prices_save()
{
    trade_prices_reset();
    
    buffer buf;
    trade_prices_save_state(&buf);
    
    verify_buffer_write_i32_times(32); // 16 resources x {buy,sell}
    // verify a few random prices
    verify_buffer_write_i32(&buf, 140);
    verify_buffer_write_i32(&buf, 34);
    verify_buffer_write_i32(&buf, 250);
}

void test_trade_prices_load()
{
    buffer buf;
    when_buffer_read_i32(&buf)->then_return = 1;
    
    trade_prices_load_state(&buf);
    
    verify_buffer_read_i32_times(32);
    assert_eq(1, trade_price_buy(RESOURCE_CLAY));
    assert_eq(1, trade_price_sell(RESOURCE_CLAY));
}

RUN_TESTS(empire.trade_prices,
    ADD_TEST(test_trade_price_reset)
    ADD_TEST(test_trade_price_change_up)
    ADD_TEST(test_trade_price_change_down)
    ADD_TEST(test_trade_price_change_down_to_zero)
    ADD_TEST(test_trade_price_change_down_too_much)
    ADD_TEST(test_trade_prices_save)
    ADD_TEST(test_trade_prices_load)
)
