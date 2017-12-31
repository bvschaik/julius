#ifndef CITY_RATINGS_H
#define CITY_RATINGS_H

void city_ratings_reduce_prosperity_after_bailout();

void city_ratings_change_favor(int amount);

void city_ratings_limit_favor(int max_favor);

void city_ratings_update_favor_explanation();

void city_ratings_update_explanations();

void city_ratings_update(int is_yearly_update);

#endif // CITY_RATINGS_H
