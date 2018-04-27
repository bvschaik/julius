#ifndef CITY_GODS_H
#define CITY_GODS_H

#define MAX_GODS 5

void city_gods_reset();
void city_gods_reset_neptune_blessing();

void city_gods_calculate_moods(int update_moods);

int city_gods_calculate_least_happy();

int city_god_happiness(int god_id);

int city_god_wrath_bolts(int god_id);

int city_god_months_since_festival(int god_id);

/**
 * @return God ID or -1 if no single god is the least happy
 */
int city_god_least_happy();

int city_god_spirit_of_mars_power();
void city_god_spirit_of_mars_mark_used();

int city_god_neptune_create_shipwreck_flotsam();

#endif // CITY_GODS_H
