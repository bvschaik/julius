#include "loki/loki.h"

#include "core/calc.h"

NO_MOCKS()

void test_calc_adjust_with_percentage()
{
    assert_eq(80, calc_adjust_with_percentage(100, 80));
    assert_eq(80, calc_adjust_with_percentage(80, 100));
    assert_eq(0, calc_adjust_with_percentage(100, 0));
    assert_eq(0, calc_adjust_with_percentage(0, 200));
}

void test_calc_percentage()
{
    assert_eq(80, calc_percentage(80, 100));
    assert_eq(200, calc_percentage(2, 1));
    assert_eq(50, calc_percentage(509999, 1000000));
    assert_eq(51, calc_percentage(510000, 1000000));
    assert_eq(0, calc_percentage(10000, 0));
}

void test_calc_total_distance()
{
    assert_eq(0, calc_total_distance(2, 2, 2, 2));
    assert_eq(2, calc_total_distance(1, 1, 2, 2));
    assert_eq(2, calc_total_distance(2, 2, 1, 1));
}

void test_calc_maximum_distance()
{
    assert_eq(0, calc_maximum_distance(1, 1, 1, 1));
    assert_eq(5, calc_maximum_distance(1, 2, 6, 4));
    assert_eq(5, calc_maximum_distance(6, 4, 1, 2));
    assert_eq(5, calc_maximum_distance(4, 6, 2, 1));
}

void test_calc_minimum_distance()
{
    assert_eq(0, calc_minimum_distance(1, 1, 1, 1));
    assert_eq(2, calc_minimum_distance(1, 2, 6, 4));
    assert_eq(2, calc_minimum_distance(6, 4, 1, 2));
    assert_eq(2, calc_minimum_distance(4, 6, 2, 1));
}

void test_calc_bound()
{
    assert_eq(0, calc_bound(-1, 0, 100));
    assert_eq(100, calc_bound(1000, 0, 100));
    assert_eq(50, calc_bound(50, 0, 100));
}

RUN_TESTS(core/calc,
    ADD_TEST(test_calc_adjust_with_percentage)
    ADD_TEST(test_calc_percentage)
    ADD_TEST(test_calc_total_distance)
    ADD_TEST(test_calc_maximum_distance)
    ADD_TEST(test_calc_minimum_distance)
    ADD_TEST(test_calc_bound)
)
