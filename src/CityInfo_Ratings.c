#include "CityInfo.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Settings.h"

#include "building/model.h"
#include "city/culture.h"
#include "core/calc.h"
#include "game/time.h"
#include "scenario/criteria.h"
#include "scenario/property.h"

static void updateCultureRating();
static void updateFavorRating(int isYearlyUpdate);
static void updateProsperityRating();
static void calculateMaxProsperity();
static void updatePeaceRating();

void CityInfo_Ratings_calculate(int isYearlyUpdate)
{
	updateCultureRating();
	updateFavorRating(isYearlyUpdate);
	calculateMaxProsperity();
	if (isYearlyUpdate) {
		updateProsperityRating();
		updatePeaceRating();
	}
}

static void updateCultureRating()
{
	Data_CityInfo.ratingCulture = 0;
	Data_CityInfo.ratingAdvisorExplanationCulture = 0;
	if (Data_CityInfo.population <= 0) {
		return;
	}

	int pctTheater = city_culture_coverage_theater();
	if (pctTheater >= 100) {
		Data_CityInfo.ratingCulturePointsTheater = 25;
	} else if (pctTheater > 85) {
		Data_CityInfo.ratingCulturePointsTheater = 18;
	} else if (pctTheater > 70) {
		Data_CityInfo.ratingCulturePointsTheater = 12;
	} else if (pctTheater > 50) {
		Data_CityInfo.ratingCulturePointsTheater = 8;
	} else if (pctTheater > 30) {
		Data_CityInfo.ratingCulturePointsTheater = 3;
	} else {
		Data_CityInfo.ratingCulturePointsTheater = 0;
	}
	Data_CityInfo.ratingCulture += Data_CityInfo.ratingCulturePointsTheater;

	int pctReligion = Data_CityInfo.cultureCoverageReligion;
	if (pctReligion >= 100) {
		Data_CityInfo.ratingCulturePointsReligion = 30;
	} else if (pctReligion > 85) {
		Data_CityInfo.ratingCulturePointsReligion = 22;
	} else if (pctReligion > 70) {
		Data_CityInfo.ratingCulturePointsReligion = 14;
	} else if (pctReligion > 50) {
		Data_CityInfo.ratingCulturePointsReligion = 9;
	} else if (pctReligion > 30) {
		Data_CityInfo.ratingCulturePointsReligion = 3;
	} else {
		Data_CityInfo.ratingCulturePointsReligion = 0;
	}
	Data_CityInfo.ratingCulture += Data_CityInfo.ratingCulturePointsReligion;

	int pctSchool = city_culture_coverage_school();
	if (pctSchool >= 100) {
		Data_CityInfo.ratingCulturePointsSchool = 15;
	} else if (pctSchool > 85) {
		Data_CityInfo.ratingCulturePointsSchool = 10;
	} else if (pctSchool > 70) {
		Data_CityInfo.ratingCulturePointsSchool = 6;
	} else if (pctSchool > 50) {
		Data_CityInfo.ratingCulturePointsSchool = 4;
	} else if (pctSchool > 30) {
		Data_CityInfo.ratingCulturePointsSchool = 1;
	} else {
		Data_CityInfo.ratingCulturePointsSchool = 0;
	}
	Data_CityInfo.ratingCulture += Data_CityInfo.ratingCulturePointsSchool;

	int pctAcademy = city_culture_coverage_academy();
	if (pctAcademy >= 100) {
		Data_CityInfo.ratingCulturePointsAcademy = 10;
	} else if (pctAcademy > 85) {
		Data_CityInfo.ratingCulturePointsAcademy = 7;
	} else if (pctAcademy > 70) {
		Data_CityInfo.ratingCulturePointsAcademy = 4;
	} else if (pctAcademy > 50) {
		Data_CityInfo.ratingCulturePointsAcademy = 2;
	} else if (pctAcademy > 30) {
		Data_CityInfo.ratingCulturePointsAcademy = 1;
	} else {
		Data_CityInfo.ratingCulturePointsAcademy = 0;
	}
	Data_CityInfo.ratingCulture += Data_CityInfo.ratingCulturePointsAcademy;

	int pctLibrary = city_culture_coverage_library();
	if (pctLibrary >= 100) {
		Data_CityInfo.ratingCulturePointsLibrary = 20;
	} else if (pctLibrary > 85) {
		Data_CityInfo.ratingCulturePointsLibrary = 14;
	} else if (pctLibrary > 70) {
		Data_CityInfo.ratingCulturePointsLibrary = 8;
	} else if (pctLibrary > 50) {
		Data_CityInfo.ratingCulturePointsLibrary = 4;
	} else if (pctLibrary > 30) {
		Data_CityInfo.ratingCulturePointsLibrary = 2;
	} else {
		Data_CityInfo.ratingCulturePointsLibrary = 0;
	}
	Data_CityInfo.ratingCulture += Data_CityInfo.ratingCulturePointsLibrary;

    Data_CityInfo.ratingCulture = calc_bound(Data_CityInfo.ratingCulture, 0, 100);
	CityInfo_Ratings_updateCultureExplanation();
}

void CityInfo_Ratings_updateCultureExplanation()
{
	int minPercentage = 100;
	int reason = 1;
	if (Data_CityInfo.cultureCoverageReligion < minPercentage) {
		minPercentage = Data_CityInfo.cultureCoverageReligion;
		reason = 4;
	}
	int pctTheater = city_culture_coverage_theater();
	if (pctTheater < minPercentage) {
		minPercentage = pctTheater;
		reason = 5;
	}
	int pctLibrary = city_culture_coverage_library();
	if (pctLibrary < minPercentage) {
		minPercentage = pctLibrary;
		reason = 2;
	}
	int pctSchool = city_culture_coverage_school();
	if (pctSchool < minPercentage) {
		minPercentage = pctSchool;
		reason = 1;
	}
	int pctAcademy = city_culture_coverage_academy();
	if (pctAcademy < minPercentage) {
		minPercentage = pctAcademy;
		reason = 3;
	}
	Data_CityInfo.ratingAdvisorExplanationCulture = reason;
}

static void updateFavorRating(int isYearlyUpdate)
{
	if (scenario_is_open_play()) {
		Data_CityInfo.ratingFavor = 50;
		return;
	}
	Data_CityInfo.giftMonthsSinceLast++;
	if (Data_CityInfo.giftMonthsSinceLast >= 12) {
		Data_CityInfo.giftOverdosePenalty = 0;
	}
	if (isYearlyUpdate) {
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
		int salaryDiff = Data_CityInfo.salaryRank - Data_CityInfo.playerRank;
		if (Data_CityInfo.playerRank != 0) {
			if (salaryDiff > 0) {
				// salary too high
				Data_CityInfo.ratingFavor -= salaryDiff;
				Data_CityInfo.ratingFavorSalaryPenalty = salaryDiff + 1;
			} else if (salaryDiff < 0) {
				// salary lower than rank
				Data_CityInfo.ratingFavor += 1;
			}
		} else if (salaryDiff > 0) {
			Data_CityInfo.ratingFavor -= salaryDiff;
			Data_CityInfo.ratingFavorSalaryPenalty = salaryDiff;
		}
		// milestone
		int milestonePct;
		if (scenario_criteria_milestone_year(25) == game_time_year()) {
			milestonePct = 25;
		} else if (scenario_criteria_milestone_year(50) == game_time_year()) {
			milestonePct = 50;
		} else if (scenario_criteria_milestone_year(75) == game_time_year()) {
			milestonePct = 75;
		} else {
			milestonePct = 0;
		}
		if (milestonePct) {
			int bonus = 1;
			if (scenario_criteria_culture_enabled() &&
				Data_CityInfo.ratingCulture < calc_adjust_with_percentage(
					scenario_criteria_culture(), milestonePct)) {
				bonus = 0;
			}
			if (scenario_criteria_prosperity_enabled() &&
				Data_CityInfo.ratingProsperity < calc_adjust_with_percentage(
					scenario_criteria_prosperity(), milestonePct)) {
				bonus = 0;
			}
			if (scenario_criteria_peace_enabled() &&
				Data_CityInfo.ratingPeace < calc_adjust_with_percentage(
					scenario_criteria_peace(), milestonePct)) {
				bonus = 0;
			}
			if (scenario_criteria_favor_enabled() &&
				Data_CityInfo.ratingFavor < calc_adjust_with_percentage(
					scenario_criteria_favor(), milestonePct)) {
				bonus = 0;
			}
			if (scenario_criteria_population_enabled() &&
				Data_CityInfo.population < calc_adjust_with_percentage(
					scenario_criteria_population(), milestonePct)) {
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
	CityInfo_Ratings_updateFavorExplanation();
}

void CityInfo_Ratings_updateFavorExplanation()
{
	Data_CityInfo.ratingFavorSalaryPenalty = 0;
	int salaryDiff = Data_CityInfo.salaryRank - Data_CityInfo.playerRank;
	if (Data_CityInfo.playerRank != 0) {
		if (salaryDiff > 0) {
			Data_CityInfo.ratingFavorSalaryPenalty = salaryDiff + 1;
		}
	} else if (salaryDiff > 0) {
		Data_CityInfo.ratingFavorSalaryPenalty = salaryDiff;
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

static void updateProsperityRating()
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
	int avgWage = Data_CityInfo.wageRatePaidLastYear / 12;
	if (avgWage >= Data_CityInfo.wagesRome + 2) {
		change += 1;
	} else if (avgWage < Data_CityInfo.wagesRome) {
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

	CityInfo_Ratings_updateProsperityExplanation();
}

void CityInfo_Ratings_updateProsperityExplanation()
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
	int avgWage = Data_CityInfo.wageRatePaidLastYear / 12;
	if (avgWage >= Data_CityInfo.wagesRome + 2) {
		change += 1;
	} else if (avgWage < Data_CityInfo.wagesRome) {
		change -= 1;
	}
	// high percentage poor: -1, high percentage rich: +1
	int pctTents = calc_percentage(Data_CityInfo.populationPeopleInTentsShacks, Data_CityInfo.population);
	if (pctTents > 30) {
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
	} else if (avgWage < Data_CityInfo.wagesRome) {
		reason = 5;
	} else if (pctTents > 30) {
		reason = 6;
	} else if (Data_CityInfo.tributeNotPaidLastYear) {
		reason = 7;
	} else {
		reason = 9;
	}
	// 8 = for bailout
	Data_CityInfo.ratingAdvisorExplanationProsperity = reason;
}

void CityInfo_Ratings_reduceProsperityAfterBailout()
{
	Data_CityInfo.ratingProsperity -= 3;
	if (Data_CityInfo.ratingProsperity < 0) {
		Data_CityInfo.ratingProsperity = 0;
	}
	Data_CityInfo.ratingAdvisorExplanationProsperity = 8;
}

static void calculateMaxProsperity()
{
	int points = 0;
	int houses = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (Data_Buildings[i].state && Data_Buildings[i].houseSize) {
			points += model_get_house(Data_Buildings[i].subtype.houseLevel)->prosperity;
			houses++;
		}
	}
	if (houses > 0) {
		Data_CityInfo.ratingProsperityMax = points / houses;
	} else {
		Data_CityInfo.ratingProsperityMax = 0;
	}
}

static void updatePeaceRating()
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
	CityInfo_Ratings_updatePeaceExplanation();
}

void CityInfo_Ratings_updatePeaceExplanation()
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
//REVIEW
void CityInfo_Ratings_sendGiftToCaesar()
{
	int cost;
	if (Data_CityInfo.giftSizeSelected == 0) {
		cost = Data_CityInfo.giftCost_modest;
	} else if (Data_CityInfo.giftSizeSelected == 1) {
		cost = Data_CityInfo.giftCost_generous;
	} else if (Data_CityInfo.giftSizeSelected == 2) {
		cost = Data_CityInfo.giftCost_lavish;
	} else {
		return;
	}

	if (cost > Data_CityInfo.personalSavings) {
		return;
	}

	if (Data_CityInfo.giftOverdosePenalty <= 0) {
		Data_CityInfo.giftOverdosePenalty = 1;
		if (Data_CityInfo.giftSizeSelected == 0) {
			CityInfo_Ratings_changeFavor(3);
		} else if (Data_CityInfo.giftSizeSelected == 1) {
			CityInfo_Ratings_changeFavor(5);
		} else if (Data_CityInfo.giftSizeSelected == 2) {
			CityInfo_Ratings_changeFavor(10);
		}
	} else if (Data_CityInfo.giftOverdosePenalty == 1) {
		Data_CityInfo.giftOverdosePenalty = 2;
		if (Data_CityInfo.giftSizeSelected == 0) {
			CityInfo_Ratings_changeFavor(1);
		} else if (Data_CityInfo.giftSizeSelected == 1) {
			CityInfo_Ratings_changeFavor(3);
		} else if (Data_CityInfo.giftSizeSelected == 2) {
			CityInfo_Ratings_changeFavor(5);
		}
	} else if (Data_CityInfo.giftOverdosePenalty == 2) {
		Data_CityInfo.giftOverdosePenalty = 3;
		if (Data_CityInfo.giftSizeSelected == 0) {
			CityInfo_Ratings_changeFavor(0);
		} else if (Data_CityInfo.giftSizeSelected == 1) {
			CityInfo_Ratings_changeFavor(1);
		} else if (Data_CityInfo.giftSizeSelected == 2) {
			CityInfo_Ratings_changeFavor(3);
		}
	} else if (Data_CityInfo.giftOverdosePenalty == 3) {
		Data_CityInfo.giftOverdosePenalty = 4;
		if (Data_CityInfo.giftSizeSelected == 0) {
			CityInfo_Ratings_changeFavor(0);
		} else if (Data_CityInfo.giftSizeSelected == 1) {
			CityInfo_Ratings_changeFavor(0);
		} else if (Data_CityInfo.giftSizeSelected == 2) {
			CityInfo_Ratings_changeFavor(1);
		}
	}

	Data_CityInfo.giftMonthsSinceLast = 0;
	// rotate gift type
	if (Data_CityInfo.giftSizeSelected == 0) {
		Data_CityInfo.giftId_modest++;
	} else if (Data_CityInfo.giftSizeSelected == 1) {
		Data_CityInfo.giftId_generous++;
	} else if (Data_CityInfo.giftSizeSelected == 2) {
		Data_CityInfo.giftId_lavish++;
	}
	if (Data_CityInfo.giftId_modest >= 4) {
		Data_CityInfo.giftId_modest = 0;
	}
	if (Data_CityInfo.giftId_generous >= 4) {
		Data_CityInfo.giftId_generous = 0;
	}
	if (Data_CityInfo.giftId_lavish >= 4) {
		Data_CityInfo.giftId_lavish = 0;
	}

	Data_CityInfo.personalSavings -= cost;
}

void CityInfo_Ratings_changeFavor(int amount)
{
	Data_CityInfo.ratingFavor = calc_bound(Data_CityInfo.ratingFavor + amount, 0, 100);
}

void CityInfo_Ratings_setMaxFavor(int maxFavor)
{
	if (Data_CityInfo.ratingFavor > maxFavor) {
		Data_CityInfo.ratingFavor = maxFavor;
	}
}
