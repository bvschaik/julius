#ifndef BUILDING_COUNT_H
#define BUILDING_COUNT_H

#include "core/buffer.h"
#include "building/type.h"
#include "game/resource.h"

/**
 * @file
 * Building totals
 */

/**
 * Returns the active building count for the type
 * @param type Building type
 * @return Number of active buildings
 */
int building_count_active(building_type type);

/**
 * Returns the building count for the type
 * @param type Building type
 * @return Total number of buildings
 */
int building_count_total(building_type type);

/**
 * Returns the upgraded building count for the type
 * @param type Building type
 * @return Number of upgraded buildings
 */
int building_count_upgraded(building_type type);

/**
 * Returns the total number of grand temples
 * @return Number of total grand temples
 */
int building_count_grand_temples(void);

/**
 * Returns the total number of active grand temples
 * @return Number of active grand temples
 */
int building_count_grand_temples(void);
int building_count_grand_temples_active(void);

#endif // BUILDING_COUNT_H
