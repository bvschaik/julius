#include "core/random.h"

#include <string.h>

#define MAX_RANDOM 100

static struct {
    uint32_t iv1;
    uint32_t iv2;
    int8_t random1_7bit;
    int16_t random1_15bit;
    int8_t random2_7bit;
    int16_t random2_15bit;
    int pool_index;
    int32_t pool[MAX_RANDOM];
} data;

void random_init(void)
{
    memset(&data, 0, sizeof(data));
    data.iv1 = 0x54657687;
    data.iv2 = 0x72641663;
}

void random_generate_next(void)
{
    data.pool[data.pool_index++] = data.random1_7bit;
    if (data.pool_index >= MAX_RANDOM) {
        data.pool_index = 0;
    }
    for (int i = 0; i < 31; i++) {
        unsigned int r1 = (((data.iv1 & 0x10) >> 4) ^ data.iv1) & 1;
        unsigned int r2 = (((data.iv2 & 0x10) >> 4) ^ data.iv2) & 1;
        data.iv1 = data.iv1 >> 1;
        data.iv2 = data.iv2 >> 1;
        if (r1) {
            data.iv1 |= 0x40000000;
        }
        if (r2) {
            data.iv2 |= 0x40000000;
        }
    }
    data.random1_7bit = data.iv1 & 0x7f;
    data.random1_15bit = data.iv1 & 0x7fff;
    data.random2_7bit = data.iv2 & 0x7f;
    data.random2_15bit = data.iv2 & 0x7fff;
}

void random_generate_pool(void)
{
    data.pool_index = 0;
    for (int i = 0; i < MAX_RANDOM; i++) {
        random_generate_next();
    }
}

int8_t random_byte(void)
{
    return data.random1_7bit;
}

int8_t random_byte_alt(void)
{
    return data.random2_7bit;
}

int16_t random_short(void)
{
    return data.random1_15bit;
}

int32_t random_from_pool(int index)
{
    return data.pool[(data.pool_index + index) % MAX_RANDOM];
}

void random_load_state(buffer *buf)
{
    data.iv1 = buffer_read_u32(buf);
    data.iv2 = buffer_read_u32(buf);
}

void random_save_state(buffer *buf)
{
    buffer_write_u32(buf, data.iv1);
    buffer_write_u32(buf, data.iv2);
}
