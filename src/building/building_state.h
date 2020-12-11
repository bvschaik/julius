#ifndef BUILDING_BUILDING_STATE_H
#define BUILDING_BUILDING_STATE_H

#include "building/building.h"
#include "core/buffer.h"

#define BUILDING_STATE_ORIGINAL_BUFFER_SIZE 128
#define BUILDING_STATE_CURRENT_BUFFER_SIZE 128

void building_state_save_to_buffer(buffer *buf, const building *b);

void building_state_load_from_buffer(buffer *buf, building *b, int building_buf_size);

#endif // BUILDING_BUILDING_STATE_H
