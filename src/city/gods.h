#ifndef CITY_GODS_H
#define CITY_GODS_H

#define MAX_GODS 5
#define NEPTUNE_BLESSING_MONTHS 12
#define VENUS_BLESSING_MONTHS 35

void city_gods_reset(void);
void city_gods_reset_neptune_blessing(void);
void city_gods_update_blessings(void);

void city_gods_calculate_moods(int update_moods);

int city_gods_calculate_least_happy(void);

int city_god_happiness(int god_id);

int city_god_wrath_bolts(int god_id);

int city_god_happy_bolts(int god_id);

int city_god_months_since_festival(int god_id);

/**
 * @return God ID or -1 if no single god is the least happy
 */
int city_god_least_happy(void);

int city_god_spirit_of_mars_power(void);
void city_god_spirit_of_mars_mark_used(void);

int city_god_neptune_create_shipwreck_flotsam(void);

int city_god_venus_bonus_employment(void);

void city_god_blessing_cheat(int god_id);

#endif // CITY_GODS_H
