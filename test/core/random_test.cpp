#include "loki/loki.h"
#include "core/random.hpp"

CREATE_VMOCK2(buffer_write_u32, buffer*, uint32_t)
CREATE_MOCK1(uint32_t, buffer_read_u32, buffer*)

INIT_MOCKS(
    INIT_MOCK(buffer_read_u32)
    INIT_MOCK(buffer_write_u32)
)

void test_random_init_ivs_are_set()
{
    buffer buf;
    random_init();

    random_save_state(&buf);
    random_save_state(&buf);

    verify_buffer_write_u32(&buf, 0x54657687);
    verify_buffer_write_u32(&buf, 0x72641663);
}

void test_random_can_load_and_save_ivs()
{
    buffer buf;
    when_buffer_read_u32(&buf)->then_return = 21;

    random_load_state(&buf);
    random_save_state(&buf);

    verify_buffer_read_u32_times(2);
    verify_buffer_write_u32(&buf, 21);
}

void test_random_generate_next()
{
    buffer buf;

    random_init();
    random_generate_next();

    assert_eq(0x6F, random_byte());
    assert_eq(0x21EF, random_short());
    assert_eq(0x05, random_byte_alt());

    random_save_state(&buf);
    verify_buffer_write_u32(&buf, 0x292321EF);
    verify_buffer_write_u32(&buf, 0x5D425705);
}

void test_random_generate_pool()
{
    random_init();
    random_generate_pool();

    uint8_t expected[100] =
    {
        0x00, 0x6F, 0x71, 0x4E, 0x62, 0x4C, 0x40, 0x44, 0x70, 0x17,
        0x16, 0x3F, 0x04, 0x6C, 0x62, 0x7C, 0x7B, 0x5C, 0x49, 0x7D,
        0x72, 0x1D, 0x5C, 0x41, 0x55, 0x78, 0x37, 0x04, 0x7C, 0x73,
        0x7C, 0x2B, 0x29, 0x63, 0x75, 0x7A, 0x05, 0x4D, 0x69, 0x7F,
        0x18, 0x19, 0x08, 0x38, 0x23, 0x79, 0x56, 0x73, 0x14, 0x1D,
        0x54, 0x69, 0x5F, 0x52, 0x6F, 0x39, 0x5A, 0x77, 0x60, 0x06,
        0x36, 0x65, 0x73, 0x54, 0x61, 0x47, 0x63, 0x1D, 0x04, 0x5C,
        0x59, 0x64, 0x42, 0x36, 0x2D, 0x5F, 0x2A, 0x30, 0x6B, 0x45,
        0x19, 0x20, 0x02, 0x0A, 0x72, 0x45, 0x09, 0x09, 0x09, 0x61,
        0x47, 0x33, 0x10, 0x41, 0x0D, 0x4D, 0x39, 0x42, 0x5E, 0x3B
    };
    for (int i = 0; i < 100; i++)
    {
        assert_eq(expected[i], random_from_pool(i));
    }
}

RUN_TESTS(random,
          ADD_TEST(test_random_can_load_and_save_ivs)
          ADD_TEST(test_random_init_ivs_are_set)
          ADD_TEST(test_random_generate_next)
          ADD_TEST(test_random_generate_pool)
         )
