#include "entertainment.h"

#include "building/building.h"
#include "city/data_private.h"

#include "Data/CityInfo.h"

int city_entertainment_theater_shows()
{
    return city_data.entertainment.theater_shows;
}

int city_entertainment_amphitheater_shows()
{
    return city_data.entertainment.amphitheater_shows;
}

int city_entertainment_colosseum_shows()
{
    return city_data.entertainment.colosseum_shows;
}

int city_entertainment_hippodrome_shows()
{
    return city_data.entertainment.hippodrome_shows;
}

void city_entertainment_set_hippodrome_has_race(int has_race)
{
    city_data.entertainment.hippodrome_has_race = has_race;
}

int city_entertainment_hippodrome_has_race()
{
    return city_data.entertainment.hippodrome_has_race;
}

int city_entertainment_venue_needing_shows()
{
    return city_data.entertainment.venue_needing_shows;
}

void city_entertainment_calculate_shows()
{
    city_data.entertainment.theater_shows = 0;
    city_data.entertainment.theater_no_shows_weighted = 0;
    city_data.entertainment.amphitheater_shows = 0;
    city_data.entertainment.amphitheater_no_shows_weighted = 0;
    city_data.entertainment.colosseum_shows = 0;
    city_data.entertainment.colosseum_no_shows_weighted = 0;
    city_data.entertainment.hippodrome_shows = 0;
    city_data.entertainment.hippodrome_no_shows_weighted = 0;
    city_data.entertainment.venue_needing_shows = 0;

    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        switch (b->type) {
            case BUILDING_THEATER:
                if (b->data.entertainment.days1) {
                    city_data.entertainment.theater_shows++;
                } else {
                    city_data.entertainment.theater_no_shows_weighted++;
                }
                break;
            case BUILDING_AMPHITHEATER:
                if (b->data.entertainment.days1) {
                    city_data.entertainment.amphitheater_shows++;
                } else {
                    city_data.entertainment.amphitheater_no_shows_weighted += 2;
                }
                if (b->data.entertainment.days2) {
                    city_data.entertainment.amphitheater_shows++;
                } else {
                    city_data.entertainment.amphitheater_no_shows_weighted += 2;
                }
                break;
            case BUILDING_COLOSSEUM:
                if (b->data.entertainment.days1) {
                    city_data.entertainment.colosseum_shows++;
                } else {
                    city_data.entertainment.colosseum_no_shows_weighted += 3;
                }
                if (b->data.entertainment.days2) {
                    city_data.entertainment.colosseum_shows++;
                } else {
                    city_data.entertainment.colosseum_no_shows_weighted += 3;
                }
                break;
            case BUILDING_HIPPODROME:
                if (b->data.entertainment.days1) {
                    city_data.entertainment.hippodrome_shows++;
                } else {
                    city_data.entertainment.hippodrome_no_shows_weighted += 100;
                }
                break;
        }
    }
    int worst_shows = 0;
    if (city_data.entertainment.theater_no_shows_weighted > worst_shows) {
        worst_shows = city_data.entertainment.theater_no_shows_weighted;
        city_data.entertainment.venue_needing_shows = 1;
    }
    if (city_data.entertainment.amphitheater_no_shows_weighted > worst_shows) {
        worst_shows = city_data.entertainment.amphitheater_no_shows_weighted;
        city_data.entertainment.venue_needing_shows = 2;
    }
    if (city_data.entertainment.colosseum_no_shows_weighted > worst_shows) {
        worst_shows = city_data.entertainment.colosseum_no_shows_weighted;
        city_data.entertainment.venue_needing_shows = 3;
    }
    if (city_data.entertainment.hippodrome_no_shows_weighted > worst_shows) {
        city_data.entertainment.venue_needing_shows = 4;
    }
}
