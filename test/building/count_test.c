#include "loki/loki.h"
#include "building/count.h"

#include "mocks/buffer.h"

CREATE_BUFFER_MOCKS

void setup()
{
    building_count_clear();
}

INIT_MOCKS(
    INIT_BUFFER_MOCKS
    SETUP(setup)
)

void test_building_count_increase()
{
    building_count_increase(BUILDING_ACADEMY, 0);
    building_count_increase(BUILDING_ACADEMY, 1);
    building_count_increase(BUILDING_AQUEDUCT, 0);
    
    assert_eq(1, building_count_active(BUILDING_ACADEMY));
    assert_eq(2, building_count_total(BUILDING_ACADEMY));
    assert_eq(0, building_count_active(BUILDING_AQUEDUCT));
    assert_eq(1, building_count_total(BUILDING_AQUEDUCT));
}

void test_building_count_industry_increase()
{
    building_count_industry_increase(RESOURCE_CLAY, 0);
    building_count_industry_increase(RESOURCE_CLAY, 1);
    building_count_industry_increase(RESOURCE_FURNITURE, 0);
    
    assert_eq(1, building_count_industry_active(RESOURCE_CLAY));
    assert_eq(2, building_count_industry_total(RESOURCE_CLAY));
    assert_eq(0, building_count_industry_active(RESOURCE_FURNITURE));
    assert_eq(1, building_count_industry_total(RESOURCE_FURNITURE));
}

void test_building_count_limit_hippodrome()
{
    building_count_limit_hippodrome();
    
    assert_eq(0, building_count_active(BUILDING_HIPPODROME));
    assert_eq(0, building_count_total(BUILDING_HIPPODROME));

    building_count_increase(BUILDING_HIPPODROME, 1);
    building_count_increase(BUILDING_HIPPODROME, 0);
    building_count_increase(BUILDING_HIPPODROME, 1);
    
    building_count_limit_hippodrome();
    
    assert_eq(1, building_count_active(BUILDING_HIPPODROME));
    assert_eq(1, building_count_total(BUILDING_HIPPODROME));
}

void test_building_count_save()
{
    buffer ind, mil, sup, cu1, cu2, cu3;
    
    building_count_save_state(&ind, &cu1, &cu2, &cu3, &mil, &sup);
    
    assert_eq(32, count_buffer_write_i32(&ind, 0));
    assert_eq(33, count_buffer_write_i32(&cu1, 0));
    assert_eq(8, count_buffer_write_i32(&cu2, 0));
    assert_eq(10, count_buffer_write_i32(&cu3, 0));
    assert_eq(4, count_buffer_write_i32(&mil, 0));
    assert_eq(6, count_buffer_write_i32(&sup, 0));
}

void test_building_count_load()
{
    buffer ind, mil, sup, cu1, cu2, cu3;
    
    building_count_load_state(&ind, &cu1, &cu2, &cu3, &mil, &sup);
    
    assert_eq(32, count_buffer_read_i32(&ind));
    assert_eq(33, count_buffer_read_i32(&cu1));
    assert_eq(8, count_buffer_read_i32(&cu2));
    assert_eq(10, count_buffer_read_i32(&cu3));
    assert_eq(4, count_buffer_read_i32(&mil));
    assert_eq(6, count_buffer_read_i32(&sup));
}

RUN_TESTS(empire.trade_prices,
    ADD_TEST(test_building_count_increase)
    ADD_TEST(test_building_count_industry_increase)
    ADD_TEST(test_building_count_limit_hippodrome)
    ADD_TEST(test_building_count_save)
    ADD_TEST(test_building_count_load)
)
