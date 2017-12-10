#include "loki/loki.h"
#include "game/time.h"

#include "mocks/buffer.h"

CREATE_BUFFER_MOCKS

INIT_MOCKS(
    INIT_BUFFER_MOCKS
)

void test_game_time_init()
{
    game_time_init(0);
    game_time_advance_tick();
    game_time_advance_day();
    game_time_advance_month();
    game_time_advance_year();
    
    game_time_init(100);
    
    assert_eq(0, game_time_tick());
    assert_eq(0, game_time_day());
    assert_eq(0, game_time_month());
    assert_eq(100, game_time_year());
}

void test_game_time_advance_tick()
{
    game_time_init(0);
    
    for (int i = 1; i < 50; i++) {
        assert_false(game_time_advance_tick());
        assert_eq(i, game_time_tick());
    }
    
    assert_true(game_time_advance_tick());
    assert_eq(0, game_time_tick());
}

void test_game_time_advance_day()
{
    game_time_init(0);
    
    for (int i = 1; i < 16; i++) {
        assert_false(game_time_advance_day());
        assert_eq(i, game_time_day());
    }
    
    assert_true(game_time_advance_day());
    assert_eq(0, game_time_day());
}

void test_game_time_advance_month()
{
    game_time_init(0);
    
    for (int i = 1; i < 12; i++) {
        assert_false(game_time_advance_month());
        assert_eq(i, game_time_month());
    }
    
    assert_true(game_time_advance_month());
    assert_eq(0, game_time_month());
}

void test_game_time_advance_year()
{
    game_time_init(1234);
    
    game_time_advance_year();
    
    assert_eq(1235, game_time_year());
}

void test_game_time_save()
{
    game_time_init(4);
    game_time_advance_tick();
    game_time_advance_day();
    game_time_advance_day();
    game_time_advance_month();
    game_time_advance_month();
    game_time_advance_month();
    
    buffer buf;
    game_time_save_state(&buf);
    
    verify_buffer_write_i32_times(5);
    verify_buffer_write_i32(&buf, 1);
    verify_buffer_write_i32(&buf, 2);
    verify_buffer_write_i32(&buf, 3);
    verify_buffer_write_i32(&buf, 4);
}

void test_game_time_load()
{
    buffer buf;
    when_buffer_read_i32(&buf)->then_return = 123;
    
    game_time_load_state(&buf);

    assert_eq(123, game_time_day());
    verify_buffer_read_i32_times(5);
}

RUN_TESTS(game.time,
    ADD_TEST(test_game_time_init)
    ADD_TEST(test_game_time_advance_tick)
    ADD_TEST(test_game_time_advance_day)
    ADD_TEST(test_game_time_advance_month)
    ADD_TEST(test_game_time_advance_year)
    ADD_TEST(test_game_time_save)
    ADD_TEST(test_game_time_load)
)
