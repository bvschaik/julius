#ifndef CITY_POPULATION_H
#define CITY_POPULATION_H

int city_population();
int city_population_school_age();
int city_population_academy_age();

/**
 * Add people to the city.
 * @param num_people Number of people to add
 */
void city_population_add(int num_people);

/**
 * Add people returning after becoming homeless.
 * @param num_people Number of people to add
 */
void city_population_add_homeless(int num_people);

/**
 * Remove people from the city.
 * @param num_people Number of people to remove
 */
void city_population_remove(int num_people);

void city_population_remove_homeless(int num_people);

void city_population_remove_home_removed(int num_people);

void city_population_remove_for_troop_request(int num_people);

int city_population_people_of_working_age();

void city_population_calculate_educational_age();

void city_population_record_monthly();

int city_population_monthly_count();

int city_population_at_month(int max_months, int month);

int city_population_at_age(int age);

int city_population_at_level(int house_level);

void city_population_request_yearly_update();

void city_population_yearly_update();

void city_population_check_consistency();

#endif // CITY_POPULATION_H
