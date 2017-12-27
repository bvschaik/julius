#ifndef BUILDING_HOUSE_POPULATION_H
#define BUILDING_HOUSE_POPULATION_H

/**
 * Add people to the houses in the city, use for births
 * @param num_people Number of people to add
 * @return Number of people added
 */
int house_population_add_to_city(int num_people);

/**
 * Removes people from the houses in the city, use for births and troop requests
 * @param num_people Number of people to remove
 * @return Number of people removed
 */
int house_population_remove_from_city(int num_people);

/**
 * Calculates aggregates of people per housing type
 * @return Total population
 */
int house_population_calculate_people_per_type();

/**
 * Update room available in houses
 */
void house_population_update_room();

/**
 * Update migration statistics and create immigrants/emigrants
 */
void house_population_update_migration();

/**
 * Evict people from overcrowded houses
 */
void house_population_evict_overcrowded();

#endif // BUILDING_HOUSE_POPULATION_H
