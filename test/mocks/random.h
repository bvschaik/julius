#include "core/random.hpp"

#define CREATE_RANDOM_MOCKS \
    CREATE_VMOCK0(random_generate_next) \
    CREATE_VMOCK0(random_generate_pool) \
    CREATE_MOCK0(int8_t, random_byte) \
    CREATE_MOCK0(int8_t, random_byte_alt) \
    CREATE_MOCK0(int16_t, random_short) \
    CREATE_MOCK1(int32_t, random_from_pool, int)

#define INIT_RANDOM_MOCKS \
    INIT_MOCK(random_generate_next) \
    INIT_MOCK(random_generate_pool) \
    INIT_MOCK(random_byte) \
    INIT_MOCK(random_byte_alt) \
    INIT_MOCK(random_short) \
    INIT_MOCK(random_from_pool)
