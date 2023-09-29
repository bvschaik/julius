#ifndef BUILDING_COUNT_H
#define BUILDING_COUNT_H

#include "core/buffer.h"
#include "building/type.h"
#include "figure/type.h"
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
 * Returns the building count of all building types
 * @param active_only Only count the building if it is active
 * @return Total number of buildings
 */
int building_count_any_total(int active_only);

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
int building_count_grand_temples_active(void);

/**
 * Returns the building count for the type in the given area
 * @param type Building type
 * @param minx Minimum x-axis value of the area to count in
 * @param miny Minimum y-axis value of the area to count in
 * @param maxx Maximum x-axis value of the area to count in
 * @param maxy Maximum y-axis value of the area to count in
 * @return Total number of buildings
 */
int building_count_in_area(building_type type, int minx, int miny, int maxx, int maxy);

/**
 * Returns the building count for any type in the given area
 * @param minx Minimum x-axis value of the area to count in
 * @param miny Minimum y-axis value of the area to count in
 * @param maxx Maximum x-axis value of the area to count in
 * @param maxy Maximum y-axis value of the area to count in
 * @param type Figure type
 * @return Total number of buildings
 */
int building_count_fort_type_in_area(int minx, int miny, int maxx, int maxy, figure_type type);

/**
 * Returns the total number of buildings (where the type is from the set of buildings)
 * @param active_only Only count the building if it is active
 * @return Number of buildings
 */
int building_set_count_farms(int active_only);
int building_set_count_raw_materials(int active_only);
int building_set_count_workshops(int active_only);
int building_set_count_small_temples(int active_only);
int building_set_count_large_temples(int active_only);

/**
 * Returns the total number of buildings (where the type is from the set of buildings)
 * @return Number of buildings
 */
int building_set_count_deco_trees(void);
int building_set_count_deco_paths(void);
int building_set_count_deco_statues(void);

/**
 * Returns the total number of buildings (where the type is from the set of buildings) in the given area
 * @param minx Minimum x-axis value of the area to count in
 * @param miny Minimum y-axis value of the area to count in
 * @param maxx Maximum x-axis value of the area to count in
 * @param maxy Maximum y-axis value of the area to count in
 * @return Number of buildings
 */
int building_set_area_count_farms(int minx, int miny, int maxx, int maxy);
int building_set_area_count_raw_materials(int minx, int miny, int maxx, int maxy);
int building_set_area_count_workshops(int minx, int miny, int maxx, int maxy);
int building_set_area_count_small_temples(int minx, int miny, int maxx, int maxy);
int building_set_area_count_large_temples(int minx, int miny, int maxx, int maxy);
int building_set_area_count_grand_temples(int minx, int miny, int maxx, int maxy);
int building_set_area_count_deco_trees(int minx, int miny, int maxx, int maxy);
int building_set_area_count_deco_paths(int minx, int miny, int maxx, int maxy);
int building_set_area_count_deco_statues(int minx, int miny, int maxx, int maxy);

/**
 * Returns the active building count for forts based on the assigned soldier (figure) type
 * @param type Figure type
 * @return Number of active forts of that type
 */
int building_count_active_fort_type(figure_type type);

/**
 * Returns the building count for forts based on the assigned soldier (figure) type
 * @param type Figure type
 * @return Total number of forts of that type
 */
int building_count_fort_type_total(figure_type type);

#endif // BUILDING_COUNT_H
