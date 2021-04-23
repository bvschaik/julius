#ifndef CITY_GAMES_H
#define CITY_GAMES_H

#include "city/resource.h"
#include "translation/translation.h"

#define MAX_GAMES 3

typedef struct {
    int id;
    int header_key;
    int description_key;
    int cost_base;
    int cost_scaling;
    int delay_months;
    int duration_days;
    int bonus_duration;
    int building_id_required;
    int resource_cost[RESOURCE_MAX];
} games_type;

void city_games_schedule(int game_id);
void city_games_decrement_month_counts(void);
void city_games_decrement_duration(void);

games_type *city_games_get_game_type(int id);

#endif // CITY_GAMES_H
