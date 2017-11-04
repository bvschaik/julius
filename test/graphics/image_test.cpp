#include "loki/loki.h"

#include "mocks/buffer.h"
#include "graphics/image.hpp"
#include "core/buffer.hpp"

CREATE_BUFFER_MOCKS

CREATE_VMOCK2(file_change_extension, char*, const char*)

CREATE_MOCK3(int, io_read_file_into_buffer, const char*, void*, unsigned int)
CREATE_MOCK4(int, io_read_file_part_into_buffer, const char*, void*, unsigned int, unsigned int)
CREATE_VMOCK3(debug_log, const char*, const char*, int)

INIT_MOCKS(
    INIT_BUFFER_MOCKS
    INIT_MOCK(file_change_extension)
    INIT_MOCK(io_read_file_into_buffer)
    INIT_MOCK(io_read_file_part_into_buffer)
    INIT_MOCK(debug_log)
)

int any_read_file(const char *file, void *buf, unsigned int size)
{
    return 1;
}

int any_read_part(const char *file, void *buf, unsigned int size, unsigned int skip)
{
    return 1;
}


void test_image_load_climate_fail()
{
    when_io_read_file_into_buffer_dynamic(any_read_file)->then_return = 0;

    image_init();
    int result = image_load_climate(1);

    assert_false(result);
}

void test_image_load_climate_ok()
{
    when_io_read_file_into_buffer_dynamic(any_read_file)->then_return = 660680;

    image_init();
    int result = image_load_climate(1);

    assert_true(result);
    verify_io_read_file_into_buffer_times(2);
    verify_buffer_read_i32_times(10000 * 3);
}

void test_image_load_climate_request_twice_loaded_once()
{
    when_io_read_file_into_buffer_dynamic(any_read_file)->then_return = 660680;

    image_init();
    image_load_climate(2);
    int result = image_load_climate(2);

    assert_true(result);
    verify_io_read_file_into_buffer_times(2);
    verify_buffer_read_i32_times(10000 * 3);
}

void test_image_load_enemy_fail()
{
    when_io_read_file_part_into_buffer_dynamic(any_read_part)->then_return = 0;

    image_init();
    int result = image_load_enemy(1);

    assert_false(result);
}

void test_image_load_enemy_fail_data()
{
    when_io_read_file_part_into_buffer_dynamic(any_read_part)->then_return = 51264;

    image_init();
    int result = image_load_enemy(1);

    assert_false(result);
}

void test_image_load_enemy_ok()
{
    when_io_read_file_part_into_buffer_dynamic(any_read_part)->then_return = 51264;
    when_io_read_file_into_buffer_dynamic(any_read_file)->then_return = 1;

    image_init();
    int result = image_load_enemy(1);

    assert_true(result);
    verify_buffer_init_times(2);
    verify_buffer_read_i32_times(801 * 3);
}

RUN_TESTS(graphics.image,
          ADD_TEST(test_image_load_climate_fail)
          ADD_TEST(test_image_load_climate_ok)
          ADD_TEST(test_image_load_climate_request_twice_loaded_once)
          ADD_TEST(test_image_load_enemy_fail)
          ADD_TEST(test_image_load_enemy_fail_data)
          ADD_TEST(test_image_load_enemy_ok)
         )
