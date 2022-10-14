#ifndef MAP_AQUEDUCT_H
#define MAP_AQUEDUCT_H

#include "core/buffer.h"


int map_aqueduct_has_water_access_at(int grid_offset);
int map_aqueduct_image_at(int grid_offset);

void map_aqueduct_set_water_access(int grid_offset, int value);
void map_aqueduct_set_image(int grid_offset, int value);

/**
 * Removes aqueduct and updates adjacent aqueduct tiles
 * @param grid_offset Offset
 */
void map_aqueduct_remove(int grid_offset);

void map_aqueduct_clear(void);

void map_aqueduct_backup(void);

void map_aqueduct_restore(void);

void map_aqueduct_save_state(buffer *buf, buffer *backup);

void map_aqueduct_load_state(buffer *buf, buffer *backup);

#endif // MAP_AQUEDUCT_H
