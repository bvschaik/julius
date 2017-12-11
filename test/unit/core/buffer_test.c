#include "loki/loki.h"

#include "core/buffer.h"

uint8_t bytes[100];
static buffer buf;

void setup()
{
    buffer_init(&buf, bytes, 100);
}

INIT_MOCKS(
    SETUP(setup)
)

void set_bytes(uint8_t *input, int size)
{
    for (int i = 0; i < size; i++) {
        bytes[i] = input[i];
    }
}

void assert_bytes(uint8_t *expected, int size)
{
    assert_eq(size, buf.index);
    
    for (int i = 0; i < size; i++) {
        assert_eq(expected[i], bytes[i]);
    }
}

void test_buffer_reset()
{
    buffer_write_i32(&buf, 1);
    assert_eq(4, buf.index);
    
    buffer_reset(&buf);
    assert_eq(0, buf.index);
}

void test_buffer_write_u8()
{
    uint8_t v1 = 0;
    uint8_t v2 = 255;
    uint8_t v3 = 123;
    
    buffer_write_u8(&buf, v1);
    buffer_write_u8(&buf, v2);
    buffer_write_u8(&buf, v3);
    
    uint8_t expected[] = {0, 255, 123};
    assert_bytes(expected, 3);
}

void test_buffer_write_u16()
{
    uint16_t v1 = 0xffff;
    uint16_t v2 = 0x1234;
    
    buffer_write_u16(&buf, v1);
    buffer_write_u16(&buf, v2);
    
    uint8_t expected[] = {255, 255, 0x34, 0x12};
    assert_bytes(expected, 4);
}

void test_buffer_write_u32()
{
    uint32_t v1 = 0x12345678;
    uint32_t v2 = 0xfedcba98;
    
    buffer_write_u32(&buf, v1);
    buffer_write_u32(&buf, v2);
    
    uint8_t expected[] = {0x78, 0x56, 0x34, 0x12, 0x98, 0xba, 0xdc, 0xfe};
    assert_bytes(expected, 8);
}

void test_buffer_write_i8()
{
    int8_t v1 = 0;
    int8_t v2 = 127;
    int8_t v3 = -128;
    
    buffer_write_i8(&buf, v1);
    buffer_write_i8(&buf, v2);
    buffer_write_i8(&buf, v3);
    
    uint8_t expected[] = {0, 0x7f, 0x80};
    assert_bytes(expected, 3);
}

void test_buffer_write_i16()
{
    int16_t v1 = 32767;
    int16_t v2 = -32768;
    int16_t v3 = -999;
    
    buffer_write_i16(&buf, v1);
    buffer_write_i16(&buf, v2);
    buffer_write_i16(&buf, v3);
    
    uint8_t expected[] = {0xff, 0x7f, 0x00, 0x80, 0x19, 0xfc};
    assert_bytes(expected, 6);
}

void test_buffer_write_i32()
{
    int32_t v1 = 0x12345678;
    int32_t v2 = -999;
    int32_t v3 = 2147483648;
    
    buffer_write_i32(&buf, v1);
    buffer_write_i32(&buf, v2);
    buffer_write_i32(&buf, v3);
    
    uint8_t expected[] = {0x78, 0x56, 0x34, 0x12, 0x19, 0xfc, 0xff, 0xff, 0, 0, 0, 0x80};
    assert_bytes(expected, 12);
}

void test_buffer_write_raw()
{
    uint8_t input[] = {0x78, 0x56, 0x34, 0x12, 0x19, 0xfc, 0xff, 0xff, 0, 0, 0, 0x80};
    
    buffer_write_raw(&buf, input, 4);
    buffer_write_raw(&buf, &input[4], 8);
    
    assert_bytes(input, 12);
}

void test_buffer_write_overflow()
{
    buf.size = 0;
    
    buffer_write_i8(&buf, 1);
    buffer_write_i16(&buf, 1);
    buffer_write_i32(&buf, 1);
    buffer_write_u8(&buf, 1);
    buffer_write_u16(&buf, 1);
    buffer_write_u32(&buf, 1);
    uint8_t val = 1;
    buffer_write_raw(&buf, &val, 1);
    
    assert_eq(0, buf.index);
}

void test_buffer_read_u8()
{
    uint8_t input[] = {0, 255, 123};
    set_bytes(input, 3);

    uint8_t expected1 = 0;
    uint8_t expected2 = 255;
    uint8_t expected3 = 123;
    
    uint8_t v1 = buffer_read_u8(&buf);
    uint8_t v2 = buffer_read_u8(&buf);
    uint8_t v3 = buffer_read_u8(&buf);

    assert_eq(expected1, v1);
    assert_eq(expected2, v2);
    assert_eq(expected3, v3);
}

void test_buffer_read_u16()
{
    uint8_t input[] = {255, 255, 0x34, 0x12};
    set_bytes(input, 4);

    uint16_t expected1 = 0xffff;
    uint16_t expected2 = 0x1234;
    
    uint16_t v1 = buffer_read_u16(&buf);
    uint16_t v2 = buffer_read_u16(&buf);

    assert_eq(expected1, v1);
    assert_eq(expected2, v2);
}

void test_buffer_read_u32()
{
    uint8_t input[] = {0x78, 0x56, 0x34, 0x12, 0x98, 0xba, 0xdc, 0xfe};
    set_bytes(input, 8);

    uint32_t expected1 = 0x12345678;
    uint32_t expected2 = 0xfedcba98;
    
    uint32_t v1 = buffer_read_u32(&buf);
    uint32_t v2 = buffer_read_u32(&buf);

    assert_eq(expected1, v1);
    assert_eq(expected2, v2);
}

void test_buffer_read_i8()
{
    uint8_t input[] = {0, 0x7f, 0x80};
    set_bytes(input, 3);
    
    int16_t expected1 = 0;
    int16_t expected2 = 127;
    int16_t expected3 = -128;

    int16_t v1 = buffer_read_i8(&buf);
    int16_t v2 = buffer_read_i8(&buf);
    int16_t v3 = buffer_read_i8(&buf);
    
    assert_eq(expected1, v1);
    assert_eq(expected2, v2);
    assert_eq(expected3, v3);
}

void test_buffer_read_i16()
{
    uint8_t input[] = {0xff, 0x7f, 0x00, 0x80, 0x19, 0xfc};
    set_bytes(input, 6);

    int16_t expected1 = 32767;
    int16_t expected2 = -32768;
    int16_t expected3 = -999;

    int16_t v1 = buffer_read_i16(&buf);
    int16_t v2 = buffer_read_i16(&buf);
    int16_t v3 = buffer_read_i16(&buf);
    
    assert_eq(expected1, v1);
    assert_eq(expected2, v2);
    assert_eq(expected3, v3);
}

void test_buffer_read_i32()
{
    uint8_t input[] = {0x78, 0x56, 0x34, 0x12, 0x19, 0xfc, 0xff, 0xff, 0, 0, 0, 0x80};
    set_bytes(input, 12);

    int32_t expected1 = 0x12345678;
    int32_t expected2 = -999;
    int32_t expected3 = 2147483648;
    
    int32_t v1 = buffer_read_i32(&buf);
    int32_t v2 = buffer_read_i32(&buf);
    int32_t v3 = buffer_read_i32(&buf);

    assert_eq(expected1, v1);
    assert_eq(expected2, v2);
    assert_eq(expected3, v3);
}

void test_buffer_read_raw()
{
    uint8_t input[] = {0x78, 0x56, 0x34, 0x12, 0x19, 0xfc, 0xff, 0xff, 0, 0, 0, 0x80};
    set_bytes(input, 12);
    
    uint8_t result[12];
    
    buffer_read_raw(&buf, result, 4);
    buffer_read_raw(&buf, &result[4], 8);
    
    for (int i = 0; i < 12; i++) {
        assert_eq(input[i], result[i]);
    }
}

void test_buffer_read_overflow()
{
    buf.size = 0;
    
    buffer_read_i8(&buf);
    buffer_read_i16(&buf);
    buffer_read_i32(&buf);
    buffer_read_u8(&buf);
    buffer_read_u16(&buf);
    buffer_read_u32(&buf);

    assert_eq(0, buf.index);
}

void test_buffer_read_raw_overflow()
{
    uint8_t output[50];
    buf.size = 10;

    int result = buffer_read_raw(&buf, &output, 50);
    
    assert_eq(10, result);
    assert_eq(10, buf.index);
}

void test_buffer_skip()
{
    uint8_t input[] = {1, 2, 3, 4, 5, 6, 7, 8};
    set_bytes(input, 8);
    
    buffer_skip(&buf, 2);
    uint8_t v1 = buffer_read_u8(&buf);
    buffer_skip(&buf, 3);
    uint8_t v2 = buffer_read_u8(&buf);
    
    assert_eq(7, buf.index);
    assert_eq(3, v1);
    assert_eq(7, v2);
}

RUN_TESTS(core/buffer,
    ADD_TEST(test_buffer_reset)

    ADD_TEST(test_buffer_write_u8)
    ADD_TEST(test_buffer_write_u16)
    ADD_TEST(test_buffer_write_u32)
    ADD_TEST(test_buffer_write_i8)
    ADD_TEST(test_buffer_write_i16)
    ADD_TEST(test_buffer_write_i32)
    ADD_TEST(test_buffer_write_raw)
    ADD_TEST(test_buffer_write_overflow);
    
    ADD_TEST(test_buffer_read_u8)
    ADD_TEST(test_buffer_read_u16)
    ADD_TEST(test_buffer_read_u32)
    ADD_TEST(test_buffer_read_i8)
    ADD_TEST(test_buffer_read_i16)
    ADD_TEST(test_buffer_read_i32)
    ADD_TEST(test_buffer_read_raw)
    ADD_TEST(test_buffer_read_overflow)
    ADD_TEST(test_buffer_read_raw_overflow)
    ADD_TEST(test_buffer_skip)
)
