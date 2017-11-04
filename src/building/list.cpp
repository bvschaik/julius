#include "list.hpp"

#include <string.h>

#define MAX_SMALL 500
#define MAX_LARGE 2000

static struct
{
    struct
    {
        int size;
        int items[MAX_SMALL];
    } small;
    struct
    {
        int size;
        int items[MAX_LARGE];
    } large;
} data;

void building_list_small_clear()
{
    data.small.size = 0;
}

void building_list_small_add(int building_id)
{
    data.small.items[data.small.size++] = building_id;
    if (data.small.size >= MAX_SMALL)
    {
        data.small.size = MAX_SMALL - 1;
    }
}

int building_list_small_size()
{
    return data.small.size;
}

const int *building_list_small_items()
{
    return data.small.items;
}

void building_list_large_clear(int clear_entries)
{
    data.large.size = 0;
    if (clear_entries)
    {
        memset(data.large.items, 0, MAX_LARGE * sizeof(int));
    }
}

void building_list_large_add(int building_id)
{
    if (data.large.size < MAX_LARGE)
    {
        data.large.items[data.large.size++] = building_id;
    }
}

int building_list_large_size()
{
    return data.large.size;
}

const int *building_list_large_items()
{
    return data.large.items;
}

void building_list_save_state(buffer *small, buffer *large)
{
    for (int i = 0; i < MAX_SMALL; i++)
    {
        buffer_write_i16(small, data.small.items[i]);
    }
    for (int i = 0; i < MAX_LARGE; i++)
    {
        buffer_write_i16(large, data.large.items[i]);
    }
}

void building_list_load_state(buffer *small, buffer *large)
{
    for (int i = 0; i < MAX_SMALL; i++)
    {
        data.small.items[i] = buffer_read_i16(small);
    }
    for (int i = 0; i < MAX_LARGE; i++)
    {
        data.large.items[i] = buffer_read_i16(large);
    }
}
