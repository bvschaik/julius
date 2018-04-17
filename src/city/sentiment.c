#include "sentiment.h"

#include "building/building.h"
#include "building/model.h"
#include "city/constants.h"
#include "city/data_private.h"
#include "city/message.h"
#include "city/population.h"
#include "core/calc.h"
#include "game/difficulty.h"
#include "game/tutorial.h"

#include "Data/CityInfo.h"

static const int SENTIMENT_PER_TAX_RATE[26] = {
    3, 2, 2, 2, 1, 1, 1, 0, 0, -1,
    -2, -2, -3, -3, -3, -5, -5, -5, -5, -6,
    -6, -6, -6, -6, -6, -6
};

int city_sentiment()
{
    return city_data.sentiment.value;
}

void city_sentiment_change_happiness(int amount)
{
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->houseSize) {
            b->sentiment.houseHappiness += amount;
            b->sentiment.houseHappiness = calc_bound(b->sentiment.houseHappiness, 0, 100);
        }
    }
}

void city_sentiment_set_max_happiness(int max)
{
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->houseSize) {
            if (b->sentiment.houseHappiness > max) {
                b->sentiment.houseHappiness = max;
            }
            b->sentiment.houseHappiness = calc_bound(b->sentiment.houseHappiness, 0, 100);
        }
    }
}


static int get_sentiment_penalty_for_tent_dwellers()
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

static int get_sentiment_contribution_wages()
{
    city_data.sentiment.wages = Data_CityInfo.wages;
    int contribution = 0;
    int wage_diff = Data_CityInfo.wages - Data_CityInfo.wagesRome;
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

static int get_sentiment_contribution_employment()
{
    int unemployment = city_data.sentiment.unemployment = Data_CityInfo.unemploymentPercentage;
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

void city_sentiment_update()
{
    city_population_check_consistency();

    int sentiment_contribution_taxes = SENTIMENT_PER_TAX_RATE[city_data.finance.tax_percentage];
    int sentiment_contribution_wages = get_sentiment_contribution_wages();
    int sentiment_contribution_employment = get_sentiment_contribution_employment();
    int sentiment_penalty_tents = get_sentiment_penalty_for_tent_dwellers();

    int houses_calculated = 0;
    int houses_needing_food = 0;
    int total_sentiment_contribution_food = 0;
    int total_sentiment_penalty_tents = 0;
    int default_sentiment = difficulty_sentiment();
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE || !b->houseSize) {
            continue;
        }
        if (!b->housePopulation) {
            b->sentiment.houseHappiness = 10 + default_sentiment;
            continue;
        }
        if (city_data.population.population < 300) {
            // small town has no complaints
            sentiment_contribution_employment = 0;
            sentiment_contribution_taxes = 0;
            sentiment_contribution_wages = 0;

            b->sentiment.houseHappiness = default_sentiment;
            if (city_data.population.population < 200) {
                b->sentiment.houseHappiness += 10;
            }
            continue;
        }
        // population >= 300
        houses_calculated++;
        int sentiment_contribution_food = 0;
        int sentiment_contribution_tents = 0;
        if (!model_get_house(b->subtype.houseLevel)->food_types) {
            // tents
            b->houseDaysWithoutFood = 0;
            sentiment_contribution_tents = sentiment_penalty_tents;
            total_sentiment_penalty_tents += sentiment_penalty_tents;
        } else {
            // shack+
            houses_needing_food++;
            if (b->data.house.numFoods >= 2) {
                sentiment_contribution_food = 2;
                total_sentiment_contribution_food += 2;
                b->houseDaysWithoutFood = 0;
            } else if (b->data.house.numFoods >= 1) {
                sentiment_contribution_food = 1;
                total_sentiment_contribution_food += 1;
                b->houseDaysWithoutFood = 0;
            } else {
                // needs food but has no food
                if (b->houseDaysWithoutFood < 3) {
                    b->houseDaysWithoutFood++;
                }
                sentiment_contribution_food = -b->houseDaysWithoutFood;
                total_sentiment_contribution_food -= b->houseDaysWithoutFood;
            }
        }
        b->sentiment.houseHappiness += sentiment_contribution_taxes;
        b->sentiment.houseHappiness += sentiment_contribution_wages;
        b->sentiment.houseHappiness += sentiment_contribution_employment;
        b->sentiment.houseHappiness += sentiment_contribution_food;
        b->sentiment.houseHappiness += sentiment_contribution_tents;
        b->sentiment.houseHappiness = calc_bound(b->sentiment.houseHappiness, 0, 100);
    }

    int sentiment_contribution_food = 0;
    int sentiment_contribution_tents = 0;
    if (houses_needing_food) {
        sentiment_contribution_food = total_sentiment_contribution_food / houses_needing_food;
    }
    if (houses_calculated) {
        sentiment_contribution_tents = total_sentiment_penalty_tents / houses_calculated;
    }

    int total_sentiment = 0;
    int total_houses = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->houseSize && b->housePopulation) {
            total_houses++;
            total_sentiment += b->sentiment.houseHappiness;
        }
    }
    if (total_houses) {
        city_data.sentiment.value = total_sentiment / total_houses;
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
            } else if (city_data.sentiment.value < 40) {
                city_message_post(0, MESSAGE_PEOPLE_UNHAPPY, 0, 0);
            } else {
                city_message_post(0, MESSAGE_PEOPLE_DISGRUNTLED, 0, 0);
            }
        }
    }

    int worst_sentiment = 0;
    Data_CityInfo.populationEmigrationCause = EMIGRATION_CAUSE_NONE;
    if (sentiment_contribution_food < worst_sentiment) {
        worst_sentiment = sentiment_contribution_food;
        Data_CityInfo.populationEmigrationCause = EMIGRATION_CAUSE_NO_FOOD;
    }
    if (sentiment_contribution_employment < worst_sentiment) {
        worst_sentiment = sentiment_contribution_employment;
        Data_CityInfo.populationEmigrationCause = EMIGRATION_CAUSE_NO_JOBS;
    }
    if (sentiment_contribution_taxes < worst_sentiment) {
        worst_sentiment = sentiment_contribution_taxes;
        Data_CityInfo.populationEmigrationCause = EMIGRATION_CAUSE_HIGH_TAXES;
    }
    if (sentiment_contribution_wages < worst_sentiment) {
        worst_sentiment = sentiment_contribution_wages;
        Data_CityInfo.populationEmigrationCause = EMIGRATION_CAUSE_LOW_WAGES;
    }
    if (sentiment_contribution_tents < worst_sentiment) {
        Data_CityInfo.populationEmigrationCause = EMIGRATION_CAUSE_MANY_TENTS;
    }
    city_data.sentiment.previous_value = city_data.sentiment.value;
}
