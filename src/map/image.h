#ifndef MAP_IMAGE_H
#define MAP_IMAGE_H

#include "core/buffer.h"

int map_image_at(int grid_offset);

void map_image_set(int grid_offset, int image_id);

void map_image_backup();

void map_image_restore();

void map_image_restore_at(int grid_offset);

void map_image_clear();

void map_image_save_state(buffer *buf);

void map_image_load_state(buffer *buf);

#endif // MAP_IMAGE_H
