#include "building.h"

#include <string.h>

struct Data_Building *building_get(int id)
{
    return &Data_Buildings[id];
}

void building_delete(struct Data_Building *b)
{
    memset(b, 0, sizeof(struct Data_Building));
}

void building_clear_all()
{
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        memset(&Data_Buildings[i], 0, sizeof(struct Data_Building));
    }
}

static void building_save(struct Data_Building *b, buffer *buf)
{
    buffer_write_raw(buf, b, 128);
}

static void building_load(struct Data_Building *b, buffer *buf)
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
        //Data_Buildings[i].id = i;
    }
}
