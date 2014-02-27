#include "CityInfoUpdater.h"
#include "Data/CityInfo.h"
#include "Data/Scenario.h"
#include "Util.h"

static void updateCultureRating();
static void updateFavorRating(int isYearlyUpdate);
static void updateProsperityRating();
static void updatePeaceRating();

void CityInfoUpdater_Ratings_calculate(int isYearlyUpdate)
{
	updateCultureRating();
	updateFavorRating(isYearlyUpdate);
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
	CityInfoUpdater_Ratings_updateCultureExplanation();
}

void CityInfoUpdater_Ratings_updateCultureExplanation()
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
	CityInfoUpdater_Ratings_updateFavorExplanation();
}

void CityInfoUpdater_Ratings_updateFavorExplanation()
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
	// TODO
}
static void updatePeaceRating()
{
	// TODO
}

void CityInfoUpdater_Ratings_sendGiftToCaesar()
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
			CityInfoUpdater_Ratings_increaseFavor(3);
		} else if (Data_CityInfo.giftSizeSelected == 1) {
			CityInfoUpdater_Ratings_increaseFavor(5);
		} else if (Data_CityInfo.giftSizeSelected == 2) {
			CityInfoUpdater_Ratings_increaseFavor(10);
		}
	} else if (Data_CityInfo.giftOverdosePenalty == 1) {
		Data_CityInfo.giftOverdosePenalty = 2;
		if (Data_CityInfo.giftSizeSelected == 0) {
			CityInfoUpdater_Ratings_increaseFavor(1);
		} else if (Data_CityInfo.giftSizeSelected == 1) {
			CityInfoUpdater_Ratings_increaseFavor(3);
		} else if (Data_CityInfo.giftSizeSelected == 2) {
			CityInfoUpdater_Ratings_increaseFavor(5);
		}
	} else if (Data_CityInfo.giftOverdosePenalty == 2) {
		Data_CityInfo.giftOverdosePenalty = 3;
		if (Data_CityInfo.giftSizeSelected == 0) {
			CityInfoUpdater_Ratings_increaseFavor(0);
		} else if (Data_CityInfo.giftSizeSelected == 1) {
			CityInfoUpdater_Ratings_increaseFavor(1);
		} else if (Data_CityInfo.giftSizeSelected == 2) {
			CityInfoUpdater_Ratings_increaseFavor(3);
		}
	} else if (Data_CityInfo.giftOverdosePenalty == 3) {
		Data_CityInfo.giftOverdosePenalty = 4;
		if (Data_CityInfo.giftSizeSelected == 0) {
			CityInfoUpdater_Ratings_increaseFavor(0);
		} else if (Data_CityInfo.giftSizeSelected == 1) {
			CityInfoUpdater_Ratings_increaseFavor(0);
		} else if (Data_CityInfo.giftSizeSelected == 2) {
			CityInfoUpdater_Ratings_increaseFavor(1);
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

void CityInfoUpdater_Ratings_increaseFavor(int amount)
{
	Data_CityInfo.ratingFavor += amount;
	BOUND(Data_CityInfo.ratingFavor, 0, 100);
}

void CityInfoUpdater_Ratings_setMaxFavor(int maxFavor)
{
	if (Data_CityInfo.ratingFavor > maxFavor) {
		Data_CityInfo.ratingFavor = maxFavor;
	}
}
