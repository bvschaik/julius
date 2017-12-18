#ifndef BUILDING_BUILDING_H
#define BUILDING_BUILDING_H

#include "core/buffer.h"

#include "Data/Building.h"

#define MAX_BUILDINGS 2000

struct Data_Building *building_get(int id);

void building_delete(struct Data_Building *b);

void building_clear_all();

void building_save_state(buffer *buf);

void building_load_state(buffer *buf);

#endif // BUILDING_BUILDING_H
