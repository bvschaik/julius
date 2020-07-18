#ifndef CITY_RATINGS_H
#define CITY_RATINGS_H

#include "building/type.h"

typedef enum {
    SELECTED_RATING_NONE = 0,
    SELECTED_RATING_CULTURE = 1,
    SELECTED_RATING_PROSPERITY = 2,
    SELECTED_RATING_PEACE = 3,
    SELECTED_RATING_FAVOR = 4
} selected_rating;

int city_rating_culture(void);
int city_rating_prosperity(void);
int city_rating_peace(void);
int city_rating_favor(void);

selected_rating city_rating_selected(void);
void city_rating_select(selected_rating rating);
int city_rating_selected_explanation(void);

void city_ratings_reduce_prosperity_after_bailout(void);

void city_ratings_peace_building_destroyed(building_type type);

void city_ratings_peace_record_criminal(void);

void city_ratings_peace_record_rioter(void);

void city_ratings_change_favor(int amount);

void city_ratings_reset_favor_emperor_change(void);

void city_ratings_reduce_favor_missed_request(int penalty);

void city_ratings_limit_favor(int max_favor);

void city_ratings_update_favor_explanation(void);

void city_ratings_update_explanations(void);

void city_ratings_update(int is_yearly_update);

int city_ratings_prosperity_max(void);

#endif // CITY_RATINGS_H
