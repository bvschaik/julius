#ifndef CITY_ENTERTAINMENT_H
#define CITY_ENTERTAINMENT_H

int city_entertainment_theater_shows();
int city_entertainment_amphitheater_shows();
int city_entertainment_colosseum_shows();
int city_entertainment_hippodrome_shows();

void city_entertainment_set_hippodrome_has_show(int has_show);
int city_entertainment_hippodrome_has_show();

int city_entertainment_venue_needing_shows();

void city_entertainment_calculate_shows();

#endif // CITY_ENTERTAINMENT_H
