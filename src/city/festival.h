#ifndef CITY_FESTIVAL_H
#define CITY_FESTIVAL_H

#include "city/resource.h"
#include "translation/translation.h"

#define MAX_GAMES 3

typedef struct {
	int god_id;
	int month;
} auto_festival;

int city_festival_is_planned(void);
int city_festival_months_since_last(void);

int city_festival_small_cost(void);
int city_festival_large_cost(void);
int city_festival_grand_cost(void);
int city_festival_grand_wine(void);

int city_festival_out_of_wine(void);

int city_festival_selected_god(void);
void city_festival_select_god(int god_id);

int city_festival_selected_size(void);
int city_festival_select_size(int size);

int city_festival_selected_game_id(void);
int city_festival_games_active(void);
int city_festival_games_active_venue_type(void);
int city_festival_games_bonus_active(int game_id);
int city_festival_games_remaining_duration(void);
int city_festival_games_planning_time(void);
int city_festival_games_cooldown(void);

void city_festival_schedule(void);

void city_festival_update(void);
void city_festival_calculate_costs(void);

#endif // CITY_FESTIVAL_H
