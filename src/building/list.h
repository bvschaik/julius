#ifndef BUILDING_LIST_H
#define BUILDING_LIST_H

#include "core/buffer.h"

/**
 * @file
 * Temporary building lists for tick processing
 */

/**
 * Clears the small building list
 */
void building_list_small_clear(void);

/**
 * Adds a building to the small list
 * @param building_id Building ID
 */
void building_list_small_add(int building_id);

/**
 * Returns the number of buildings on the small list
 * @return List size
 */
int building_list_small_size(void);

/**
 * Returns the items on the small list
 * @return List of building IDs
 */
const int *building_list_small_items(void);

/**
 * Clears the large building list
 * @param clear_entries Whether to clear the item values as well
 */
void building_list_large_clear(int clear_entries);

/**
 * Adds a building to the large list
 * @param building_id Building ID
 */
void building_list_large_add(int building_id);

/**
 * Returns the number of buildings on the large list
 * @return List size
 */
int building_list_large_size(void);

/**
 * Returns the items on the large list
 * @return List of building IDs
 */
const int *building_list_large_items(void);

void building_list_burning_clear(void);

void building_list_burning_add(int building_id);

int building_list_burning_size(void);

const int *building_list_burning_items(void);

void building_list_save_state(buffer *small, buffer *large, buffer *burning, buffer *burning_totals);

void building_list_load_state(buffer *small, buffer *large, buffer *burning, buffer *burning_totals);

#endif // BUILDING_LIST_H
