#ifndef MAP_IMAGE_H
#define MAP_IMAGE_H

#include "core/buffer.h"

unsigned int map_image_at(int grid_offset);

void map_image_set(int grid_offset, int image_id);

void map_image_backup(void);

void map_image_restore(void);

void map_image_restore_at(int grid_offset);

void map_image_clear(void);
void map_image_init_edges(void);
void map_image_update_all(void);

void map_image_save_state_legacy(buffer *buf);

void map_image_load_state_legacy(buffer *buf);

#endif // MAP_IMAGE_H
