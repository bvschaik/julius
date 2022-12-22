#ifndef FIGURE_VISITED_BUILDINGS_H
#define FIGURE_VISITED_BUILDINGS_H

#include "core/buffer.h"

typedef struct {
    int index;
    int building_id;
    int prev_index;
} visited_building;

/**
 * Initializes the visited buildings list
 */
void figure_visited_buildings_init(void);

/**
 * Checks if a specific building has already on the visited list
 * @param index The index of the last element of the list
 * @param building_id The building to check
 * @return 1 if the building is already on the list, 0 otherwise
 */
int figure_visited_building_in_list(int index, int building_id);

/**
 * Adds a building to the list of visited buildings
 * @param index The index of the last element of the list
 * @param building_id The building to add
 * @return A new list index if the building was added or the same index if the building was already on the list
 */
int figure_visited_buildings_add(int index, int building_id);

/**
 * Removes an entire list of visited buildings, started at index.
 *
 * The function recursively removes the current index and all last indexes associated to the index
 * To prevent dangling lists, make sure you remove from the last actual index for the list!
 *
 * @param index The index of the last element on the list to remove
 */
void figure_visited_buildings_remove_list(int index);

/**
 * Migrates the list of visited buildings from the old format to the new one
 */
void figure_visited_buildings_migrate(void);

/**
 * Save state to buffer
 * @param buf Buffer
 */
void figure_visited_buildings_save_state(buffer *buf);

/**
 * Load state from buffer
 * @param buf Buffer
 */
void figure_visited_buildings_load_state(buffer *buf);

#endif // FIGURE_VISITED_BUILDINGS_H
