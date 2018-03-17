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
    if (!Data_CityInfo.populationSentimentIncludeTents) {
        Data_CityInfo.populationSentimentIncludeTents = 1;
        return 0;
    }
    Data_CityInfo.populationSentimentIncludeTents = 0;

    int penalty;
    int pct_tents = calc_percentage(Data_CityInfo.populationPeopleInTents, Data_CityInfo.population);
    if (Data_CityInfo.populationPeopleInVillasPalaces > 0) {
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
    } else if (Data_CityInfo.populationPeopleInLargeInsulaAndAbove > 0) {
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
    Data_CityInfo.populationSentimentWages = Data_CityInfo.wages;
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
    Data_CityInfo.populationSentimentUnemployment = Data_CityInfo.unemploymentPercentage;
    if (Data_CityInfo.unemploymentPercentage > 25) {
        return -3;
    } else if (Data_CityInfo.unemploymentPercentage > 17) {
        return -2;
    } else if (Data_CityInfo.unemploymentPercentage > 10) {
        return -1;
    } else if (Data_CityInfo.unemploymentPercentage > 4) {
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
        if (Data_CityInfo.population < 300) {
            // small town has no complaints
            sentiment_contribution_employment = 0;
            sentiment_contribution_taxes = 0;
            sentiment_contribution_wages = 0;

            b->sentiment.houseHappiness = default_sentiment;
            if (Data_CityInfo.population < 200) {
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
        Data_CityInfo.citySentiment = total_sentiment / total_houses;
    } else {
        Data_CityInfo.citySentiment = 60;
    }
    if (Data_CityInfo.citySentimentChangeMessageDelay) {
        Data_CityInfo.citySentimentChangeMessageDelay--;
    }
    if (Data_CityInfo.citySentiment < 48 && Data_CityInfo.citySentiment < Data_CityInfo.citySentimentLastTime) {
        if (Data_CityInfo.citySentimentChangeMessageDelay <= 0) {
            Data_CityInfo.citySentimentChangeMessageDelay = 3;
            if (Data_CityInfo.citySentiment < 35) {
                city_message_post(0, MESSAGE_PEOPLE_ANGRY, 0, 0);
            } else if (Data_CityInfo.citySentiment < 40) {
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
    Data_CityInfo.citySentimentLastTime = Data_CityInfo.citySentiment;
}

void city_sentiment_update_migration_status()
{
    if (Data_CityInfo.citySentiment > 70) {
        Data_CityInfo.populationMigrationPercentage = 100;
    } else if (Data_CityInfo.citySentiment > 60) {
        Data_CityInfo.populationMigrationPercentage = 75;
    } else if (Data_CityInfo.citySentiment >= 50) {
        Data_CityInfo.populationMigrationPercentage = 50;
    } else if (Data_CityInfo.citySentiment > 40) {
        Data_CityInfo.populationMigrationPercentage = 0;
    } else if (Data_CityInfo.citySentiment > 30) {
        Data_CityInfo.populationMigrationPercentage = -10;
    } else if (Data_CityInfo.citySentiment > 20) {
        Data_CityInfo.populationMigrationPercentage = -25;
    } else {
        Data_CityInfo.populationMigrationPercentage = -50;
    }

    Data_CityInfo.populationImmigrationAmountPerBatch = 0;
    Data_CityInfo.populationEmigrationAmountPerBatch = 0;

    int population_cap = tutorial_get_population_cap(200000);
    if (Data_CityInfo.population >= population_cap) {
        Data_CityInfo.populationMigrationPercentage = 0;
        return;
    }
    // war scares immigrants away
    if (Data_CityInfo.numEnemiesInCity + Data_CityInfo.numImperialSoldiersInCity > 3 &&
        Data_CityInfo.populationMigrationPercentage > 0) {
        Data_CityInfo.populationMigrationPercentage = 0;
        return;
    }
    if (Data_CityInfo.populationMigrationPercentage > 0) {
        // immigration
        if (Data_CityInfo.populationEmigrationDuration) {
            Data_CityInfo.populationEmigrationDuration--;
        } else {
            Data_CityInfo.populationImmigrationAmountPerBatch =
                calc_adjust_with_percentage(12, Data_CityInfo.populationMigrationPercentage);
            Data_CityInfo.populationImmigrationDuration = 2;
        }
    } else if (Data_CityInfo.populationMigrationPercentage < 0) {
        // emigration
        if (Data_CityInfo.populationImmigrationDuration) {
            Data_CityInfo.populationImmigrationDuration--;
        } else if (Data_CityInfo.population > 100) {
            Data_CityInfo.populationEmigrationAmountPerBatch =
                calc_adjust_with_percentage(12, -Data_CityInfo.populationMigrationPercentage);
            Data_CityInfo.populationEmigrationDuration = 2;
        }
    }
}
