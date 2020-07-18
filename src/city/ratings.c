#include "ratings.h"

#include "building/building.h"
#include "building/model.h"
#include "city/culture.h"
#include "city/data_private.h"
#include "core/calc.h"
#include "game/time.h"
#include "scenario/criteria.h"
#include "scenario/property.h"

int city_rating_culture(void)
{
    return city_data.ratings.culture;
}

int city_rating_prosperity(void)
{
    return city_data.ratings.prosperity;
}

int city_rating_peace(void)
{
    return city_data.ratings.peace;
}

int city_rating_favor(void)
{
    return city_data.ratings.favor;
}

selected_rating city_rating_selected(void)
{
    return city_data.ratings.selected;
}

void city_rating_select(selected_rating rating)
{
    city_data.ratings.selected = rating;
}

int city_rating_selected_explanation(void)
{
    switch (city_data.ratings.selected) {
        case SELECTED_RATING_CULTURE:
            return city_data.ratings.culture_explanation;
        case SELECTED_RATING_PROSPERITY:
            return city_data.ratings.prosperity_explanation;
        case SELECTED_RATING_PEACE:
            return city_data.ratings.peace_explanation;
        case SELECTED_RATING_FAVOR:
            return city_data.ratings.favor_explanation;
        default:
            return 0;
    }
}

void city_ratings_reduce_prosperity_after_bailout(void)
{
    city_data.ratings.prosperity -= 3;
    if (city_data.ratings.prosperity < 0) {
        city_data.ratings.prosperity = 0;
    }
    city_data.ratings.prosperity_explanation = 8;
}

void city_ratings_peace_building_destroyed(building_type type)
{
    switch (type) {
        case BUILDING_HOUSE_SMALL_TENT:
        case BUILDING_HOUSE_LARGE_TENT:
        case BUILDING_PREFECTURE:
        case BUILDING_ENGINEERS_POST:
        case BUILDING_WELL:
        case BUILDING_FORT:
        case BUILDING_FORT_GROUND:
        case BUILDING_GATEHOUSE:
        case BUILDING_TOWER:
            break;
        default:
            city_data.ratings.peace_destroyed_buildings++;
            break;
    }
    if (city_data.ratings.peace_destroyed_buildings >= 12) {
        city_data.ratings.peace_destroyed_buildings = 12;
    }
}

void city_ratings_peace_record_criminal(void)
{
    city_data.ratings.peace_num_criminals++;
}

void city_ratings_peace_record_rioter(void)
{
    city_data.ratings.peace_num_rioters++;
    city_data.ratings.peace_riot_cause = city_data.sentiment.low_mood_cause;
}

void city_ratings_change_favor(int amount)
{
    city_data.ratings.favor = calc_bound(city_data.ratings.favor + amount, 0, 100);
}

void city_ratings_reset_favor_emperor_change(void)
{
    city_data.ratings.favor = 50;
}

void city_ratings_reduce_favor_missed_request(int penalty)
{
    city_ratings_change_favor(-penalty);
    city_data.ratings.favor_ignored_request_penalty = penalty;
}

void city_ratings_limit_favor(int max_favor)
{
    if (city_data.ratings.favor > max_favor) {
        city_data.ratings.favor = max_favor;
    }
}

static void update_culture_explanation(void)
{
    int min_percentage = 100;
    int reason = 1;
    if (city_data.culture.religion_coverage < min_percentage) {
        min_percentage = city_data.culture.religion_coverage;
        reason = 4;
    }
    int pct_theater = city_culture_coverage_theater();
    if (pct_theater < min_percentage) {
        min_percentage = pct_theater;
        reason = 5;
    }
    int pct_library = city_culture_coverage_library();
    if (pct_library < min_percentage) {
        min_percentage = pct_library;
        reason = 2;
    }
    int pct_school = city_culture_coverage_school();
    if (pct_school < min_percentage) {
        min_percentage = pct_school;
        reason = 1;
    }
    int pct_academy = city_culture_coverage_academy();
    if (pct_academy < min_percentage) {
        reason = 3;
    }
    city_data.ratings.culture_explanation = reason;
}

static int has_made_money(void)
{
    return city_data.finance.last_year.expenses.construction + city_data.finance.treasury >
        city_data.ratings.prosperity_treasury_last_year;
}

static void update_prosperity_explanation(void)
{
    int change = 0;
    int profit = 0;
    // unemployment: -1 for too high, +1 for low
    if (city_data.labor.unemployment_percentage < 5) {
        change += 1;
    } else if (city_data.labor.unemployment_percentage >= 15) {
        change -= 1;
    }
    // losing/earning money: -1 for losing, +5 for profit
    if (has_made_money()) {
        change += 5;
        profit = 1;
    } else {
        change -= 1;
    }
    // food types: +1 for multiple foods
    if (city_data.resource.food_types_eaten >= 2) {
        change += 1;
    }
    // wages: +1 for wages 2+ above Rome, -1 for wages below Rome
    int avg_wage = city_data.finance.wage_rate_paid_last_year / 12;
    if (avg_wage >= city_data.labor.wages_rome + 2) {
        change += 1;
    } else if (avg_wage < city_data.labor.wages_rome) {
        change -= 1;
    }
    // high percentage poor: -1, high percentage rich: +1
    int pct_tents = calc_percentage(city_data.population.people_in_tents_shacks, city_data.population.population);
    if (pct_tents > 30) {
        change -= 1;
    }
    if (calc_percentage(city_data.population.people_in_villas_palaces, city_data.population.population) > 10) {
        change += 1;
    }
    // tribute not paid: -1
    if (city_data.finance.tribute_not_paid_last_year) {
        change -= 1;
    }
    // working hippodrome: +1
    if (city_data.entertainment.hippodrome_shows > 0) {
        change += 1;
    }

    int reason;
    if (city_data.ratings.prosperity <= 0 && game_time_year() == scenario_property_start_year()) {
        reason = 0;
    } else if (city_data.ratings.prosperity >= city_data.ratings.prosperity_max) {
        reason = 1;
    } else if (change > 0) {
        reason = 2;
    } else if (!profit) {
        reason = 3;
    } else if (city_data.labor.unemployment_percentage >= 15) {
        reason = 4;
    } else if (avg_wage < city_data.labor.wages_rome) {
        reason = 5;
    } else if (pct_tents > 30) {
        reason = 6;
    } else if (city_data.finance.tribute_not_paid_last_year) {
        reason = 7;
    } else {
        reason = 9;
    }
    // 8 = for bailout
    city_data.ratings.prosperity_explanation = reason;
}

static void update_peace_explanation(void)
{
    int reason;
    if (city_data.figure.imperial_soldiers) {
        reason = 8; // FIXED: 7+8 interchanged
    } else if (city_data.figure.enemies) {
        reason = 7;
    } else if (city_data.figure.rioters) {
        reason = 6;
    } else {
        if (city_data.ratings.peace < 10) {
            reason = 0;
        } else if (city_data.ratings.peace < 30) {
            reason = 1;
        } else if (city_data.ratings.peace < 60) {
            reason = 2;
        } else if (city_data.ratings.peace < 90) {
            reason = 3;
        } else if (city_data.ratings.peace < 100) {
            reason = 4;
        } else { // >= 100
            reason = 5;
        }
    }
    city_data.ratings.peace_explanation = reason;
}

void city_ratings_update_favor_explanation(void)
{
    city_data.ratings.favor_salary_penalty = 0;
    int salary_delta = city_data.emperor.salary_rank - city_data.emperor.player_rank;
    if (city_data.emperor.player_rank != 0) {
        if (salary_delta > 0) {
            city_data.ratings.favor_salary_penalty = salary_delta + 1;
        }
    } else if (salary_delta > 0) {
        city_data.ratings.favor_salary_penalty = salary_delta;
    }

    if (city_data.ratings.favor_salary_penalty >= 8) {
        city_data.ratings.favor_explanation = 1;
    } else if (city_data.finance.tribute_not_paid_total_years >= 3) {
        city_data.ratings.favor_explanation = 2;
    } else if (city_data.ratings.favor_ignored_request_penalty >= 5) {
        city_data.ratings.favor_explanation = 3;
    } else if (city_data.ratings.favor_salary_penalty >= 5) {
        city_data.ratings.favor_explanation = 4;
    } else if (city_data.finance.tribute_not_paid_total_years >= 2) {
        city_data.ratings.favor_explanation = 5;
    } else if (city_data.ratings.favor_ignored_request_penalty >= 3) {
        city_data.ratings.favor_explanation = 6;
    } else if (city_data.ratings.favor_salary_penalty >= 3) {
        city_data.ratings.favor_explanation = 7;
    } else if (city_data.finance.tribute_not_paid_last_year) {
        city_data.ratings.favor_explanation = 8;
    } else if (city_data.ratings.favor_salary_penalty >= 2) {
        city_data.ratings.favor_explanation = 9;
    } else if (city_data.ratings.favor_milestone_penalty) {
        city_data.ratings.favor_explanation = 10;
    } else if (city_data.ratings.favor_salary_penalty) {
        city_data.ratings.favor_explanation = 11;
    } else if (city_data.ratings.favor_change == 2) { // rising
        city_data.ratings.favor_explanation = 12;
    } else if (city_data.ratings.favor_change == 1) { // the same
        city_data.ratings.favor_explanation = 13;
    } else {
        city_data.ratings.favor_explanation = 0;
    }
}

void city_ratings_update_explanations(void)
{
    update_culture_explanation();
    update_prosperity_explanation();
    update_peace_explanation();
    city_ratings_update_favor_explanation();
}

static void update_culture_rating(void)
{
    city_data.ratings.culture = 0;
    city_data.ratings.culture_explanation = 0;
    if (city_data.population.population <= 0) {
        return;
    }

    int pct_theater = city_culture_coverage_theater();
    if (pct_theater >= 100) {
        city_data.ratings.culture_points.theater = 25;
    } else if (pct_theater > 85) {
        city_data.ratings.culture_points.theater = 18;
    } else if (pct_theater > 70) {
        city_data.ratings.culture_points.theater = 12;
    } else if (pct_theater > 50) {
        city_data.ratings.culture_points.theater = 8;
    } else if (pct_theater > 30) {
        city_data.ratings.culture_points.theater = 3;
    } else {
        city_data.ratings.culture_points.theater = 0;
    }
    city_data.ratings.culture += city_data.ratings.culture_points.theater;

    int pct_religion = city_data.culture.religion_coverage;
    if (pct_religion >= 100) {
        city_data.ratings.culture_points.religion = 30;
    } else if (pct_religion > 85) {
        city_data.ratings.culture_points.religion = 22;
    } else if (pct_religion > 70) {
        city_data.ratings.culture_points.religion = 14;
    } else if (pct_religion > 50) {
        city_data.ratings.culture_points.religion = 9;
    } else if (pct_religion > 30) {
        city_data.ratings.culture_points.religion = 3;
    } else {
        city_data.ratings.culture_points.religion = 0;
    }
    city_data.ratings.culture += city_data.ratings.culture_points.religion;

    int pct_school = city_culture_coverage_school();
    if (pct_school >= 100) {
        city_data.ratings.culture_points.school = 15;
    } else if (pct_school > 85) {
        city_data.ratings.culture_points.school = 10;
    } else if (pct_school > 70) {
        city_data.ratings.culture_points.school = 6;
    } else if (pct_school > 50) {
        city_data.ratings.culture_points.school = 4;
    } else if (pct_school > 30) {
        city_data.ratings.culture_points.school = 1;
    } else {
        city_data.ratings.culture_points.school = 0;
    }
    city_data.ratings.culture += city_data.ratings.culture_points.school;

    int pct_academy = city_culture_coverage_academy();
    if (pct_academy >= 100) {
        city_data.ratings.culture_points.academy = 10;
    } else if (pct_academy > 85) {
        city_data.ratings.culture_points.academy = 7;
    } else if (pct_academy > 70) {
        city_data.ratings.culture_points.academy = 4;
    } else if (pct_academy > 50) {
        city_data.ratings.culture_points.academy = 2;
    } else if (pct_academy > 30) {
        city_data.ratings.culture_points.academy = 1;
    } else {
        city_data.ratings.culture_points.academy = 0;
    }
    city_data.ratings.culture += city_data.ratings.culture_points.academy;

    int pct_library = city_culture_coverage_library();
    if (pct_library >= 100) {
        city_data.ratings.culture_points.library = 20;
    } else if (pct_library > 85) {
        city_data.ratings.culture_points.library = 14;
    } else if (pct_library > 70) {
        city_data.ratings.culture_points.library = 8;
    } else if (pct_library > 50) {
        city_data.ratings.culture_points.library = 4;
    } else if (pct_library > 30) {
        city_data.ratings.culture_points.library = 2;
    } else {
        city_data.ratings.culture_points.library = 0;
    }
    city_data.ratings.culture += city_data.ratings.culture_points.library;

    city_data.ratings.culture = calc_bound(city_data.ratings.culture, 0, 100);
    update_culture_explanation();
}

static void update_prosperity_rating(void)
{
    int change = 0;
    // unemployment: -1 for too high, +1 for low
    if (city_data.labor.unemployment_percentage < 5) {
        change += 1;
    } else if (city_data.labor.unemployment_percentage >= 15) {
        change -= 1;
    }
    // losing/earning money: -1 for losing, +5 for profit
    if (has_made_money()) {
        change += 5;
    } else {
        change -= 1;
    }
    city_data.ratings.prosperity_treasury_last_year = city_data.finance.treasury;
    // food types: +1 for multiple foods
    if (city_data.resource.food_types_eaten >= 2) {
        change += 1;
    }
    // wages: +1 for wages 2+ above Rome, -1 for wages below Rome
    int avg_wage = city_data.finance.wage_rate_paid_last_year / 12;
    if (avg_wage >= city_data.labor.wages_rome + 2) {
        change += 1;
    } else if (avg_wage < city_data.labor.wages_rome) {
        change -= 1;
    }
    // high percentage poor: -1, high percentage rich: +1
    if (calc_percentage(city_data.population.people_in_tents_shacks, city_data.population.population) > 30) {
        change -= 1;
    }
    if (calc_percentage(city_data.population.people_in_villas_palaces, city_data.population.population) > 10) {
        change += 1;
    }
    // tribute not paid: -1
    if (city_data.finance.tribute_not_paid_last_year) {
        change -= 1;
    }
    // working hippodrome: +1
    if (city_data.entertainment.hippodrome_shows > 0) {
        change += 1;
    }
    city_data.ratings.prosperity += change;
    if (city_data.ratings.prosperity > city_data.ratings.prosperity_max) {
        city_data.ratings.prosperity = city_data.ratings.prosperity_max;
    }
    city_data.ratings.prosperity = calc_bound(city_data.ratings.prosperity, 0, 100);

    update_prosperity_explanation();
}

static void calculate_max_prosperity(void)
{
    int points = 0;
    int houses = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state && b->house_size) {
            points += model_get_house(b->subtype.house_level)->prosperity;
            houses++;
        }
    }
    if (houses > 0) {
        city_data.ratings.prosperity_max = points / houses;
    } else {
        city_data.ratings.prosperity_max = 0;
    }
}

static void update_peace_rating(void)
{
    int change = 0;
    if (city_data.ratings.peace_years_of_peace < 2) {
        change += 2;
    } else {
        change += 5;
    }
    if (city_data.ratings.peace_num_criminals) {
        change -= 1;
    }
    if (city_data.ratings.peace_num_rioters) {
        change -= 5;
    }
    if (city_data.ratings.peace_destroyed_buildings) {
        change -= city_data.ratings.peace_destroyed_buildings;
    }
    if (city_data.ratings.peace_num_rioters || city_data.ratings.peace_destroyed_buildings) {
        city_data.ratings.peace_years_of_peace = 0;
    } else {
        city_data.ratings.peace_years_of_peace += 1;
    }
    city_data.ratings.peace_num_criminals = 0;
    city_data.ratings.peace_num_rioters = 0;
    city_data.ratings.peace_destroyed_buildings = 0;

    city_data.ratings.peace = calc_bound(city_data.ratings.peace + change, 0, 100);
    update_peace_explanation();
}

static void update_favor_rating(int is_yearly_update)
{
    if (scenario_is_open_play()) {
        city_data.ratings.favor = 50;
        return;
    }
    city_data.emperor.months_since_gift++;
    if (city_data.emperor.months_since_gift >= 12) {
        city_data.emperor.gift_overdose_penalty = 0;
    }
    if (is_yearly_update) {
        city_data.ratings.favor_salary_penalty = 0;
        city_data.ratings.favor_milestone_penalty = 0;
        city_data.ratings.favor_ignored_request_penalty = 0;
        if (!scenario_is_tutorial_1() && !scenario_is_tutorial_2()) {
            city_data.ratings.favor -= 2;
        }
        // tribute penalty
        if (city_data.finance.tribute_not_paid_last_year) {
            if (city_data.finance.tribute_not_paid_total_years <= 1) {
                city_data.ratings.favor -= 3;
            } else if (city_data.finance.tribute_not_paid_total_years <= 2) {
                city_data.ratings.favor -= 5;
            } else {
                city_data.ratings.favor -= 8;
            }
        }
        // salary
        int salary_delta = city_data.emperor.salary_rank - city_data.emperor.player_rank;
        if (city_data.emperor.player_rank != 0) {
            if (salary_delta > 0) {
                // salary too high
                city_data.ratings.favor -= salary_delta;
                city_data.ratings.favor_salary_penalty = salary_delta + 1;
            } else if (salary_delta < 0) {
                // salary lower than rank
                city_data.ratings.favor += 1;
            }
        } else if (salary_delta > 0) {
            city_data.ratings.favor -= salary_delta;
            city_data.ratings.favor_salary_penalty = salary_delta;
        }
        // milestone
        int milestone_pct;
        if (scenario_criteria_milestone_year(25) == game_time_year()) {
            milestone_pct = 25;
        } else if (scenario_criteria_milestone_year(50) == game_time_year()) {
            milestone_pct = 50;
        } else if (scenario_criteria_milestone_year(75) == game_time_year()) {
            milestone_pct = 75;
        } else {
            milestone_pct = 0;
        }
        if (milestone_pct) {
            int bonus = 1;
            if (scenario_criteria_culture_enabled() &&
                city_data.ratings.culture < calc_adjust_with_percentage(
                    scenario_criteria_culture(), milestone_pct)) {
                bonus = 0;
            }
            if (scenario_criteria_prosperity_enabled() &&
                city_data.ratings.prosperity < calc_adjust_with_percentage(
                    scenario_criteria_prosperity(), milestone_pct)) {
                bonus = 0;
            }
            if (scenario_criteria_peace_enabled() &&
                city_data.ratings.peace < calc_adjust_with_percentage(
                    scenario_criteria_peace(), milestone_pct)) {
                bonus = 0;
            }
            if (scenario_criteria_favor_enabled() &&
                city_data.ratings.favor < calc_adjust_with_percentage(
                    scenario_criteria_favor(), milestone_pct)) {
                bonus = 0;
            }
            if (scenario_criteria_population_enabled() &&
                city_data.population.population < calc_adjust_with_percentage(
                    scenario_criteria_population(), milestone_pct)) {
                bonus = 0;
            }
            if (bonus) {
                city_data.ratings.favor += 5;
            } else {
                city_data.ratings.favor -= 2;
                city_data.ratings.favor_milestone_penalty = 2;
            }
        }

        if (city_data.ratings.favor < city_data.ratings.favor_last_year) {
            city_data.ratings.favor_change = 0;
        } else if (city_data.ratings.favor == city_data.ratings.favor_last_year) {
            city_data.ratings.favor_change = 1;
        } else {
            city_data.ratings.favor_change = 2;
        }
        city_data.ratings.favor_last_year = city_data.ratings.favor;
    }
    city_data.ratings.favor = calc_bound(city_data.ratings.favor, 0, 100);
    city_ratings_update_favor_explanation();
}

void city_ratings_update(int is_yearly_update)
{
    update_culture_rating();
    update_favor_rating(is_yearly_update);
    calculate_max_prosperity();
    if (is_yearly_update) {
        update_prosperity_rating();
        update_peace_rating();
    }
}

int city_ratings_prosperity_max(void) {
    return city_data.ratings.prosperity_max;
}
