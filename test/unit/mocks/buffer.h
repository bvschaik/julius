#include "core/buffer.h"

#define CREATE_BUFFER_MOCKS \
    CREATE_VMOCK3(buffer_init, buffer*, void*, int) \
    CREATE_VMOCK2(buffer_write_u8, buffer*, uint8_t) \
    CREATE_VMOCK2(buffer_write_u16, buffer*, uint16_t) \
    CREATE_VMOCK2(buffer_write_u32, buffer*, uint32_t) \
    CREATE_VMOCK2(buffer_write_i8, buffer*, int8_t) \
    CREATE_VMOCK2(buffer_write_i16, buffer*, int16_t) \
    CREATE_VMOCK2(buffer_write_i32, buffer*, int32_t) \
    CREATE_VMOCK3(buffer_write_raw, buffer*, const void *, int) \
    CREATE_MOCK1(uint8_t, buffer_read_u8, buffer*) \
    CREATE_MOCK1(uint16_t, buffer_read_u16, buffer*) \
    CREATE_MOCK1(uint32_t, buffer_read_u32, buffer*) \
    CREATE_MOCK1(int8_t, buffer_read_i8, buffer*) \
    CREATE_MOCK1(int16_t, buffer_read_i16, buffer*) \
    CREATE_MOCK1(int32_t, buffer_read_i32, buffer*) \
    CREATE_MOCK3(int, buffer_read_raw, buffer*, void*, int) \
    CREATE_VMOCK2(buffer_skip, buffer*, int) \
    CREATE_VMOCK2(buffer_set, buffer*, int)

#define INIT_BUFFER_MOCKS \
    INIT_MOCK(buffer_init) \
    INIT_MOCK(buffer_write_u8) \
    INIT_MOCK(buffer_write_u16) \
    INIT_MOCK(buffer_write_u32) \
    INIT_MOCK(buffer_write_i8) \
    INIT_MOCK(buffer_write_i16) \
    INIT_MOCK(buffer_write_i32) \
    INIT_MOCK(buffer_write_raw) \
    INIT_MOCK(buffer_read_u8) \
    INIT_MOCK(buffer_read_u16) \
    INIT_MOCK(buffer_read_u32) \
    INIT_MOCK(buffer_read_i8) \
    INIT_MOCK(buffer_read_i16) \
    INIT_MOCK(buffer_read_i32) \
    INIT_MOCK(buffer_read_raw) \
    INIT_MOCK(buffer_skip) \
    INIT_MOCK(buffer_set)

