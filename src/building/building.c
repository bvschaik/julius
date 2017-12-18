#include "building.h"

#include <string.h>

static building Data_Buildings[MAX_BUILDINGS];

building *building_get(int id)
{
    return &Data_Buildings[id];
}

void building_delete(building *b)
{
    memset(b, 0, sizeof(building));
}

void building_clear_all()
{
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        memset(&Data_Buildings[i], 0, sizeof(building));
        Data_Buildings[i].id = i;
    }
}

static void building_save(building *b, buffer *buf)
{
    buffer_write_raw(buf, b, 128);
}

static void building_load(building *b, buffer *buf)
{
    buffer_read_raw(buf, b, 128);
}

void building_save_state(buffer *buf)
{
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        building_save(&Data_Buildings[i], buf);
    }
}

void building_load_state(buffer *buf)
{
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        building_load(&Data_Buildings[i], buf);
        Data_Buildings[i].id = i;
    }
}
