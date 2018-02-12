#include "ratings.h"

#include "building/building.h"
#include "building/model.h"
#include "city/culture.h"
#include "core/calc.h"
#include "game/time.h"
#include "scenario/criteria.h"
#include "scenario/property.h"

#include "Data/CityInfo.h"

void city_ratings_reduce_prosperity_after_bailout()
{
    Data_CityInfo.ratingProsperity -= 3;
    if (Data_CityInfo.ratingProsperity < 0) {
        Data_CityInfo.ratingProsperity = 0;
    }
    Data_CityInfo.ratingAdvisorExplanationProsperity = 8;
}

void city_ratings_change_favor(int amount)
{
    Data_CityInfo.ratingFavor = calc_bound(Data_CityInfo.ratingFavor + amount, 0, 100);
}

void city_ratings_limit_favor(int max_favor)
{
    if (Data_CityInfo.ratingFavor > max_favor) {
        Data_CityInfo.ratingFavor = max_favor;
    }
}

static void update_culture_explanation()
{
    int min_percentage = 100;
    int reason = 1;
    if (Data_CityInfo.cultureCoverageReligion < min_percentage) {
        min_percentage = Data_CityInfo.cultureCoverageReligion;
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
    Data_CityInfo.ratingAdvisorExplanationCulture = reason;
}

static void update_prosperity_explanation()
{
    int change = 0;
    int profit = 0;
    // unemployment: -1 for too high, +1 for low
    if (Data_CityInfo.unemploymentPercentage < 5) {
        change += 1;
    } else if (Data_CityInfo.unemploymentPercentage >= 15) {
        change -= 1;
    }
    // losing/earning money: -1 for losing, +5 for profit
    if (Data_CityInfo.financeConstructionLastYear + Data_CityInfo.treasury <=
        Data_CityInfo.ratingProsperityTreasuryLastYear) {
        change -= 1;
    } else {
        change += 5;
        profit = 1;
    }
    // food types: +1 for multiple foods
    if (Data_CityInfo.foodInfoFoodTypesEaten >= 2) {
        change += 1;
    }
    // wages: +1 for wages 2+ above Rome, -1 for wages below Rome
    int avg_wage = Data_CityInfo.wageRatePaidLastYear / 12;
    if (avg_wage >= Data_CityInfo.wagesRome + 2) {
        change += 1;
    } else if (avg_wage < Data_CityInfo.wagesRome) {
        change -= 1;
    }
    // high percentage poor: -1, high percentage rich: +1
    int pct_tents = calc_percentage(Data_CityInfo.populationPeopleInTentsShacks, Data_CityInfo.population);
    if (pct_tents > 30) {
        change -= 1;
    }
    if (calc_percentage(Data_CityInfo.populationPeopleInVillasPalaces, Data_CityInfo.population) > 10) {
        change += 1;
    }
    // tribute not paid: -1
    if (Data_CityInfo.tributeNotPaidLastYear) {
        change -= 1;
    }
    // working hippodrome: +1
    if (Data_CityInfo.entertainmentHippodromeShows > 0) {
        change += 1;
    }

    int reason;
    if (Data_CityInfo.ratingProsperity <= 0 && game_time_year() == scenario_property_start_year()) {
        reason = 0;
    } else if (Data_CityInfo.ratingProsperity >= Data_CityInfo.ratingProsperityMax) {
        reason = 1;
    } else if (change > 0) {
        reason = 2;
    } else if (!profit) {
        reason = 3;
    } else if (Data_CityInfo.unemploymentPercentage >= 15) {
        reason = 4;
    } else if (avg_wage < Data_CityInfo.wagesRome) {
        reason = 5;
    } else if (pct_tents > 30) {
        reason = 6;
    } else if (Data_CityInfo.tributeNotPaidLastYear) {
        reason = 7;
    } else {
        reason = 9;
    }
    // 8 = for bailout
    Data_CityInfo.ratingAdvisorExplanationProsperity = reason;
}

static void update_peace_explanation()
{
    int reason;
    if (Data_CityInfo.numImperialSoldiersInCity) {
        reason = 8; // FIXED: 7+8 interchanged
    } else if (Data_CityInfo.numEnemiesInCity) {
        reason = 7;
    } else if (Data_CityInfo.numRiotersInCity) {
        reason = 6;
    } else {
        if (Data_CityInfo.ratingPeace < 10) {
            reason = 0;
        } else if (Data_CityInfo.ratingPeace < 30) {
            reason = 1;
        } else if (Data_CityInfo.ratingPeace < 60) {
            reason = 2;
        } else if (Data_CityInfo.ratingPeace < 90) {
            reason = 3;
        } else if (Data_CityInfo.ratingPeace < 100) {
            reason = 4;
        } else { // >= 100
            reason = 5;
        }
    }
    Data_CityInfo.ratingAdvisorExplanationPeace = reason;
}

void city_ratings_update_favor_explanation()
{
    Data_CityInfo.ratingFavorSalaryPenalty = 0;
    int salary_delta = Data_CityInfo.salaryRank - Data_CityInfo.playerRank;
    if (Data_CityInfo.playerRank != 0) {
        if (salary_delta > 0) {
            Data_CityInfo.ratingFavorSalaryPenalty = salary_delta + 1;
        }
    } else if (salary_delta > 0) {
        Data_CityInfo.ratingFavorSalaryPenalty = salary_delta;
    }

    if (Data_CityInfo.ratingFavorSalaryPenalty >= 8) {
        Data_CityInfo.ratingAdvisorExplanationFavor = 1;
    } else if (Data_CityInfo.tributeNotPaidTotalYears >= 3) {
        Data_CityInfo.ratingAdvisorExplanationFavor = 2;
    } else if (Data_CityInfo.ratingFavorIgnoredRequestPenalty >= 5) {
        Data_CityInfo.ratingAdvisorExplanationFavor = 3;
    } else if (Data_CityInfo.ratingFavorSalaryPenalty >= 5) {
        Data_CityInfo.ratingAdvisorExplanationFavor = 4;
    } else if (Data_CityInfo.tributeNotPaidTotalYears >= 2) {
        Data_CityInfo.ratingAdvisorExplanationFavor = 5;
    } else if (Data_CityInfo.ratingFavorIgnoredRequestPenalty >= 3) {
        Data_CityInfo.ratingAdvisorExplanationFavor = 6;
    } else if (Data_CityInfo.ratingFavorSalaryPenalty >= 3) {
        Data_CityInfo.ratingAdvisorExplanationFavor = 7;
    } else if (Data_CityInfo.tributeNotPaidLastYear) {
        Data_CityInfo.ratingAdvisorExplanationFavor = 8;
    } else if (Data_CityInfo.ratingFavorSalaryPenalty >= 2) {
        Data_CityInfo.ratingAdvisorExplanationFavor = 9;
    } else if (Data_CityInfo.ratingFavorMilestonePenalty) {
        Data_CityInfo.ratingAdvisorExplanationFavor = 10;
    } else if (Data_CityInfo.ratingFavorSalaryPenalty) {
        Data_CityInfo.ratingAdvisorExplanationFavor = 11;
    } else if (Data_CityInfo.ratingFavorChange == 2) { // rising
        Data_CityInfo.ratingAdvisorExplanationFavor = 12;
    } else if (Data_CityInfo.ratingFavorChange == 1) { // the same
        Data_CityInfo.ratingAdvisorExplanationFavor = 13;
    } else {
        Data_CityInfo.ratingAdvisorExplanationFavor = 0;
    }
}

void city_ratings_update_explanations()
{
    update_culture_explanation();
    update_prosperity_explanation();
    update_peace_explanation();
    city_ratings_update_favor_explanation();
}

static void update_culture_rating()
{
    Data_CityInfo.ratingCulture = 0;
    Data_CityInfo.ratingAdvisorExplanationCulture = 0;
    if (Data_CityInfo.population <= 0) {
        return;
    }

    int pct_theater = city_culture_coverage_theater();
    if (pct_theater >= 100) {
        Data_CityInfo.ratingCulturePointsTheater = 25;
    } else if (pct_theater > 85) {
        Data_CityInfo.ratingCulturePointsTheater = 18;
    } else if (pct_theater > 70) {
        Data_CityInfo.ratingCulturePointsTheater = 12;
    } else if (pct_theater > 50) {
        Data_CityInfo.ratingCulturePointsTheater = 8;
    } else if (pct_theater > 30) {
        Data_CityInfo.ratingCulturePointsTheater = 3;
    } else {
        Data_CityInfo.ratingCulturePointsTheater = 0;
    }
    Data_CityInfo.ratingCulture += Data_CityInfo.ratingCulturePointsTheater;

    int pct_religion = Data_CityInfo.cultureCoverageReligion;
    if (pct_religion >= 100) {
        Data_CityInfo.ratingCulturePointsReligion = 30;
    } else if (pct_religion > 85) {
        Data_CityInfo.ratingCulturePointsReligion = 22;
    } else if (pct_religion > 70) {
        Data_CityInfo.ratingCulturePointsReligion = 14;
    } else if (pct_religion > 50) {
        Data_CityInfo.ratingCulturePointsReligion = 9;
    } else if (pct_religion > 30) {
        Data_CityInfo.ratingCulturePointsReligion = 3;
    } else {
        Data_CityInfo.ratingCulturePointsReligion = 0;
    }
    Data_CityInfo.ratingCulture += Data_CityInfo.ratingCulturePointsReligion;

    int pct_school = city_culture_coverage_school();
    if (pct_school >= 100) {
        Data_CityInfo.ratingCulturePointsSchool = 15;
    } else if (pct_school > 85) {
        Data_CityInfo.ratingCulturePointsSchool = 10;
    } else if (pct_school > 70) {
        Data_CityInfo.ratingCulturePointsSchool = 6;
    } else if (pct_school > 50) {
        Data_CityInfo.ratingCulturePointsSchool = 4;
    } else if (pct_school > 30) {
        Data_CityInfo.ratingCulturePointsSchool = 1;
    } else {
        Data_CityInfo.ratingCulturePointsSchool = 0;
    }
    Data_CityInfo.ratingCulture += Data_CityInfo.ratingCulturePointsSchool;

    int pct_academy = city_culture_coverage_academy();
    if (pct_academy >= 100) {
        Data_CityInfo.ratingCulturePointsAcademy = 10;
    } else if (pct_academy > 85) {
        Data_CityInfo.ratingCulturePointsAcademy = 7;
    } else if (pct_academy > 70) {
        Data_CityInfo.ratingCulturePointsAcademy = 4;
    } else if (pct_academy > 50) {
        Data_CityInfo.ratingCulturePointsAcademy = 2;
    } else if (pct_academy > 30) {
        Data_CityInfo.ratingCulturePointsAcademy = 1;
    } else {
        Data_CityInfo.ratingCulturePointsAcademy = 0;
    }
    Data_CityInfo.ratingCulture += Data_CityInfo.ratingCulturePointsAcademy;

    int pct_library = city_culture_coverage_library();
    if (pct_library >= 100) {
        Data_CityInfo.ratingCulturePointsLibrary = 20;
    } else if (pct_library > 85) {
        Data_CityInfo.ratingCulturePointsLibrary = 14;
    } else if (pct_library > 70) {
        Data_CityInfo.ratingCulturePointsLibrary = 8;
    } else if (pct_library > 50) {
        Data_CityInfo.ratingCulturePointsLibrary = 4;
    } else if (pct_library > 30) {
        Data_CityInfo.ratingCulturePointsLibrary = 2;
    } else {
        Data_CityInfo.ratingCulturePointsLibrary = 0;
    }
    Data_CityInfo.ratingCulture += Data_CityInfo.ratingCulturePointsLibrary;

    Data_CityInfo.ratingCulture = calc_bound(Data_CityInfo.ratingCulture, 0, 100);
    update_culture_explanation();
}

static void update_prosperity_rating()
{
    int change = 0;
    // unemployment: -1 for too high, +1 for low
    if (Data_CityInfo.unemploymentPercentage < 5) {
        change += 1;
    } else if (Data_CityInfo.unemploymentPercentage >= 15) {
        change -= 1;
    }
    // losing/earning money: -1 for losing, +5 for profit
    if (Data_CityInfo.financeConstructionLastYear + Data_CityInfo.treasury <=
        Data_CityInfo.ratingProsperityTreasuryLastYear) {
        change -= 1;
    } else {
        change += 5;
    }
    Data_CityInfo.ratingProsperityTreasuryLastYear = Data_CityInfo.treasury;
    // food types: +1 for multiple foods
    if (Data_CityInfo.foodInfoFoodTypesEaten >= 2) {
        change += 1;
    }
    // wages: +1 for wages 2+ above Rome, -1 for wages below Rome
    int avg_wage = Data_CityInfo.wageRatePaidLastYear / 12;
    if (avg_wage >= Data_CityInfo.wagesRome + 2) {
        change += 1;
    } else if (avg_wage < Data_CityInfo.wagesRome) {
        change -= 1;
    }
    // high percentage poor: -1, high percentage rich: +1
    if (calc_percentage(Data_CityInfo.populationPeopleInTentsShacks, Data_CityInfo.population) > 30) {
        change -= 1;
    }
    if (calc_percentage(Data_CityInfo.populationPeopleInVillasPalaces, Data_CityInfo.population) > 10) {
        change += 1;
    }
    // tribute not paid: -1
    if (Data_CityInfo.tributeNotPaidLastYear) {
        change -= 1;
    }
    // working hippodrome: +1
    if (Data_CityInfo.entertainmentHippodromeShows > 0) {
        change += 1;
    }
    Data_CityInfo.ratingProsperity += change;
    if (Data_CityInfo.ratingProsperity > Data_CityInfo.ratingProsperityMax) {
        Data_CityInfo.ratingProsperity = Data_CityInfo.ratingProsperityMax;
    }
    Data_CityInfo.ratingProsperity = calc_bound(Data_CityInfo.ratingProsperity, 0, 100);

    update_prosperity_explanation();
}

static void calculate_max_prosperity()
{
    int points = 0;
    int houses = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state && b->houseSize) {
            points += model_get_house(b->subtype.houseLevel)->prosperity;
            houses++;
        }
    }
    if (houses > 0) {
        Data_CityInfo.ratingProsperityMax = points / houses;
    } else {
        Data_CityInfo.ratingProsperityMax = 0;
    }
}

static void update_peace_rating()
{
    int change = 0;
    if (Data_CityInfo.ratingPeaceYearsOfPeace < 2) {
        change += 2;
    } else {
        change += 5;
    }
    if (Data_CityInfo.ratingPeaceNumCriminalsThisYear) {
        change -= 1;
    }
    if (Data_CityInfo.ratingPeaceNumRiotersThisYear) {
        change -= 5;
    }
    if (Data_CityInfo.ratingPeaceNumDestroyedBuildingsThisYear) {
        change -= Data_CityInfo.ratingPeaceNumDestroyedBuildingsThisYear;
    }
    if (Data_CityInfo.ratingPeaceNumRiotersThisYear || Data_CityInfo.ratingPeaceNumDestroyedBuildingsThisYear) {
        Data_CityInfo.ratingPeaceYearsOfPeace = 0;
    } else {
        Data_CityInfo.ratingPeaceYearsOfPeace += 1;
    }
    Data_CityInfo.ratingPeaceNumCriminalsThisYear = 0;
    Data_CityInfo.ratingPeaceNumRiotersThisYear = 0;
    Data_CityInfo.ratingPeaceNumDestroyedBuildingsThisYear = 0;

    Data_CityInfo.ratingPeace = calc_bound(Data_CityInfo.ratingPeace + change, 0, 100);
    update_peace_explanation();
}

static void update_favor_rating(int is_yearly_update)
{
    if (scenario_is_open_play()) {
        Data_CityInfo.ratingFavor = 50;
        return;
    }
    Data_CityInfo.giftMonthsSinceLast++;
    if (Data_CityInfo.giftMonthsSinceLast >= 12) {
        Data_CityInfo.giftOverdosePenalty = 0;
    }
    if (is_yearly_update) {
        Data_CityInfo.ratingFavorSalaryPenalty = 0;
        Data_CityInfo.ratingFavorMilestonePenalty = 0;
        Data_CityInfo.ratingFavorIgnoredRequestPenalty = 0;
        if (!scenario_is_tutorial_1() && !scenario_is_tutorial_2()) {
            Data_CityInfo.ratingFavor -= 2;
        }
        // tribute penalty
        if (Data_CityInfo.tributeNotPaidLastYear) {
            if (Data_CityInfo.tributeNotPaidTotalYears <= 1) {
                Data_CityInfo.ratingFavor -= 3;
            } else if (Data_CityInfo.tributeNotPaidTotalYears <= 2) {
                Data_CityInfo.ratingFavor -= 5;
            } else {
                Data_CityInfo.ratingFavor -= 8;
            }
        }
        // salary
        int salary_delta = Data_CityInfo.salaryRank - Data_CityInfo.playerRank;
        if (Data_CityInfo.playerRank != 0) {
            if (salary_delta > 0) {
                // salary too high
                Data_CityInfo.ratingFavor -= salary_delta;
                Data_CityInfo.ratingFavorSalaryPenalty = salary_delta + 1;
            } else if (salary_delta < 0) {
                // salary lower than rank
                Data_CityInfo.ratingFavor += 1;
            }
        } else if (salary_delta > 0) {
            Data_CityInfo.ratingFavor -= salary_delta;
            Data_CityInfo.ratingFavorSalaryPenalty = salary_delta;
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
                Data_CityInfo.ratingCulture < calc_adjust_with_percentage(
                    scenario_criteria_culture(), milestone_pct)) {
                bonus = 0;
            }
            if (scenario_criteria_prosperity_enabled() &&
                Data_CityInfo.ratingProsperity < calc_adjust_with_percentage(
                    scenario_criteria_prosperity(), milestone_pct)) {
                bonus = 0;
            }
            if (scenario_criteria_peace_enabled() &&
                Data_CityInfo.ratingPeace < calc_adjust_with_percentage(
                    scenario_criteria_peace(), milestone_pct)) {
                bonus = 0;
            }
            if (scenario_criteria_favor_enabled() &&
                Data_CityInfo.ratingFavor < calc_adjust_with_percentage(
                    scenario_criteria_favor(), milestone_pct)) {
                bonus = 0;
            }
            if (scenario_criteria_population_enabled() &&
                Data_CityInfo.population < calc_adjust_with_percentage(
                    scenario_criteria_population(), milestone_pct)) {
                bonus = 0;
            }
            if (bonus) {
                Data_CityInfo.ratingFavor += 5;
            } else {
                Data_CityInfo.ratingFavor -= 2;
                Data_CityInfo.ratingFavorMilestonePenalty = 2;
            }
        }

        if (Data_CityInfo.ratingFavor < Data_CityInfo.ratingFavorLastYear) {
            Data_CityInfo.ratingFavorChange = 0;
        } else if (Data_CityInfo.ratingFavor == Data_CityInfo.ratingFavorLastYear) {
            Data_CityInfo.ratingFavorChange = 1;
        } else {
            Data_CityInfo.ratingFavorChange = 2;
        }
        Data_CityInfo.ratingFavorLastYear = Data_CityInfo.ratingFavor;
    }
    Data_CityInfo.ratingFavor = calc_bound(Data_CityInfo.ratingFavor, 0, 100);
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
