#include "list.h"

#include <stdlib.h>
#include <string.h>

#define SMALL_SIZE_STEP 500
#define LARGE_SIZE_STEP 2000
#define BURNING_SIZE_STEP 500

static struct {
    struct {
        int size;
        int *items;
        int array_size;
    } small;
    struct {
        int size;
        int *items;
        int array_size;
    } large;
    struct {
        int size;
        int *items;
        int array_size;
    } burning;
} data;

void building_list_small_clear(void)
{
    data.small.size = 0;
}

void building_list_small_add(int building_id)
{
    if (data.small.size == data.small.array_size) {
        data.small.array_size += SMALL_SIZE_STEP;
        int *items = realloc(data.small.items, data.small.array_size * sizeof(int));
        if (!items) {
            data.small.size--;
        } else {
            data.small.items = items;
        }
    }
    data.small.items[data.small.size++] = building_id;
}

int building_list_small_size(void)
{
    return data.small.size;
}

const int *building_list_small_items(void)
{
    return data.small.items;
}

void building_list_large_clear(int clear_entries)
{
    data.large.size = 0;
    if (clear_entries) {
        memset(data.large.items, 0, data.large.array_size * sizeof(int));
    }
}

void building_list_large_add(int building_id)
{
    if (data.large.size == data.large.array_size) {
        data.large.array_size += LARGE_SIZE_STEP;
        int *items = realloc(data.large.items, data.large.array_size * sizeof(int));
        if (!items) {
            return;
        }
        data.large.items = items;
    }
    data.large.items[data.large.size++] = building_id;
}

int building_list_large_size(void)
{
    return data.large.size;
}

const int *building_list_large_items(void)
{
    return data.large.items;
}

void building_list_burning_clear(void)
{
    data.burning.size = 0;
}

void building_list_burning_add(int building_id)
{
    if (data.burning.size == data.burning.array_size) {
        data.burning.array_size += BURNING_SIZE_STEP;
        int *items = realloc(data.burning.items, data.burning.array_size * sizeof(int));
        if (!items) {
            data.burning.size--;
        } else {
            data.burning.items = items;
        }
    }
    data.burning.items[data.burning.size++] = building_id;
}

int building_list_burning_size(void)
{
    return data.burning.size;
}

const int *building_list_burning_items(void)
{
    return data.burning.items;
}

void building_list_save_state(buffer *small, buffer *large, buffer *burning, buffer *burning_totals)
{
    int buf_size = data.small.size * sizeof(int32_t);
    if (buf_size) {
        uint8_t *buf_data = malloc(buf_size);
        buffer_init(small, buf_data, buf_size);
        for (int i = 0; i < data.small.size; i++) {
            buffer_write_i32(small, data.small.items[i]);
        }
    }

    buf_size = data.large.size * sizeof(int32_t);
    if (buf_size) {
        uint8_t *buf_data = malloc(buf_size);
        buffer_init(large, buf_data, buf_size);
        for (int i = 0; i < data.large.size; i++) {
            buffer_write_i32(large, data.large.items[i]);
        }
    }

    buf_size = data.burning.size * sizeof(int32_t);
    if (buf_size) {
        uint8_t *buf_data = malloc(buf_size);
        buffer_init(burning, buf_data, buf_size);
        for (int i = 0; i < data.burning.size; i++) {
            buffer_write_i32(burning, data.burning.items[i]);
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
        int size = small->size / sizeof(int16_t);
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
        int size = small->size / sizeof(int32_t);
        for (int i = 0; i < size; i++) {
            building_list_large_add(buffer_read_i32(small));
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
