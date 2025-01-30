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
    BUILDING_STORAGE_PERMISSION_BARKEEP = 5,
    BUILDING_STORAGE_PERMISSION_CARAVANSERAI = 6,
    BUILDING_STORAGE_PERMISSION_LIGHTHOUSE = 7,
    BUILDING_STORAGE_PERMISSION_ARMOURY = 8,
    BUILDING_STORAGE_PERMISSION_WORKCAMP = 9,
} building_storage_permission_states;

/**
 * Building storage struct
 */
typedef struct {
    int empty_all;
    building_storage_state resource_state[RESOURCE_MAX];
    int permissions;
} building_storage;

typedef struct {
    unsigned int id;
    int in_use;
    int building_id;
    building_storage storage;
} data_storage;

/**
 * Clear and reset all building storages
 */
void building_storage_clear_all(void);

/**
 * Creates a building storage
 * @param building_id The id of the building this is a storage for
 * @return storage id, 0 when creation failed
 */
int building_storage_create(int building_id);

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
 * Gets the size of the storages array.
 * @return size of array
 */
int building_storage_get_array_size(void);

/**
 * Gets an entry from the data storage array.
 * @return Read-only data entry.
 */
const data_storage *building_storage_get_array_entry(int storage_id);

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
 * Sets all goods to 'accepting'
 * @param storage_id Storage id
 */
void building_storage_accept_all(int storage_id);

/**
 * Checks if all goods are set to 'not accepting'
 * Returns 1 if they are and 0 otherwise
 * @param storage_id Storage id
 */
int building_storage_check_if_accepts_nothing(int storage_id);

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
 * Gets the maximum (in full units) a given storage will store of a given resource
 * @param b The building to check
 * @param resource_id Resource id
 * @return Max amount that can be stored, 0 if it does not accept the resource at all.
 */
int building_storage_resource_max_storable(building *b, resource_type resource_id);

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
