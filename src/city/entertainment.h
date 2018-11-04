#ifndef CITY_ENTERTAINMENT_H
#define CITY_ENTERTAINMENT_H

int city_entertainment_theater_shows(void);
int city_entertainment_amphitheater_shows(void);
int city_entertainment_colosseum_shows(void);
int city_entertainment_hippodrome_shows(void);

void city_entertainment_set_hippodrome_has_race(int has_race);
int city_entertainment_hippodrome_has_race(void);

int city_entertainment_venue_needing_shows(void);

void city_entertainment_calculate_shows(void);

int city_entertainment_show_message_colosseum(void);

int city_entertainment_show_message_hippodrome(void);

#endif // CITY_ENTERTAINMENT_H
