#ifndef BUILDING_STORAGE_H
#define BUILDING_STORAGE_H

#include <stdint.h>

#include "core/buffer.h"
#include "game/resource.h"

/**
 * @file
 * Building storage functions
 */

enum {
    BUILDING_STORAGE_DATA
};

/**
 * Storage state
 */
typedef enum {
    BUILDING_STORAGE_STATE_ACCEPTING = 0,
    BUILDING_STORAGE_STATE_NOT_ACCEPTING = 1,
    BUILDING_STORAGE_STATE_GETTING = 2
} building_storage_state;

/**
 * Building storage struct
 */
typedef struct {
    int empty_all;
    building_storage_state resource_state[RESOURCE_MAX];
} building_storage;

/**
 * Clear and reset all building storages
 */
void building_storage_clear_all();

/**
 * Creates a building storage
 * @return storage id, 0 when creation failed
 */
int building_storage_create();

/**
 * Deletes a building storage
 * @param storage_id Storage id
 */
void building_storage_delete(int storage_id);

/**
 * Gets a read-only building storage
 * @param storage_id Storage id
 * @return Read-only storage
 */
const building_storage *building_storage_get(int storage_id);

/**
 * Cycles the resource state for the storage
 * @param storage_id Storage id
 * @param resource_id Resource id
 */
void building_storage_cycle_resource_state(int storage_id, resource_type resource_id);

/**
 * Toggles the empty all flag for the storage
 * @param storage_id Storage id
 */
void building_storage_toggle_empty_all(int storage_id);

/**
 * Resets building id's for all storages
 */
void building_storage_reset_building_ids();

/**
 * Save data
 * @param buf Buffer to read from
 */
void building_storage_save_state(buffer *buf);

/**
 * Load data
 * @param buf Buffer to read from
 */
void building_storage_load_state(buffer *buf);

#endif // BUILDING_STORAGE_H
