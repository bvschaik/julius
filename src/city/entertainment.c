#include "entertainment.h"

#include "building/building.h"
#include "city/data_private.h"

int city_entertainment_theater_shows(void)
{
    return city_data.entertainment.theater_shows;
}

int city_entertainment_amphitheater_shows(void)
{
    return city_data.entertainment.amphitheater_shows;
}

int city_entertainment_arena_shows(void)
{
    return city_data.entertainment.arena_shows;
}

int city_entertainment_colosseum_shows(void)
{
    return city_data.entertainment.colosseum_shows;
}

int city_entertainment_hippodrome_shows(void)
{
    return city_data.entertainment.hippodrome_shows;
}

void city_entertainment_set_hippodrome_has_race(int has_race)
{
    city_data.entertainment.hippodrome_has_race = has_race;
}

int city_entertainment_hippodrome_has_race(void)
{
    return city_data.entertainment.hippodrome_has_race;
}

int city_entertainment_venue_needing_shows(void)
{
    return city_data.entertainment.venue_needing_shows;
}

void city_entertainment_calculate_shows(void)
{
    city_data.entertainment.theater_shows = 0;
    city_data.entertainment.theater_no_shows_weighted = 0;
    city_data.entertainment.amphitheater_shows = 0;
    city_data.entertainment.amphitheater_no_shows_weighted = 0;
    city_data.entertainment.arena_shows = 0;
    city_data.entertainment.arena_no_shows_weighted = 0;
    city_data.entertainment.colosseum_shows = 0;
    city_data.entertainment.colosseum_no_shows_weighted = 0;
    city_data.entertainment.hippodrome_shows = 0;
    city_data.entertainment.hippodrome_no_shows_weighted = 0;
    city_data.entertainment.venue_needing_shows = 0;

    for (building *b = building_first_of_type(BUILDING_THEATER); b; b = b->next_of_type) {
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        if (b->data.entertainment.days1) {
            city_data.entertainment.theater_shows++;
        } else {
            city_data.entertainment.theater_no_shows_weighted++;
        }
    }
    for (building *b = building_first_of_type(BUILDING_AMPHITHEATER); b; b = b->next_of_type) {
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
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
    }
    for (building *b = building_first_of_type(BUILDING_ARENA); b; b = b->next_of_type) {
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        if (b->data.entertainment.days1) {
            city_data.entertainment.arena_shows++;
        } else {
            city_data.entertainment.arena_no_shows_weighted += 3;
        }
        if (b->data.entertainment.days2) {
            city_data.entertainment.arena_shows++;
        } else {
            city_data.entertainment.arena_no_shows_weighted += 3;
        }
    }

    for (building *b = building_first_of_type(BUILDING_COLOSSEUM); b; b = b->next_of_type) {
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
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
    }

    for (building *b = building_first_of_type(BUILDING_HIPPODROME); b; b = b->next_of_type) {
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        if (b->data.entertainment.days1) {
            city_data.entertainment.hippodrome_shows++;
        } else {
            city_data.entertainment.hippodrome_no_shows_weighted += 100;
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

int city_entertainment_show_message_colosseum(void)
{
    if (!city_data.entertainment.colosseum_message_shown) {
        city_data.entertainment.colosseum_message_shown = 1;
        return 1;
    } else {
        return 0;
    }
}

int city_entertainment_show_message_hippodrome(void)
{
    if (!city_data.entertainment.hippodrome_message_shown) {
        city_data.entertainment.hippodrome_message_shown = 1;
        return 1;
    } else {
        return 0;
    }
}


