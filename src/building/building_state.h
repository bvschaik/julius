#ifndef BUILDING_BUILDING_STATE_H
#define BUILDING_BUILDING_STATE_H

#include "building/building.h"
#include "core/buffer.h"

void building_state_save_to_buffer(buffer *buf, const building *b);

#endif // BUILDING_BUILDING_STATE_H
