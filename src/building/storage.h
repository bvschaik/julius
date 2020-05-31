#ifndef BUILDING_STORAGE_H
#define BUILDING_STORAGE_H

#include "building/building.h"
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
    BUILDING_STORAGE_STATE_GETTING = 2,
    BUILDING_STORAGE_STATE_ACCEPTING_HALF = 3,
    BUILDING_STORAGE_STATE_ACCEPTING_QUARTER = 4,
    BUILDING_STORAGE_STATE_GETTING_HALF = 5,
    BUILDING_STORAGE_STATE_GETTING_QUARTER = 6,
    BUILDING_STORAGE_STATE_GETTING_3QUARTERS = 7,
    BUILDING_STORAGE_STATE_ACCEPTING_3QUARTERS = 8,
} building_storage_state;

typedef enum {
    BUILDING_STORAGE_PERMISSION_MARKET = 0,
    BUILDING_STORAGE_PERMISSION_TRADERS = 1,
    BUILDING_STORAGE_PERMISSION_DOCK = 2,
} building_storage_permission_states;


/**
 * Building storage struct
 */
typedef struct {
    int empty_all;
    building_storage_state resource_state[RESOURCE_MAX];
    int permissions;
} building_storage;

/**
 * Clear and reset all building storages
 */
void building_storage_clear_all(void);

/**
 * Creates a building storage
 * @return storage id, 0 when creation failed
 */
int building_storage_create(void);

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

void building_storage_cycle_partial_resource_state(int storage_id, resource_type resource_id);
/**
 * Sets all goods to 'not accepting'
 * @param storage_id Storage id
 */
void building_storage_accept_none(int storage_id);

/**
 * Toggles the empty all flag for the storage
 * @param storage_id Storage id
 */
void building_storage_toggle_empty_all(int storage_id);

/**
 * Resets building id's for all storages
 */
void building_storage_reset_building_ids(void);

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



void building_storage_set_permission(building_storage_permission_states p, building *b);
int building_storage_get_permission(building_storage_permission_states p, building *b);


#endif // BUILDING_STORAGE_H
