#include "sentiment.h"

#include "building/building.h"
#include "building/model.h"
#include "city/constants.h"
#include "city/data_private.h"
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
#define DECENT_HOUSING_LEVEL HOUSE_LARGE_CASA
#define UNEMPLOYMENT_THRESHHOLD 5
#define UNEMPLOYMENT_MODIFIER 3
#define TAX_MODIFIER_POSITIVE_CURVE 0.6
#define TAX_MODIFIER_NEGATIVE_CURVE 1.6
#define WAGE_NEGATIVE_MODIFIER 3
#define WAGE_POSITIVE_MODIFIER 2
#define BASE_TAX_RATE 6
#define SQUALOR_MULTIPLIER 2
#define SENTIMENT_PER_ENTERTAINMENT 1
#define SENTIMENT_PER_EXTRA_FOOD 12


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
    for (int i = 1; i < building_count(); i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->house_size) {
            b->sentiment.house_happiness = calc_bound(b->sentiment.house_happiness + amount, 0, 100);
        }
    }
}

void city_sentiment_set_max_happiness(int max)
{
    for (int i = 1; i < building_count(); i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->house_size) {
            if (b->sentiment.house_happiness > max) {
                b->sentiment.house_happiness = max;
            }
            b->sentiment.house_happiness = calc_bound(b->sentiment.house_happiness, 0, 100);
        }
    }
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

int city_sentiment_blessing_festival_sentiment_boost(void)
{
    return city_data.sentiment.blessing_festival_sentiment_boost;
}

void decrement_blessing_festival_boost(void) {
    double boost = city_data.sentiment.blessing_festival_sentiment_boost;
    city_data.sentiment.blessing_festival_sentiment_boost = (int)floor(boost * 0.84);
}

static int get_sentiment_penalty_for_tent_dwellers(void)
{
    // alternate the penalty for every update
    if (!city_data.sentiment.include_tents) {
        city_data.sentiment.include_tents = 1;
        return 0;
    }
    city_data.sentiment.include_tents = 0;

    int penalty;
    int pct_tents = calc_percentage(city_data.population.people_in_tents, city_data.population.population);
    if (city_data.population.people_in_villas_palaces > 0) {
        if (pct_tents >= 57) {
            penalty = 0;
        } else if (pct_tents >= 40) {
            penalty = -3;
        } else if (pct_tents >= 26) {
            penalty = -4;
        } else if (pct_tents >= 10) {
            penalty = -5;
        } else {
            penalty = -6;
        }
    } else if (city_data.population.people_in_large_insula_and_above > 0) {
        if (pct_tents >= 57) {
            penalty = 0;
        } else if (pct_tents >= 40) {
            penalty = -2;
        } else if (pct_tents >= 26) {
            penalty = -3;
        } else if (pct_tents >= 10) {
            penalty = -4;
        } else {
            penalty = -5;
        }
    } else {
        if (pct_tents >= 40) {
            penalty = 0;
        } else if (pct_tents >= 26) {
            penalty = -1;
        } else if (pct_tents >= 10) {
            penalty = -2;
        } else {
            penalty = -3;
        }
    }
    return penalty;
}

static int get_sentiment_contribution_wages(void)
{
    city_data.sentiment.wages = city_data.labor.wages;
    int contribution = 0;
    int wage_diff = city_data.labor.wages - city_data.labor.wages_rome;
    if (wage_diff < 0) {
        contribution = wage_diff / 2;
        if (!contribution) {
            contribution = -1;
        }
    } else if (wage_diff > 7) {
        contribution = 4;
    } else if (wage_diff > 4) {
        contribution = 3;
    } else if (wage_diff > 1) {
        contribution = 2;
    } else if (wage_diff > 0) {
        contribution = 1;
    }
    return contribution;
}

static int get_sentiment_contribution_employment(void)
{
    int unemployment = city_data.sentiment.unemployment = city_data.labor.unemployment_percentage;
    if (unemployment > 25) {
        return -3;
    } else if (unemployment > 17) {
        return -2;
    } else if (unemployment > 10) {
        return -1;
    } else if (unemployment > 4) {
        return 0;
    } else {
        return 1;
    }
}

static int calc_economy_modifier_wage(void) {
    int wage_differential = (city_data.labor.wages - city_data.labor.wages_rome);
    return (wage_differential > 0 ? wage_differential * WAGE_POSITIVE_MODIFIER : wage_differential * WAGE_NEGATIVE_MODIFIER);
}

static int calc_economy_modifier_unemployment(void) {
    int unemployment_penalty = 0;
    if (city_data.labor.unemployment_percentage > UNEMPLOYMENT_THRESHHOLD) {
        unemployment_penalty = ((city_data.labor.unemployment_percentage - UNEMPLOYMENT_THRESHHOLD) / UNEMPLOYMENT_MODIFIER) * -1;
    }
    return unemployment_penalty;
}

static int calc_economy_modifier_tax(void) {
    double tax_differential = (double)city_data.finance.tax_percentage - BASE_TAX_RATE;
    double result = 0.0;
    if (tax_differential > 0.0) {
        result = (pow(tax_differential, 2.0) * (TAX_MODIFIER_NEGATIVE_CURVE)) * -1;
        return (int)floor(result);
    }
    else if (tax_differential < 0.0) {
        result = (pow(tax_differential, 2.0) * (TAX_MODIFIER_POSITIVE_CURVE));
        return (int)floor(result);
    }
    return 0;
}

static int calc_average_housing_level(void) {
    int avg = 0;
    int num_houses = 0;

    for (int i = 1; i < building_count(); i++) {
        building* b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->house_size && b->house_population) {
            avg += b->subtype.house_level;
            if (b->subtype.house_level >= HOUSE_SMALL_VILLA) {
                // villas count twice
                avg += b->subtype.house_level;
                if (b->subtype.house_level >= HOUSE_SMALL_PALACE) {
                    // palaces count thrice
                    avg += b->subtype.house_level;
                }
            }
            num_houses++;
        }
    }        

    if (num_houses) {
        avg = avg / num_houses;
    }

    return avg * SQUALOR_MULTIPLIER;
}

static int calc_scaling_housing_penalty(int house_level, int average) {
    int penalty = (average - (house_level * 2)) * 2;
    return (penalty > 0) ? penalty * -1 : 0;
}

static int calc_scaling_housing_bonus(int house_level, int average) {
    int bonus = (house_level - average) * 2;
    return (bonus > 0) ? bonus : 0;
}

static int calculate_extra_ent_bonus(int entertainment, int required) {
    int extra = (entertainment - required) * SENTIMENT_PER_ENTERTAINMENT;
    if (extra > 0) {
        return calc_bound(extra, 0, MAX_SENTIMENT_FROM_EXTRA_ENTERTAINMENT);
    }
    return 0;
}

static int calculate_extra_food_bonus(int types, int required) {
    int extra = (types - required * SENTIMENT_PER_EXTRA_FOOD);
    if (extra > 0) {
        return calc_bound(extra, 0, MAX_SENTIMENT_FROM_EXTRA_FOOD);
    }
    return 0;
}

void city_sentiment_update(void)
{
    city_population_check_consistency();

    int default_sentiment = difficulty_sentiment();
    int houses_calculated = 0;
    int sentiment_contribution_taxes = calc_economy_modifier_tax();
    int sentiment_contribution_wages = calc_economy_modifier_wage();
    int sentiment_contribution_employment = calc_economy_modifier_unemployment();
    int average_housing_level = calc_average_housing_level();
    int blessing_festival_sentiment_boost = city_data.sentiment.blessing_festival_sentiment_boost;
    int average_squalor_penalty = 0;

    for (int i = 1; i < building_count(); i++) {
        building* b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE || !b->house_size) {
            continue;
        }
        if (!b->house_population) {
            b->sentiment.house_happiness = 10 + default_sentiment;
            continue;
        }
        if (city_data.population.population < 300) {
            // small town has no complaints
            b->sentiment.house_happiness = default_sentiment;
            if (default_sentiment < 50) {
                // Fix very hard immigration bug: give a boost for Very Hard difficulty so that
                // immigration is not halted simply because you are between pop 200 and 300
                b->sentiment.house_happiness += 50 - default_sentiment;
            }
            continue;
        }

        int sentiment = default_sentiment;
        int squalor_penalty = 0;
        int ent_bonus = 0;
        int food_bonus = 0;
        
        sentiment += sentiment_contribution_taxes;
        
        if (b->subtype.house_level <= HOUSE_GRAND_INSULA) {
            sentiment += sentiment_contribution_wages;
        }
        if (b->subtype.house_level < DECENT_HOUSING_LEVEL) {
            squalor_penalty = calc_scaling_housing_penalty(b->subtype.house_level, average_housing_level);
            sentiment += squalor_penalty;
            average_squalor_penalty += squalor_penalty * -1;

        }
        sentiment += calc_scaling_housing_bonus(b->subtype.house_level, average_housing_level);
        ent_bonus += calculate_extra_ent_bonus(b->data.house.entertainment, model_get_house(b->subtype.house_level)->entertainment);
        food_bonus += calculate_extra_food_bonus(b->data.house.num_foods, model_get_house(b->subtype.house_level)->food_types);

        sentiment += ent_bonus;
        sentiment += food_bonus;

        sentiment += blessing_festival_sentiment_boost;
        

        b->sentiment.house_happiness = calc_bound((sentiment + b->sentiment.house_happiness) / 2, 0, 100); // sentiment changes to an average of current sentiment and new calculated value
        houses_calculated++;

        int worst_sentiment = 0;
        city_data.sentiment.low_mood_cause = LOW_MOOD_CAUSE_NONE;

        if (sentiment_contribution_employment < worst_sentiment) {
            worst_sentiment = sentiment_contribution_employment;
            b->house_sentiment_message = LOW_MOOD_CAUSE_NO_JOBS;
        }
        if (sentiment_contribution_taxes < worst_sentiment) {
            worst_sentiment = sentiment_contribution_taxes;
            b->house_sentiment_message = LOW_MOOD_CAUSE_HIGH_TAXES;
        }
        if (sentiment_contribution_wages < worst_sentiment) {
            worst_sentiment = sentiment_contribution_wages;
            b->house_sentiment_message = LOW_MOOD_CAUSE_LOW_WAGES;
        }
        if (squalor_penalty < worst_sentiment) {
            worst_sentiment = squalor_penalty;
            b->house_sentiment_message = LOW_MOOD_CAUSE_SQUALOR;
        }

        if (worst_sentiment > -15) {
            if (sentiment < 80) {
                if (ent_bonus < SENTIMENT_PER_EXTRA_FOOD || ent_bonus < food_bonus) {
                    b->house_sentiment_message = SUGGEST_MORE_ENT;
                }
                else if (food_bonus < MAX_SENTIMENT_FROM_EXTRA_FOOD && b->data.house.num_foods < 3) {
                    b->house_sentiment_message = SUGGEST_MORE_FOOD;
                }
                else {
                    b->house_sentiment_message = LOW_MOOD_CAUSE_NONE;
                }
            }
        }
    }

    int total_sentiment = 0;
    int total_pop = 0;
    int total_houses = 0;
    for (int i = 1; i < building_count(); i++) {
        building* b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->house_size && b->house_population) {
            total_pop += b->house_population;
            total_houses++;
            total_sentiment += b->sentiment.house_happiness * b->house_population;
        }
    }
    if (total_pop) {
        city_data.sentiment.value = calc_bound(total_sentiment / total_pop, 0, 100);
        average_squalor_penalty = average_squalor_penalty / total_houses;
    } else {
        city_data.sentiment.value = 60;
    }

    if (city_data.sentiment.message_delay) {
        city_data.sentiment.message_delay--;
    }

    if (city_data.sentiment.value < 48 && city_data.sentiment.value < city_data.sentiment.previous_value) {
        if (city_data.sentiment.message_delay <= 0) {
            city_data.sentiment.message_delay = 3;
            if (city_data.sentiment.value < 35) {
                city_message_post(0, MESSAGE_PEOPLE_ANGRY, 0, 0);
            }
            else if (city_data.sentiment.value < 40) {
                city_message_post(0, MESSAGE_PEOPLE_UNHAPPY, 0, 0);
            }
            else {
                city_message_post(0, MESSAGE_PEOPLE_DISGRUNTLED, 0, 0);
            }
        }
    }

    int worst_sentiment = 0;
    city_data.sentiment.low_mood_cause = LOW_MOOD_CAUSE_NONE;

    if (sentiment_contribution_employment < worst_sentiment) {
        worst_sentiment = sentiment_contribution_employment;
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
    city_data.sentiment.previous_value = city_data.sentiment.value;
}


