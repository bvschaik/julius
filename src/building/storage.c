#include "storage.h"

#include "building/building.h"
#include "city/resource.h"
#include "core/array.h"
#include "core/calc.h"
#include "core/log.h"

#define STORAGE_ARRAY_SIZE_STEP 200

#define STORAGE_ORIGINAL_BUFFER_SIZE 32
#define STORAGE_CURRENT_BUFFER_SIZE 26

typedef struct {
    int id;
    int in_use;
    int building_id;
    building_storage storage;
} data_storage;

static array(data_storage) storages;

static void storage_create(data_storage *storage, int position)
{
    storage->id = position;
}

static int storage_in_use(const data_storage *storage)
{
    return storage->in_use;
}

void building_storage_clear_all(void)
{
    if (!array_init(storages, STORAGE_ARRAY_SIZE_STEP, storage_create, storage_in_use)) {
        log_error("Unable to create storages. The game will likely crash.", 0, 0);
    }
    array_next(storages); // Ignore first storage
}

void building_storage_reset_building_ids(void)
{
    data_storage *storage;
    array_foreach(storages, storage)
    {
        storage->building_id = 0;
    }

    for (int i = 1; i < building_count(); i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_UNUSED) {
            continue;
        }
        if (b->type == BUILDING_GRANARY || b->type == BUILDING_WAREHOUSE) {
            if (b->storage_id) {
                if (storages.items[b->storage_id].building_id) {
                    // storage is already connected to a building: corrupt, create new
                    b->storage_id = building_storage_create();
                } else {
                    storages.items[b->storage_id].building_id = i;
                }
            }
        }
    }
}

int building_storage_create(void)
{
    data_storage *storage;
    array_new_item(storages, 1, storage);
    if (!storage) {
        return 0;
    }
    storage->in_use = 1;
    return storage->id;
}

int building_storage_restore(int storage_id)
{
    if (storages.items[storage_id].in_use) {
        return 0;
    }
    storages.items[storage_id].in_use = 1;
    if (storage_id >= storages.size) {
        storages.size = storage_id + 1;
    }
    return storage_id;
}

void building_storage_delete(int storage_id)
{
    storages.items[storage_id].in_use = 0;
    array_trim(storages);
}

const building_storage *building_storage_get(int storage_id)
{
    return &storages.items[storage_id].storage;
}

void building_storage_toggle_empty_all(int storage_id)
{
    storages.items[storage_id].storage.empty_all ^= 1;
}

void building_storage_cycle_resource_state(int storage_id, resource_type resource_id)
{
    int state = storages.items[storage_id].storage.resource_state[resource_id];
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
    storages.items[storage_id].storage.resource_state[resource_id] = state;
}

void building_storage_set_permission(building_storage_permission_states p, building *b)
{
    int permission_bit = 1 << p;
    storages.items[b->storage_id].storage.permissions ^= permission_bit;
}

int building_storage_get_permission(building_storage_permission_states p, building *b)
{
    const building_storage *s = building_storage_get(b->storage_id);
    int permission_bit = 1 << p;
    return !(s->permissions & permission_bit);
}

void building_storage_cycle_partial_resource_state(int storage_id, resource_type resource_id)
{
    int state = storages.items[storage_id].storage.resource_state[resource_id];
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
    storages.items[storage_id].storage.resource_state[resource_id] = state;
}
void building_storage_accept_none(int storage_id)
{
    for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        storages.items[storage_id].storage.resource_state[r] = BUILDING_STORAGE_STATE_NOT_ACCEPTING;
    }
}

void building_storage_save_state(buffer *buf)
{
    int buf_size = 4 + storages.size * STORAGE_CURRENT_BUFFER_SIZE;
    uint8_t *buf_data = malloc(buf_size);
    buffer_init(buf, buf_data, buf_size);
    buffer_write_i32(buf, STORAGE_CURRENT_BUFFER_SIZE);

    data_storage *s;
    array_foreach(storages, s)
    {
        buffer_write_i32(buf, s->storage.permissions); // Originally unused
        buffer_write_i32(buf, s->building_id);
        buffer_write_u8(buf, (uint8_t) s->in_use);
        buffer_write_u8(buf, (uint8_t) s->storage.empty_all);
        for (int r = 0; r < RESOURCE_MAX; r++) {
            buffer_write_u8(buf, s->storage.resource_state[r]);
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
    int array_size = calc_value_in_step(storages_to_load, STORAGE_ARRAY_SIZE_STEP);

    if (!array_init(storages, array_size, storage_create, storage_in_use)) {
        log_error("Unable to create storages. The game will likely crash.", 0, 0);
    }

    int highest_id_in_use = 0;

    for (int i = 0; i < storages_to_load; i++) {
        data_storage *s = array_next(storages);
        s->storage.permissions = buffer_read_i32(buf); // Originally unused
        s->building_id = buffer_read_i32(buf);
        s->in_use = buffer_read_u8(buf);
        s->storage.empty_all = buffer_read_u8(buf);
        for (int r = 0; r < RESOURCE_MAX; r++) {
            s->storage.resource_state[r] = buffer_read_u8(buf);
        }
        if (!includes_storage_size) {
            buffer_skip(buf, 6); // unused resource states
        } else if (storage_buf_size > STORAGE_CURRENT_BUFFER_SIZE) {
            buffer_skip(buf, storage_buf_size - STORAGE_CURRENT_BUFFER_SIZE);
        }
        if (s->in_use) {
            highest_id_in_use = i;
        }
    }

    storages.size = highest_id_in_use + 1;
}
