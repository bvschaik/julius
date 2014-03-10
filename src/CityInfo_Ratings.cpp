#include "CityInfo.h"
#include "Data/CityInfo.h"
#include "Data/Scenario.h"
#include "Data/Building.h"
#include "Data/Model.h"
#include "Util.h"
#include "Calc.h"

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

	int pctTheater = Data_CityInfo_CultureCoverage.theater;
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

	int pctSchool = Data_CityInfo_CultureCoverage.school;
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

	int pctAcademy = Data_CityInfo_CultureCoverage.academy;
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

	int pctLibrary = Data_CityInfo_CultureCoverage.library;
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

	BOUND(Data_CityInfo.ratingCulture, 0, 100);
	CityInfo_Ratings_updateCultureExplanation();
}

void CityInfo_Ratings_updateCultureExplanation()
{
	int minPercentage = 100;
	int reason = 0;
	if (Data_CityInfo.cultureCoverageReligion < minPercentage) {
		minPercentage = Data_CityInfo.cultureCoverageReligion;
		reason = 4;
	}
	if (Data_CityInfo_CultureCoverage.theater < minPercentage) {
		minPercentage = Data_CityInfo_CultureCoverage.theater;
		reason = 5;
	}
	if (Data_CityInfo_CultureCoverage.library < minPercentage) {
		minPercentage = Data_CityInfo_CultureCoverage.library;
		reason = 2;
	}
	if (Data_CityInfo_CultureCoverage.school < minPercentage) {
		minPercentage = Data_CityInfo_CultureCoverage.school;
		reason = 1;
	}
	if (Data_CityInfo_CultureCoverage.academy < minPercentage) {
		minPercentage = Data_CityInfo_CultureCoverage.academy;
		reason = 3;
	}
	Data_CityInfo.ratingAdvisorExplanationCulture = reason;
}

static void updateFavorRating(int isYearlyUpdate)
{
	if (Data_Scenario.isOpenPlay) {
		Data_CityInfo.ratingFavor = 50;
		return;
	}
	Data_CityInfo.giftMonthsSinceLast++;
	if (Data_CityInfo.giftMonthsSinceLast >= 12) {
		Data_CityInfo.giftOverdosePenalty = 0;
	}
	if (isYearlyUpdate) {
		
	}
	if (Data_CityInfo.ratingFavor < 0) {
		Data_CityInfo.ratingFavor = 0;
	}
	if (Data_CityInfo.ratingFavor > 100) {
		Data_CityInfo.ratingFavor = 100;
	}
	CityInfo_Ratings_updateFavorExplanation();
}

void CityInfo_Ratings_updateFavorExplanation()
{
	Data_CityInfo.salaryDifferenceFromRank = 0;
	int salaryDifference = Data_CityInfo.salaryRank - Data_CityInfo.playerRank;
	if (Data_CityInfo.playerRank) {
		if (salaryDifference > 0) {
			Data_CityInfo.salaryDifferenceFromRank = salaryDifference + 1;
		}
	} else if (salaryDifference > 0) {
		Data_CityInfo.salaryDifferenceFromRank = salaryDifference;
	}

	if (Data_CityInfo.salaryDifferenceFromRank >= 8) {
		Data_CityInfo.ratingAdvisorExplanationFavor = 1;
	}// else if (
	// TODO
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
	if (Calc_getPercentage(Data_CityInfo.populationPeopleInTentsShacks, Data_CityInfo.population) > 30) {
		change -= 1;
	}
	if (Calc_getPercentage(Data_CityInfo.populationPeopleInVillasPalaces, Data_CityInfo.population) > 10) {
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
	BOUND(Data_CityInfo.ratingProsperity, 0, 100);

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
	int pctTents = Calc_getPercentage(Data_CityInfo.populationPeopleInTentsShacks, Data_CityInfo.population);
	if (pctTents > 30) {
		change -= 1;
	}
	if (Calc_getPercentage(Data_CityInfo.populationPeopleInVillasPalaces, Data_CityInfo.population) > 10) {
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
	if (Data_CityInfo.ratingProsperity <= 0 || Data_CityInfo_Extra.gameTimeYear == Data_Scenario.startYear) {
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
		if (Data_Buildings[i].inUse && Data_Buildings[i].houseSize) {
			points += Data_Model_Houses[Data_Buildings[i].subtype.houseLevel].prosperity;
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
		change -= 1;
	}
	if (Data_CityInfo.ratingPeaceNumRiotersThisYear || Data_CityInfo.ratingPeaceNumDestroyedBuildingsThisYear) {
		Data_CityInfo.ratingPeaceYearsOfPeace = 0;
	} else {
		Data_CityInfo.ratingPeaceYearsOfPeace += 1;
	}
	Data_CityInfo.ratingPeaceNumCriminalsThisYear = 0;
	Data_CityInfo.ratingPeaceNumRiotersThisYear = 0;
	Data_CityInfo.ratingPeaceNumDestroyedBuildingsThisYear = 0;

	Data_CityInfo.ratingPeace += change;
	BOUND(Data_CityInfo.ratingPeace, 0, 100);
	CityInfo_Ratings_updatePeaceExplanation();
}

void CityInfo_Ratings_updatePeaceExplanation()
{
	int reason;
	if (Data_CityInfo.numImperialSoldiersInCity) {
		reason = 8; // BUGFIX: 7+8 interchanged
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

void CityInfo_Ratings_sendGiftToCaesar()
{
	int cost;
	if (Data_CityInfo.giftSizeSelected == 0) {
		cost = Data_CityInfo.giftCost_modest;
	} else if (Data_CityInfo.giftSizeSelected == 1) {
		cost = Data_CityInfo.giftCost_generous;
	} else if (Data_CityInfo.giftSizeSelected == 2) {
		cost = Data_CityInfo.giftCost_lavish;
	}

	if (cost > Data_CityInfo.personalSavings) {
		return;
	}

	if (Data_CityInfo.giftOverdosePenalty <= 0) {
		Data_CityInfo.giftOverdosePenalty = 1;
		if (Data_CityInfo.giftSizeSelected == 0) {
			CityInfo_Ratings_increaseFavor(3);
		} else if (Data_CityInfo.giftSizeSelected == 1) {
			CityInfo_Ratings_increaseFavor(5);
		} else if (Data_CityInfo.giftSizeSelected == 2) {
			CityInfo_Ratings_increaseFavor(10);
		}
	} else if (Data_CityInfo.giftOverdosePenalty == 1) {
		Data_CityInfo.giftOverdosePenalty = 2;
		if (Data_CityInfo.giftSizeSelected == 0) {
			CityInfo_Ratings_increaseFavor(1);
		} else if (Data_CityInfo.giftSizeSelected == 1) {
			CityInfo_Ratings_increaseFavor(3);
		} else if (Data_CityInfo.giftSizeSelected == 2) {
			CityInfo_Ratings_increaseFavor(5);
		}
	} else if (Data_CityInfo.giftOverdosePenalty == 2) {
		Data_CityInfo.giftOverdosePenalty = 3;
		if (Data_CityInfo.giftSizeSelected == 0) {
			CityInfo_Ratings_increaseFavor(0);
		} else if (Data_CityInfo.giftSizeSelected == 1) {
			CityInfo_Ratings_increaseFavor(1);
		} else if (Data_CityInfo.giftSizeSelected == 2) {
			CityInfo_Ratings_increaseFavor(3);
		}
	} else if (Data_CityInfo.giftOverdosePenalty == 3) {
		Data_CityInfo.giftOverdosePenalty = 4;
		if (Data_CityInfo.giftSizeSelected == 0) {
			CityInfo_Ratings_increaseFavor(0);
		} else if (Data_CityInfo.giftSizeSelected == 1) {
			CityInfo_Ratings_increaseFavor(0);
		} else if (Data_CityInfo.giftSizeSelected == 2) {
			CityInfo_Ratings_increaseFavor(1);
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

void CityInfo_Ratings_increaseFavor(int amount)
{
	Data_CityInfo.ratingFavor += amount;
	BOUND(Data_CityInfo.ratingFavor, 0, 100);
}

void CityInfo_Ratings_setMaxFavor(int maxFavor)
{
	if (Data_CityInfo.ratingFavor > maxFavor) {
		Data_CityInfo.ratingFavor = maxFavor;
	}
}
