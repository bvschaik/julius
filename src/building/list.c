#include "list.h"

#include "core/array.h"

#define SMALL_SIZE_STEP 500
#define LARGE_SIZE_STEP 2000
#define BURNING_SIZE_STEP 500

static struct {
    array(int) small;
    array(int) large;
    array(int) burning;
} data;

void building_list_small_clear(void)
{
    data.small.size = 0;
}

void building_list_small_add(int building_id)
{
    if (!data.small.blocks && !array_init(data.small, SMALL_SIZE_STEP, 0, 0)) {
        return;
    }
    int *element = array_advance(data.small);
    if (!element) {
        element = array_last(data.small);
    }
    *element = building_id;
}

int building_list_small_size(void)
{
    return data.small.size;
}

int building_list_small_item(int index)
{
    return *array_item(data.small, index);
}

void building_list_large_clear(void)
{
    data.large.size = 0;
}

void building_list_large_add(int building_id)
{
    if (!data.large.blocks && !array_init(data.large, LARGE_SIZE_STEP, 0, 0)) {
        return;
    }
    int *element = array_advance(data.large);
    if (!element) {
        element = array_last(data.large);
    }
    *element = building_id;
}

int building_list_large_size(void)
{
    return data.large.size;
}

int building_list_large_item(int index)
{
    return *array_item(data.large, index);
}

void building_list_burning_clear(void)
{
    data.burning.size = 0;
}

void building_list_burning_add(int building_id)
{
    if (!data.burning.blocks && !array_init(data.burning, BURNING_SIZE_STEP, 0, 0)) {
        return;
    }
    int *element = array_advance(data.burning);
    if (!element) {
        element = array_last(data.burning);
    }
    *element = building_id;
}

int building_list_burning_size(void)
{
    return data.burning.size;
}

int building_list_burning_item(int index)
{
    return *array_item(data.burning, index);
}

void building_list_save_state(buffer *small, buffer *large, buffer *burning, buffer *burning_totals)
{
    int buf_size = data.small.size * sizeof(int32_t);
    int *value;
    if (buf_size) {
        uint8_t *buf_data = malloc(buf_size);
        buffer_init(small, buf_data, buf_size);
        array_foreach(data.small, value)
        {
            buffer_write_i32(small, *value);
        }
    }

    buf_size = data.large.size * sizeof(int32_t);
    if (buf_size) {
        uint8_t *buf_data = malloc(buf_size);
        buffer_init(large, buf_data, buf_size);
        array_foreach(data.large, value)
        {
            buffer_write_i32(large, *value);
        }
    }

    buf_size = data.burning.size * sizeof(int32_t);
    if (buf_size) {
        uint8_t *buf_data = malloc(buf_size);
        buffer_init(burning, buf_data, buf_size);
        array_foreach(data.burning, value)
        {
            buffer_write_i32(burning, *value);
        }
    }

    buffer_write_i32(burning_totals, data.burning.size);
}

void building_list_load_state(buffer *small, buffer *large, buffer *burning, buffer *burning_totals, int is_new_version)
{
    data.small.size = 0;
    data.large.size = 0;
    data.burning.size = 0;

    if (!is_new_version) {
        size_t size = small->size / sizeof(int16_t);
        for (int i = 0; i < size; i++) {
            building_list_small_add(buffer_read_i16(small));
        }
        size = large->size / sizeof(int16_t);
        for (int i = 0; i < size; i++) {
            building_list_large_add(buffer_read_i16(large));
        }
        size = burning->size / sizeof(int16_t);
        for (int i = 0; i < size; i++) {
            building_list_burning_add(buffer_read_i16(burning));
        }

        buffer_skip(burning_totals, 4);
    } else {
        size_t size = small->size / sizeof(int32_t);
        for (int i = 0; i < size; i++) {
            building_list_small_add(buffer_read_i32(small));
        }
        size = large->size / sizeof(int32_t);
        for (int i = 0; i < size; i++) {
            building_list_large_add(buffer_read_i32(large));
        }
        size = burning->size / sizeof(int32_t);
        for (int i = 0; i < size; i++) {
            building_list_burning_add(buffer_read_i32(burning));
        }
    }
    data.small.size = 0;
    data.large.size = 0;
    data.burning.size = buffer_read_i32(burning_totals);
}
