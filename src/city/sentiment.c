#include "sentiment.h"

#include "building/building.h"
#include "building/model.h"
#include "city/constants.h"
#include "city/data_private.h"
#include "city/figures.h"
#include "city/games.h"
#include "city/gods.h"
#include "city/message.h"
#include "city/population.h"
#include "core/calc.h"
#include "core/config.h"
#include "core/random.h"
#include "game/difficulty.h"
#include "game/tutorial.h"

#include <math.h>
#include <stdlib.h>

#define MAX_SENTIMENT_FROM_EXTRA_ENTERTAINMENT 24
#define MAX_SENTIMENT_FROM_EXTRA_FOOD 24
#define FESTIVAL_BOOST_DECREMENT_RATE 0.84
#define UNEMPLOYMENT_THRESHHOLD 5
#define MAX_TAX_MULTIPLIER 12
#define WAGE_NEGATIVE_MODIFIER 3
#define WAGE_POSITIVE_MODIFIER 2
#define SQUALOR_MULTIPLIER 3
#define SENTIMENT_PER_ENTERTAINMENT 1
#define SENTIMENT_PER_EXTRA_FOOD 12
#define MAX_SENTIMENT_CHANGE 2
#define DESIRABILITY_TO_SENTIMENT_RATIO 2
#define COOLDOWN_AFTER_CRIME_DAYS 10
#define EXECUTIONS_GAMES_SENTIMENT_BONUS 20
#define IMPERIAL_GAMES_SENTIMENT_BONUS 15
#define POP_STEP_FOR_BASE_AVERAGE_HOUSE_LEVEL 625

int city_sentiment(void)
{
    return city_data.sentiment.value;
}

int city_sentiment_low_mood_cause(void)
{
    return city_data.sentiment.low_mood_cause;
}

void city_sentiment_change_happiness(int amount)
{
    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state == BUILDING_STATE_IN_USE && b->house_size) {
                b->sentiment.house_happiness = calc_bound(b->sentiment.house_happiness + amount, 0, 100);
            }
        }
    }
}

void city_sentiment_set_max_happiness(int max)
{
    max = calc_bound(max, 0, 100);
    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state == BUILDING_STATE_IN_USE && b->house_size) {
                if (b->sentiment.house_happiness > max) {
                    b->sentiment.house_happiness = max;
                }
            }
        }
    }
}

void city_sentiment_set_min_happiness(int min)
{
    min = calc_bound(min, 0, 100);
    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state == BUILDING_STATE_IN_USE && b->house_size) {
                if (b->sentiment.house_happiness < min) {
                    b->sentiment.house_happiness = min;
                }
            }
        }
    }
}

int city_sentiment_get_population_below_happiness(int happiness)
{
    int population = 0;
    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state == BUILDING_STATE_IN_USE && b->house_size) {
                if (b->sentiment.house_happiness < happiness) {
                    population += b->house_population;
                }
            }
        }
    }
    return population;
}

void city_sentiment_reset_protesters_criminals(void)
{
    city_data.sentiment.protesters = 0;
    city_data.sentiment.criminals = 0;
}

void city_sentiment_add_protester(void)
{
    city_data.sentiment.protesters++;
}

void city_sentiment_add_criminal(void)
{
    city_data.sentiment.criminals++;
}

int city_sentiment_protesters(void)
{
    return city_data.sentiment.protesters;
}

int city_sentiment_criminals(void)
{
    return city_data.sentiment.criminals;
}

int city_sentiment_crime_cooldown(void)
{
    return city_data.sentiment.crime_cooldown;
}

void city_sentiment_set_crime_cooldown(void)
{
    city_data.sentiment.crime_cooldown = COOLDOWN_AFTER_CRIME_DAYS;
}

void city_sentiment_reduce_crime_cooldown(void)
{
    if (city_sentiment_crime_cooldown() > 0) {
        city_data.sentiment.crime_cooldown -= 1;
    }
}


int city_sentiment_get_blessing_festival_boost(void)
{
    return city_data.sentiment.blessing_festival_boost;
}

void city_sentiment_decrement_blessing_boost(void)
{
    short new_sentiment = (short) (city_data.sentiment.blessing_festival_boost * FESTIVAL_BOOST_DECREMENT_RATE);
    city_data.sentiment.blessing_festival_boost = new_sentiment;
}

static int get_games_bonus(void)
{
    int bonus = 0;
    if (city_games_executions_active()) {
        bonus += EXECUTIONS_GAMES_SENTIMENT_BONUS;
    }
    if (city_games_imperial_festival_active()) {
        bonus += IMPERIAL_GAMES_SENTIMENT_BONUS;
    }
    return bonus;
}

static int get_wage_sentiment_modifier(void)
{
    int wage_differential = city_data.labor.wages - city_data.labor.wages_rome;
    return wage_differential * (wage_differential > 0 ? WAGE_POSITIVE_MODIFIER : WAGE_NEGATIVE_MODIFIER);
}

static int get_unemployment_sentiment_modifier(void)
{
    int unhappiness_treshhold = UNEMPLOYMENT_THRESHHOLD + city_god_venus_bonus_employment();
    if (city_data.labor.unemployment_percentage > unhappiness_treshhold) {
        return (city_data.labor.unemployment_percentage - unhappiness_treshhold);
    }
    return 0;
}

static int get_sentiment_modifier_for_tax_rate(int tax)
{
    int base_tax = difficulty_base_tax_rate();
    int tax_differential = base_tax - tax;
    tax_differential *= tax_differential < 0 ? (MAX_TAX_MULTIPLIER - base_tax) : (base_tax / 2);
    return tax_differential;
}

static int get_average_housing_level(void)
{
    int avg = 0;
    int population = 0;

    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        int multiplier = 1;
        if (type == BUILDING_HOUSE_LARGE_CASA) {
            multiplier = 10;
        } else if (type == BUILDING_HOUSE_SMALL_VILLA) {
            multiplier = 20;
        } else if (type == BUILDING_HOUSE_SMALL_PALACE) {
            multiplier = 30;
        }
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state != BUILDING_STATE_IN_USE || !b->house_size) {
                continue;
            }
            avg += b->subtype.house_level * b->house_population * multiplier;
            population += b->house_population * multiplier;
        }
    }
    if (population) {
        avg = avg / population;
    }
    int base_average_level = population / POP_STEP_FOR_BASE_AVERAGE_HOUSE_LEVEL;
    base_average_level = calc_bound(base_average_level, HOUSE_SMALL_TENT, HOUSE_SMALL_INSULA);
    return avg < base_average_level ? base_average_level : avg;
}

static int house_level_sentiment_modifier(int level, int average)
{
    int diff_from_average = level - average;
    if (diff_from_average < 0) {
        diff_from_average *= SQUALOR_MULTIPLIER;
    }
    return diff_from_average;
}

static int extra_desirability_bonus(int desirability, int max)
{
    int extra = (desirability - 50) / DESIRABILITY_TO_SENTIMENT_RATIO;
    return calc_bound(extra, 0, max);
}

static int extra_entertainment_bonus(int entertainment, int required)
{
    int extra = (entertainment - required) * SENTIMENT_PER_ENTERTAINMENT;
    return calc_bound(extra, 0, MAX_SENTIMENT_FROM_EXTRA_ENTERTAINMENT);
}

static int extra_food_bonus(int types, int required)
{
    // Tents have no food bonus because they always scavenge for food
    if (required == 0) {
        return 0;
    }
    int extra = ((types - required) * SENTIMENT_PER_EXTRA_FOOD);
    return calc_bound(extra, 0, MAX_SENTIMENT_FROM_EXTRA_FOOD);
}

void city_sentiment_update(void)
{
    city_population_check_consistency();

    int default_sentiment = difficulty_sentiment();
    int houses_calculated = 0;
    int sentiment_contribution_taxes = get_sentiment_modifier_for_tax_rate(city_data.finance.tax_percentage);
    int sentiment_contribution_no_tax = get_sentiment_modifier_for_tax_rate(0) / 2;
    int sentiment_contribution_wages = get_wage_sentiment_modifier();
    int sentiment_contribution_unemployment = get_unemployment_sentiment_modifier();
    int average_housing_level = get_average_housing_level();
    int blessing_festival_boost = city_data.sentiment.blessing_festival_boost;
    int average_squalor_penalty = 0;
    int games_bonus = get_games_bonus();

    int total_sentiment = 0;
    int total_pop = 0;
    int total_houses = 0;
    int house_level_sentiment_multiplier = 3;

    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        if (type == BUILDING_HOUSE_SMALL_SHACK) {
            house_level_sentiment_multiplier = 2;
        } else if (type == BUILDING_HOUSE_LARGE_CASA) {
            house_level_sentiment_multiplier = 1;
        } else if (type == BUILDING_HOUSE_SMALL_VILLA) {
            house_level_sentiment_multiplier = 0;
        }
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state != BUILDING_STATE_IN_USE || !b->house_size) {
                continue;
            }
            if (!b->house_population) {
                b->sentiment.house_happiness = calc_bound(city_data.sentiment.value + 10, 0, 100);
                continue;
            }

            int sentiment = default_sentiment;

            if (b->house_tax_coverage) {
                sentiment += sentiment_contribution_taxes;
            } else {
                sentiment += sentiment_contribution_no_tax;
            }

            if (b->subtype.house_level <= HOUSE_GRAND_INSULA) {
                sentiment += sentiment_contribution_wages;
                sentiment -= sentiment_contribution_unemployment;
            }

            int house_level_sentiment = house_level_sentiment_modifier(b->subtype.house_level, average_housing_level);
            if (house_level_sentiment < 0) {
                house_level_sentiment *= house_level_sentiment_multiplier;
                average_squalor_penalty += house_level_sentiment;
            }
            sentiment += house_level_sentiment;

            int max_desirability = b->subtype.house_level + 1;
            int desirability_bonus = extra_desirability_bonus(b->desirability, max_desirability);
            int entertainment_bonus = extra_entertainment_bonus(b->data.house.entertainment,
                model_get_house(b->subtype.house_level)->entertainment);
            int food_bonus = extra_food_bonus(b->data.house.num_foods,
                model_get_house(b->subtype.house_level)->food_types);

            sentiment += desirability_bonus;
            sentiment += entertainment_bonus;
            sentiment += food_bonus;
            sentiment += games_bonus;

            sentiment += blessing_festival_boost;

            // Change sentiment gradually to the new value
            int sentiment_delta = sentiment - b->sentiment.house_happiness;
            sentiment_delta = calc_bound(sentiment_delta, -MAX_SENTIMENT_CHANGE, MAX_SENTIMENT_CHANGE);
            b->sentiment.house_happiness = calc_bound(b->sentiment.house_happiness + sentiment_delta, 0, 100);
            houses_calculated++;

            total_pop += b->house_population;
            total_houses++;
            total_sentiment += b->sentiment.house_happiness * b->house_population;

            int worst_sentiment = 0;
            b->house_sentiment_message = LOW_MOOD_CAUSE_NONE;

            if (b->sentiment.house_happiness < 80) {
                if (b->house_tax_coverage) {
                    worst_sentiment = sentiment_contribution_taxes;
                    b->house_sentiment_message = LOW_MOOD_CAUSE_HIGH_TAXES;
                }
                if (b->subtype.house_level <= HOUSE_GRAND_INSULA) {
                    if (-sentiment_contribution_unemployment < worst_sentiment) {
                        worst_sentiment = -sentiment_contribution_unemployment;
                        b->house_sentiment_message = LOW_MOOD_CAUSE_NO_JOBS;
                    }
                    if (sentiment_contribution_wages < worst_sentiment) {
                        worst_sentiment = sentiment_contribution_wages;
                        b->house_sentiment_message = LOW_MOOD_CAUSE_LOW_WAGES;
                    }
                    if (house_level_sentiment < worst_sentiment) {
                        worst_sentiment = house_level_sentiment;
                        b->house_sentiment_message = LOW_MOOD_CAUSE_SQUALOR;
                    }
                }

                if (worst_sentiment > -15) {
                    if (entertainment_bonus < SENTIMENT_PER_EXTRA_FOOD ||
                        (entertainment_bonus < food_bonus && entertainment_bonus < desirability_bonus)) {
                        b->house_sentiment_message = SUGGEST_MORE_ENT;
                    } else if (desirability_bonus < max_desirability && desirability_bonus < food_bonus) {
                        b->house_sentiment_message = SUGGEST_MORE_DESIRABILITY;
                    } else if (model_get_house(b->subtype.house_level)->food_types > 0 &&
                        food_bonus < MAX_SENTIMENT_FROM_EXTRA_FOOD && b->data.house.num_foods < 3) {
                        b->house_sentiment_message = SUGGEST_MORE_FOOD;
                    } else {
                        b->house_sentiment_message = LOW_MOOD_CAUSE_NONE;
                    }
                }
            }
        }
    }

    if (total_pop) {
        city_data.sentiment.value = calc_bound(total_sentiment / total_pop, 0, 100);
        average_squalor_penalty = average_squalor_penalty / total_houses;
    } else {
        city_data.sentiment.value = default_sentiment;
    }

    int worst_sentiment = 0;
    city_data.sentiment.low_mood_cause = LOW_MOOD_CAUSE_NONE;

    if (-sentiment_contribution_unemployment < worst_sentiment) {
        worst_sentiment = -sentiment_contribution_unemployment;
        city_data.sentiment.low_mood_cause = LOW_MOOD_CAUSE_NO_JOBS;
    }
    if (sentiment_contribution_taxes < worst_sentiment) {
        worst_sentiment = sentiment_contribution_taxes;
        city_data.sentiment.low_mood_cause = LOW_MOOD_CAUSE_HIGH_TAXES;
    }
    if (sentiment_contribution_wages < worst_sentiment) {
        worst_sentiment = sentiment_contribution_wages;
        city_data.sentiment.low_mood_cause = LOW_MOOD_CAUSE_LOW_WAGES;
    }
    if (average_squalor_penalty < worst_sentiment) {
        city_data.sentiment.low_mood_cause = LOW_MOOD_CAUSE_SQUALOR;
    }

    if (city_data.sentiment.message_delay) {
        city_data.sentiment.message_delay--;
    }

    if (city_data.sentiment.value < 48 && city_data.sentiment.value < city_data.sentiment.previous_value) {
        if (city_data.sentiment.message_delay <= 0) {
            city_data.sentiment.message_delay = 3;
            int cause = city_data.sentiment.low_mood_cause;
            if (!cause) {
                cause = -1;
            }
            if (city_data.sentiment.value < 35) {
                city_message_post(0, MESSAGE_PEOPLE_ANGRY, cause, 0);
            } else if (city_data.sentiment.value < 40) {
                city_message_post(0, MESSAGE_PEOPLE_UNHAPPY, cause, 0);
            } else {
                city_message_post(0, MESSAGE_PEOPLE_DISGRUNTLED, cause, 0);
            }
        }
    }

    city_data.sentiment.previous_value = city_data.sentiment.value;
}
