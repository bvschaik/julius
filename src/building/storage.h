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
    BUILDING_STORAGE_STATE_NOT_ACCEPTING_HALF = 9,
    BUILDING_STORAGE_STATE_NOT_ACCEPTING_QUARTER = 10,
    BUILDING_STORAGE_STATE_NOT_ACCEPTING_3QUARTERS = 11,
} building_storage_state;

typedef enum {
    BUILDING_STORAGE_PERMISSION_MARKET = 0,
    BUILDING_STORAGE_PERMISSION_TRADERS = 1,
    BUILDING_STORAGE_PERMISSION_DOCK = 2,
    BUILDING_STORAGE_PERMISSION_QUARTERMASTER = 3,
    BUILDING_STORAGE_PERMISSION_WORKER = 4,
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
 * Restores a building storage after undoing destruction.
 * @param storage_id Storage id
 * @return storage id, 0 when storage already used.
 */
int building_storage_restore(int storage_id);

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
 * Sets values of a building storage to that of another building storage
 * @param storage_id Storage id to be changed
 * @param new_data New values of storage data
 */
void building_storage_set_data(int storage_id, building_storage new_data);


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
 * @param version The save version
 */
void building_storage_load_state(buffer *buf, int version);



void building_storage_set_permission(building_storage_permission_states p, building *b);
int building_storage_get_permission(building_storage_permission_states p, building *b);


#endif // BUILDING_STORAGE_H
