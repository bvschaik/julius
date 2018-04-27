#include "entertainment.h"

#include "building/building.h"
#include "city/data_private.h"

#include "Data/CityInfo.h"

int city_entertainment_theater_shows()
{
    return Data_CityInfo.entertainmentTheaterShows;
}

int city_entertainment_amphitheater_shows()
{
    return Data_CityInfo.entertainmentAmphitheaterShows;
}

int city_entertainment_colosseum_shows()
{
    return Data_CityInfo.entertainmentColosseumShows;
}

int city_entertainment_hippodrome_shows()
{
    return Data_CityInfo.entertainmentHippodromeShows;
}

void city_entertainment_set_hippodrome_has_show(int has_show)
{
    Data_CityInfo.entertainmentHippodromeHasShow = has_show;
}

int city_entertainment_hippodrome_has_show()
{
    return Data_CityInfo.entertainmentHippodromeHasShow;
}

int city_entertainment_venue_needing_shows()
{
    return Data_CityInfo.entertainmentNeedingShowsMost;
}

void city_entertainment_calculate_shows()
{
    Data_CityInfo.entertainmentTheaterShows = 0;
    Data_CityInfo.entertainmentTheaterNoShowsWeighted = 0;
    Data_CityInfo.entertainmentAmphitheaterShows = 0;
    Data_CityInfo.entertainmentAmphitheaterNoShowsWeighted = 0;
    Data_CityInfo.entertainmentColosseumShows = 0;
    Data_CityInfo.entertainmentColosseumNoShowsWeighted = 0;
    Data_CityInfo.entertainmentHippodromeShows = 0;
    Data_CityInfo.entertainmentHippodromeNoShowsWeighted = 0;
    Data_CityInfo.entertainmentNeedingShowsMost = 0;

    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        switch (b->type) {
            case BUILDING_THEATER:
                if (b->data.entertainment.days1) {
                    Data_CityInfo.entertainmentTheaterShows++;
                } else {
                    Data_CityInfo.entertainmentTheaterNoShowsWeighted++;
                }
                break;
            case BUILDING_AMPHITHEATER:
                if (b->data.entertainment.days1) {
                    Data_CityInfo.entertainmentAmphitheaterShows++;
                } else {
                    Data_CityInfo.entertainmentAmphitheaterNoShowsWeighted += 2;
                }
                if (b->data.entertainment.days2) {
                    Data_CityInfo.entertainmentAmphitheaterShows++;
                } else {
                    Data_CityInfo.entertainmentAmphitheaterNoShowsWeighted += 2;
                }
                break;
            case BUILDING_COLOSSEUM:
                if (b->data.entertainment.days1) {
                    Data_CityInfo.entertainmentColosseumShows++;
                } else {
                    Data_CityInfo.entertainmentColosseumNoShowsWeighted += 3;
                }
                if (b->data.entertainment.days2) {
                    Data_CityInfo.entertainmentColosseumShows++;
                } else {
                    Data_CityInfo.entertainmentColosseumNoShowsWeighted += 3;
                }
                break;
            case BUILDING_HIPPODROME:
                if (b->data.entertainment.days1) {
                    Data_CityInfo.entertainmentHippodromeShows++;
                } else {
                    Data_CityInfo.entertainmentHippodromeNoShowsWeighted += 100;
                }
                break;
        }
    }
    int worstShows = 0;
    if (Data_CityInfo.entertainmentTheaterNoShowsWeighted > worstShows) {
        worstShows = Data_CityInfo.entertainmentTheaterNoShowsWeighted;
        Data_CityInfo.entertainmentNeedingShowsMost = 1;
    }
    if (Data_CityInfo.entertainmentAmphitheaterNoShowsWeighted > worstShows) {
        worstShows = Data_CityInfo.entertainmentAmphitheaterNoShowsWeighted;
        Data_CityInfo.entertainmentNeedingShowsMost = 2;
    }
    if (Data_CityInfo.entertainmentColosseumNoShowsWeighted > worstShows) {
        worstShows = Data_CityInfo.entertainmentColosseumNoShowsWeighted;
        Data_CityInfo.entertainmentNeedingShowsMost = 3;
    }
    if (Data_CityInfo.entertainmentHippodromeNoShowsWeighted > worstShows) {
        Data_CityInfo.entertainmentNeedingShowsMost = 4;
    }
}
