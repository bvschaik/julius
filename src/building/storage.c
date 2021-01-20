#include "storage.h"

#include "city/resource.h"
#include "building/building.h"

#include <stdlib.h>
#include <string.h>

#define STORAGE_ARRAY_SIZE_STEP 200

#define STORAGE_ORIGINAL_BUFFER_SIZE 32
#define STORAGE_CURRENT_BUFFER_SIZE 26

typedef struct {
    int in_use;
    int building_id;
    building_storage storage;
} data_storage;

static struct {
    data_storage *storages;
    int array_size;
} data;

static void create_storage_array(int size)
{
    free(data.storages);
    data.array_size = size;
    data.storages = malloc(size * sizeof(data_storage));
    memset(data.storages, 0, size * sizeof(data_storage));
}

static int expand_storage_array(void)
{
    data_storage *storage = realloc(data.storages, (data.array_size + STORAGE_ARRAY_SIZE_STEP) * sizeof(data_storage));
    if (!storage) {
        return 0;
    }
    data.storages = storage;
    memset(&data.storages[data.array_size], 0, sizeof(data_storage) * STORAGE_ARRAY_SIZE_STEP);
    data.array_size += STORAGE_ARRAY_SIZE_STEP;
    return data.array_size - STORAGE_ARRAY_SIZE_STEP;
}

void building_storage_clear_all(void)
{
    create_storage_array(STORAGE_ARRAY_SIZE_STEP);
}

void building_storage_reset_building_ids(void)
{
    for (int i = 1; i < data.array_size; i++) {
        data.storages[i].building_id = 0;
    }

    for (int i = 1; i < building_count(); i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_UNUSED) {
            continue;
        }
        if (b->type == BUILDING_GRANARY || b->type == BUILDING_WAREHOUSE) {
            if (b->storage_id) {
                if (data.storages[b->storage_id].building_id) {
                    // storage is already connected to a building: corrupt, create new
                    b->storage_id = building_storage_create();
                } else {
                    data.storages[b->storage_id].building_id = i;
                }
            }
        }
    }
}

int building_storage_create(void)
{
    for (int i = 1; i < data.array_size; i++) {
        if (!data.storages[i].in_use) {
            memset(&data.storages[i], 0, sizeof(data_storage));
            data.storages[i].in_use = 1;
            return i;
        }
    }
    int id = expand_storage_array();
    if (id) {
        data.storages[id].in_use = 1;
    }
    return id;
}

int building_storage_restore(int storage_id) 
{
    if (data.storages[storage_id].in_use) {
        return 0;
    }
    data.storages[storage_id].in_use = 1;
    return storage_id;
}

void building_storage_delete(int storage_id)
{
    data.storages[storage_id].in_use = 0;
}

const building_storage *building_storage_get(int storage_id)
{
    return &data.storages[storage_id].storage;
}

void building_storage_toggle_empty_all(int storage_id)
{
    data.storages[storage_id].storage.empty_all = 1 - data.storages[storage_id].storage.empty_all;
}

void building_storage_cycle_resource_state(int storage_id, resource_type resource_id)
{
    int state = data.storages[storage_id].storage.resource_state[resource_id];
    if (state == BUILDING_STORAGE_STATE_ACCEPTING) {
        state = BUILDING_STORAGE_STATE_NOT_ACCEPTING;
    } else if (state == BUILDING_STORAGE_STATE_NOT_ACCEPTING) {
        state = BUILDING_STORAGE_STATE_GETTING;
    } else if (state == BUILDING_STORAGE_STATE_GETTING) {
        state = BUILDING_STORAGE_STATE_ACCEPTING;
    } else if (state == BUILDING_STORAGE_STATE_ACCEPTING_HALF) {
        state = BUILDING_STORAGE_STATE_NOT_ACCEPTING_HALF;
    } else if (state == BUILDING_STORAGE_STATE_ACCEPTING_3QUARTERS) {
        state = BUILDING_STORAGE_STATE_NOT_ACCEPTING_3QUARTERS;
    } else if (state == BUILDING_STORAGE_STATE_ACCEPTING_QUARTER) {
        state = BUILDING_STORAGE_STATE_NOT_ACCEPTING_QUARTER;
    } else if (state == BUILDING_STORAGE_STATE_GETTING_3QUARTERS) {
        state = BUILDING_STORAGE_STATE_ACCEPTING_3QUARTERS;
    } else if (state == BUILDING_STORAGE_STATE_GETTING_HALF) {
        state = BUILDING_STORAGE_STATE_ACCEPTING_HALF;
    } else if (state == BUILDING_STORAGE_STATE_GETTING_QUARTER) {
        state = BUILDING_STORAGE_STATE_ACCEPTING_QUARTER;
    } else if (state == BUILDING_STORAGE_STATE_NOT_ACCEPTING_HALF) {
        state = BUILDING_STORAGE_STATE_GETTING_HALF;
    } else if (state == BUILDING_STORAGE_STATE_NOT_ACCEPTING_QUARTER) {
        state = BUILDING_STORAGE_STATE_GETTING_QUARTER;
    } else if (state == BUILDING_STORAGE_STATE_NOT_ACCEPTING_3QUARTERS) {
        state = BUILDING_STORAGE_STATE_GETTING_3QUARTERS;
    }
    data.storages[storage_id].storage.resource_state[resource_id] = state;
}

void building_storage_set_permission(building_storage_permission_states p, building* b)
{
    int permission_bit = 1 << p;
    data.storages[b->storage_id].storage.permissions ^= permission_bit;
}

int building_storage_get_permission(building_storage_permission_states p, building* b)
{
    const building_storage* s = building_storage_get(b->storage_id);
    int permission_bit = 1 << p;
    return !(s->permissions & permission_bit);
}

void building_storage_cycle_partial_resource_state(int storage_id, resource_type resource_id)
{
    int state = data.storages[storage_id].storage.resource_state[resource_id];
    if (state == BUILDING_STORAGE_STATE_ACCEPTING) {
        state = BUILDING_STORAGE_STATE_ACCEPTING_3QUARTERS;
    } else if (state == BUILDING_STORAGE_STATE_ACCEPTING_3QUARTERS) {
        state = BUILDING_STORAGE_STATE_ACCEPTING_HALF;
    } else if (state == BUILDING_STORAGE_STATE_ACCEPTING_HALF) {
        state = BUILDING_STORAGE_STATE_ACCEPTING_QUARTER;
    } else if (state == BUILDING_STORAGE_STATE_ACCEPTING_QUARTER) {
        state = BUILDING_STORAGE_STATE_ACCEPTING;
    }
    if (state == BUILDING_STORAGE_STATE_GETTING) {
        state = BUILDING_STORAGE_STATE_GETTING_3QUARTERS;
    } else if (state == BUILDING_STORAGE_STATE_GETTING_3QUARTERS) {
        state = BUILDING_STORAGE_STATE_GETTING_HALF;
    } else if (state == BUILDING_STORAGE_STATE_GETTING_HALF) {
        state = BUILDING_STORAGE_STATE_GETTING_QUARTER;
    } else if (state == BUILDING_STORAGE_STATE_GETTING_QUARTER) {
        state = BUILDING_STORAGE_STATE_GETTING;
    }
    data.storages[storage_id].storage.resource_state[resource_id] = state;
}
void building_storage_accept_none(int storage_id)
{
    for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        data.storages[storage_id].storage.resource_state[r] = BUILDING_STORAGE_STATE_NOT_ACCEPTING;
    }
}

void building_storage_save_state(buffer *buf)
{
    int buf_size = 4 + data.array_size * STORAGE_CURRENT_BUFFER_SIZE;
    uint8_t *buf_data = malloc(buf_size);
    buffer_init(buf, buf_data, buf_size);
    buffer_write_i32(buf, STORAGE_CURRENT_BUFFER_SIZE);

    for (int i = 0; i < data.array_size; i++) {
        buffer_write_i32(buf, data.storages[i].storage.permissions); // Originally unused
        buffer_write_i32(buf, data.storages[i].building_id);
        buffer_write_u8(buf, (uint8_t) data.storages[i].in_use);
        buffer_write_u8(buf, (uint8_t) data.storages[i].storage.empty_all);
        for (int r = 0; r < RESOURCE_MAX; r++) {
            buffer_write_u8(buf, data.storages[i].storage.resource_state[r]);
        }
    }
}

void building_storage_load_state(buffer *buf, int includes_storage_size)
{
    int storage_buf_size = STORAGE_ORIGINAL_BUFFER_SIZE;
    int buf_size = buf->size;

    if (includes_storage_size) {
        storage_buf_size = buffer_read_i32(buf);
        buf_size -= 4;
    }

    int storages_to_load = buf_size / storage_buf_size;

    create_storage_array(storages_to_load);

    // Reduce number of used storages on old Augustus savefiles that were hardcoded to load 1000. Improves performance
    int highest_id_in_use = 0;
    int reduce_storage_array_size = !includes_storage_size && storages_to_load == 1000;

    for (int i = 0; i < data.array_size; i++) {
        data.storages[i].storage.permissions = buffer_read_i32(buf); // Originally unused
        data.storages[i].building_id = buffer_read_i32(buf);
        data.storages[i].in_use = buffer_read_u8(buf);
        data.storages[i].storage.empty_all = buffer_read_u8(buf);
        for (int r = 0; r < RESOURCE_MAX; r++) {
            data.storages[i].storage.resource_state[r] = buffer_read_u8(buf);
        }
        if (!includes_storage_size) {
            buffer_skip(buf, 6); // unused resource states
        } else if (storage_buf_size > STORAGE_CURRENT_BUFFER_SIZE) {
            buffer_skip(buf, storage_buf_size - STORAGE_CURRENT_BUFFER_SIZE);
        }
        if (reduce_storage_array_size && data.storages[i].in_use) {
            highest_id_in_use = i;
        }
    }

    if (reduce_storage_array_size) {
        data.array_size = STORAGE_ARRAY_SIZE_STEP;
        while (highest_id_in_use > data.array_size) {
            data.array_size += STORAGE_ARRAY_SIZE_STEP;
        }
    }
}
