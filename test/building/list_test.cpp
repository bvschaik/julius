#include "loki/loki.h"
#include "building/list.hpp"

#include "mocks/buffer.h"

CREATE_BUFFER_MOCKS

void setup()
{
    building_list_small_clear();
    building_list_large_clear(0);
}

INIT_MOCKS(
    INIT_BUFFER_MOCKS
    SETUP(setup)
)

void test_building_list_small_add()
{
    building_list_small_add(1);
    building_list_small_add(2);
    building_list_small_add(3);

    assert_eq(3, building_list_small_size());
    const int *items = building_list_small_items();
    assert_eq(1, items[0]);
    assert_eq(2, items[1]);
    assert_eq(3, items[2]);
}

void test_building_list_small_add_too_many()
{
    for (int i = 0; i < 1000; i++)
    {
        building_list_small_add(i);
    }

    assert_eq(499, building_list_small_size());
    const int *items = building_list_small_items();
    assert_eq(999, items[499]);
}

void test_building_list_large_add()
{
    building_list_large_add(1);
    building_list_large_add(2);
    building_list_large_add(3);

    assert_eq(3, building_list_large_size());
    const int *items = building_list_large_items();
    assert_eq(1, items[0]);
    assert_eq(2, items[1]);
    assert_eq(3, items[2]);
}

void test_building_list_large_add_too_many()
{
    for (int i = 0; i < 2010; i++)
    {
        building_list_large_add(i);
    }

    assert_eq(2000, building_list_large_size());
    const int *items = building_list_large_items();
    assert_eq(1999, items[1999]);
}

void test_building_list_large_clear()
{
    building_list_large_add(123);
    building_list_large_clear(0);

    const int *items = building_list_large_items();
    assert_eq(0, building_list_large_size());
    assert_eq(123, items[0]);

    building_list_large_clear(1);
    assert_eq(0, items[0]);
}

void test_building_list_save()
{
    buffer small, large;

    building_list_large_clear(1);
    for (int i = 0; i < 500; i++)
    {
        building_list_small_add(0);
    }
    building_list_small_clear();

    building_list_save_state(&small, &large);

    assert_eq(500, count_buffer_write_i16(&small, 0));
    assert_eq(2000, count_buffer_write_i16(&large, 0));
}

void test_building_list_load()
{
    buffer small, large;

    building_list_load_state(&small, &large);

    assert_eq(500, count_buffer_read_i16(&small));
    assert_eq(2000, count_buffer_read_i16(&large));
}

RUN_TESTS(building.list,
          ADD_TEST(test_building_list_small_add)
          ADD_TEST(test_building_list_small_add_too_many)
          ADD_TEST(test_building_list_large_add)
          ADD_TEST(test_building_list_large_add_too_many)
          ADD_TEST(test_building_list_large_clear)
          ADD_TEST(test_building_list_save)
          ADD_TEST(test_building_list_load)
         )
