#include "CityInfo.h"

#include "Building.h"
#include "HousePopulation.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Settings.h"

#include "building/model.h"
#include "city/message.h"
#include "core/calc.h"
#include "core/random.h"
#include "game/difficulty.h"
#include "game/tutorial.h"

static void addPeopleToCensus(int numPeople);
static void removePeopleFromCensus(int numPeople);
static void removePeopleFromCensusInDecennium(int decennium, int numPeople);
static int getPeopleInAgeDecennium(int decennium);
static void recalculatePopulation();
static void healthCauseDisease(int totalPeople);

static const int yearlyBirthsPerDecennium[10] = {
	0, 3, 16, 9, 2, 0, 0, 0, 0, 0
};

static const int yearlyDeathsPerHealthPerDecennium[11][10] = {
	{20, 10, 5, 10, 20, 30, 50, 85, 100, 100},
	{15, 8, 4, 8, 16, 25, 45, 70, 90, 100},
	{10, 6, 2, 6, 12, 20, 30, 55, 80, 90},
	{5, 4, 0, 4, 8, 15, 25, 40, 65, 80},
	{3, 2, 0, 2, 6, 12, 20, 30, 50, 70},
	{2, 0, 0, 0, 4, 8, 15, 25, 40, 60},
	{1, 0, 0, 0, 2, 6, 12, 20, 30, 50},
	{0, 0, 0, 0, 0, 4, 8, 15, 20, 40},
	{0, 0, 0, 0, 0, 2, 6, 10, 15, 30},
	{0, 0, 0, 0, 0, 0, 4, 5, 10, 20},
	{0, 0, 0, 0, 0, 0, 0, 2, 5, 10}
};

static const int sentimentPerTaxRate[26] = {
	3, 2, 2, 2, 1, 1, 1, 0, 0, -1,
	-2, -2, -3, -3, -3, -5, -5, -5, -5, -6,
	-6, -6, -6, -6, -6, -6
};

void CityInfo_Population_recordMonthlyPopulation()
{
	Data_CityInfo.monthlyPopulation[Data_CityInfo.monthlyPopulationNextIndex++] = Data_CityInfo.population;
	if (Data_CityInfo.monthlyPopulationNextIndex >= 2400) {
		Data_CityInfo.monthlyPopulationNextIndex = 0;
	}
	++Data_CityInfo.monthsSinceStart;
}

void CityInfo_Population_changeHappiness(int amount)
{
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (BuildingIsInUse(i) && Data_Buildings[i].houseSize) {
			Data_Buildings[i].sentiment.houseHappiness += amount;
			Data_Buildings[i].sentiment.houseHappiness =
                calc_bound(Data_Buildings[i].sentiment.houseHappiness, 0, 100);
		}
	}
}

void CityInfo_Population_setMaxHappiness(int max)
{
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (BuildingIsInUse(i) && Data_Buildings[i].houseSize) {
			if (Data_Buildings[i].sentiment.houseHappiness > max) {
				Data_Buildings[i].sentiment.houseHappiness = max;
			}
			Data_Buildings[i].sentiment.houseHappiness =
                calc_bound(Data_Buildings[i].sentiment.houseHappiness, 0, 100);
		}
	}
}

static int getSentimentPenaltyForTentDwellers()
{
	int penalty;
	int pctTents = calc_percentage(
		Data_CityInfo.populationPeopleInTents, Data_CityInfo.population);
	if (Data_CityInfo.populationPeopleInVillasPalaces > 0) {
		if (pctTents >= 57) {
			penalty = 0;
		} else if (pctTents >= 40) {
			penalty = -3;
		} else if (pctTents >= 26) {
			penalty = -4;
		} else if (pctTents >= 10) {
			penalty = -5;
		} else {
			penalty = -6;
		}
	} else if (Data_CityInfo.populationPeopleInLargeInsulaAndAbove > 0) {
		if (pctTents >= 57) {
			penalty = 0;
		} else if (pctTents >= 40) {
			penalty = -2;
		} else if (pctTents >= 26) {
			penalty = -3;
		} else if (pctTents >= 10) {
			penalty = -4;
		} else {
			penalty = -5;
		}
	} else {
		if (pctTents >= 40) {
			penalty = 0;
		} else if (pctTents >= 26) {
			penalty = -1;
		} else if (pctTents >= 10) {
			penalty = -2;
		} else {
			penalty = -3;
		}
	}
	return penalty;
}

void CityInfo_Population_calculateSentiment()
{
	int peopleInHouses = HousePopulation_calculatePeoplePerType();
	if (peopleInHouses < Data_CityInfo.population) {
		removePeopleFromCensus(Data_CityInfo.population - peopleInHouses);
	}

	int sentimentPenaltyTents;
	int sentimentContributionEmployment = 0;
	int sentimentContributionWages = 0;
	int sentimentContributionTaxes = sentimentPerTaxRate[Data_CityInfo.taxPercentage];

	// wages contribution
	int wageDiff = Data_CityInfo.wages - Data_CityInfo.wagesRome;
	if (wageDiff < 0) {
		sentimentContributionWages = wageDiff / 2;
		if (!sentimentContributionWages) {
			sentimentContributionWages = -1;
		}
	} else if (wageDiff > 7) {
		sentimentContributionWages = 4;
	} else if (wageDiff > 4) {
		sentimentContributionWages = 3;
	} else if (wageDiff > 1) {
		sentimentContributionWages = 2;
	} else if (wageDiff > 0) {
		sentimentContributionWages = 1;
	}
	Data_CityInfo.populationSentimentWages = Data_CityInfo.wages;
	// unemployment contribution
	if (Data_CityInfo.unemploymentPercentage > 25) {
		sentimentContributionEmployment = -3;
	} else if (Data_CityInfo.unemploymentPercentage > 17) {
		sentimentContributionEmployment = -2;
	} else if (Data_CityInfo.unemploymentPercentage > 10) {
		sentimentContributionEmployment = -1;
	} else if (Data_CityInfo.unemploymentPercentage > 4) {
		sentimentContributionEmployment = 0;
	} else {
		sentimentContributionEmployment = 1;
	}
	Data_CityInfo.populationSentimentUnemployment = Data_CityInfo.unemploymentPercentage;
	// tent contribution
	if (Data_CityInfo.populationSentimentIncludeTents) {
		sentimentPenaltyTents = getSentimentPenaltyForTentDwellers();
		Data_CityInfo.populationSentimentIncludeTents = 0;
	} else {
		sentimentPenaltyTents = 0;
		Data_CityInfo.populationSentimentIncludeTents = 1;
	}

	int housesCalculated = 0;
	int housesNeedingFood = 0;
	int totalSentimentContributionFood = 0;
	int totalSentimentPenaltyTents = 0;
    int defaultSentiment = difficulty_sentiment();
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (!BuildingIsInUse(i) || !b->houseSize) {
			continue;
		}
		if (!b->housePopulation) {
			b->sentiment.houseHappiness = 10 + defaultSentiment;
			continue;
		}
		if (Data_CityInfo.population < 300) {
			// small town has no complaints
			sentimentContributionEmployment = 0;
			sentimentContributionTaxes = 0;
			sentimentContributionWages = 0;

			b->sentiment.houseHappiness = defaultSentiment;
			if (Data_CityInfo.population < 200) {
				b->sentiment.houseHappiness += 10;
			}
			continue;
		}
		// population >= 300
		housesCalculated++;
		int sentimentContributionFood = 0;
		int sentimentContributionTents = 0;
		if (!model_get_house(b->subtype.houseLevel)->food_types) {
			// tents
			b->houseDaysWithoutFood = 0;
			sentimentContributionTents = sentimentPenaltyTents;
			totalSentimentPenaltyTents += sentimentPenaltyTents;
		} else {
			// shack+
			housesNeedingFood++;
			if (b->data.house.numFoods >= 2) {
				sentimentContributionFood = 2;
				totalSentimentContributionFood += 2;
				b->houseDaysWithoutFood = 0;
			} else if (b->data.house.numFoods >= 1) {
				sentimentContributionFood = 1;
				totalSentimentContributionFood += 1;
				b->houseDaysWithoutFood = 0;
			} else {
				// needs food but has no food
				if (b->houseDaysWithoutFood < 3) {
					b->houseDaysWithoutFood++;
				}
				sentimentContributionFood = -b->houseDaysWithoutFood;
				totalSentimentContributionFood -= b->houseDaysWithoutFood;
			}
		}
		b->sentiment.houseHappiness += sentimentContributionTaxes;
		b->sentiment.houseHappiness += sentimentContributionWages;
		b->sentiment.houseHappiness += sentimentContributionEmployment;
		b->sentiment.houseHappiness += sentimentContributionFood;
		b->sentiment.houseHappiness += sentimentContributionTents;
		b->sentiment.houseHappiness = calc_bound(b->sentiment.houseHappiness, 0, 100);
	}

	int sentimentContributionFood = 0;
	int sentimentContributionTents = 0;
	if (housesNeedingFood) {
		sentimentContributionFood = totalSentimentContributionFood / housesNeedingFood;
	}
	if (housesCalculated) {
		sentimentContributionTents = totalSentimentPenaltyTents / housesCalculated;
	}

	int totalSentiment = 0;
	int totalHouses = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (BuildingIsInUse(i) && Data_Buildings[i].houseSize &&
			Data_Buildings[i].housePopulation) {
			totalHouses++;
			totalSentiment += Data_Buildings[i].sentiment.houseHappiness;
		}
	}
	if (totalHouses) {
		Data_CityInfo.citySentiment = totalSentiment / totalHouses;
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
				city_message_post(0, Message_48_PeopleAngry, 0, 0);
			} else if (Data_CityInfo.citySentiment < 40) {
				city_message_post(0, Message_47_PeopleUnhappy, 0, 0);
			} else {
				city_message_post(0, Message_46_PeopleDisgruntled, 0, 0);
			}
		}
	}

	int worstSentiment = 0;
	Data_CityInfo.populationEmigrationCause = EmigrationCause_None;
	if (sentimentContributionFood < worstSentiment) {
		worstSentiment = sentimentContributionFood;
		Data_CityInfo.populationEmigrationCause = EmigrationCause_NoFood;
	}
	if (sentimentContributionEmployment < worstSentiment) {
		worstSentiment = sentimentContributionEmployment;
		Data_CityInfo.populationEmigrationCause = EmigrationCause_NoJobs;
	}
	if (sentimentContributionTaxes < worstSentiment) {
		worstSentiment = sentimentContributionTaxes;
		Data_CityInfo.populationEmigrationCause = EmigrationCause_HighTaxes;
	}
	if (sentimentContributionWages < worstSentiment) {
		worstSentiment = sentimentContributionWages;
		Data_CityInfo.populationEmigrationCause = EmigrationCause_LowWages;
	}
	if (sentimentContributionTents < worstSentiment) {
		worstSentiment = sentimentContributionTents;
		Data_CityInfo.populationEmigrationCause = EmigrationCause_ManyTents;
	}
	Data_CityInfo.citySentimentLastTime = Data_CityInfo.citySentiment;
}

void CityInfo_Population_calculateMigrationSentiment()
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

	int populationCap = tutorial_get_population_cap(200000);
	if (Data_CityInfo.population >= populationCap) {
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

void CityInfo_Population_updateHealthRate()
{
	if (Data_CityInfo.population < 200 || IsTutorial1() || IsTutorial2()) {
		Data_CityInfo.healthRate = 50;
		Data_CityInfo.healthRateTarget = 50;
		return;
	}
	int totalPopulation = 0;
	int healthyPopulation = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (!BuildingIsInUse(i) || !b->houseSize || !b->housePopulation) {
			continue;
		}
		totalPopulation += b->housePopulation;
		if (b->subtype.houseLevel <= HOUSE_LARGE_TENT) {
			if (b->data.house.clinic) {
				healthyPopulation += b->housePopulation;
			} else {
				healthyPopulation += b->housePopulation / 4;
			}
		} else if (b->data.house.clinic) {
			if (b->houseDaysWithoutFood <= 0) {
				healthyPopulation += b->housePopulation;
			} else {
				healthyPopulation += b->housePopulation / 4;
			}
		} else if (b->houseDaysWithoutFood <= 0) {
			healthyPopulation += b->housePopulation / 4;
		}
	}
	Data_CityInfo.healthRateTarget = calc_percentage(healthyPopulation, totalPopulation);
	if (Data_CityInfo.healthRate < Data_CityInfo.healthRateTarget) {
		Data_CityInfo.healthRate += 2;
		if (Data_CityInfo.healthRate > Data_CityInfo.healthRateTarget) {
			Data_CityInfo.healthRate = Data_CityInfo.healthRateTarget;
		}
	} else if (Data_CityInfo.healthRate > Data_CityInfo.healthRateTarget) {
		Data_CityInfo.healthRate -= 2;
		if (Data_CityInfo.healthRate < Data_CityInfo.healthRateTarget) {
			Data_CityInfo.healthRate = Data_CityInfo.healthRateTarget;
		}
	}
	Data_CityInfo.healthRate = calc_bound(Data_CityInfo.healthRate, 0, 100);

	healthCauseDisease(totalPopulation);
}

static void healthCauseDisease(int totalPeople)
{
	if (Data_CityInfo.healthRate >= 40) {
		return;
	}
	int chanceValue = random_byte() & 0x3f;
	if (Data_CityInfo.godCurseVenusActive) {
		// force plague
		chanceValue = 0;
		Data_CityInfo.godCurseVenusActive = 0;
	}
	if (chanceValue > 40 - Data_CityInfo.healthRate) {
		return;
	}

	int sickPeople = calc_adjust_with_percentage(totalPeople, 7 + (random_byte() & 3));
	if (sickPeople <= 0) {
		return;
	}
	CityInfo_Population_changeHealthRate(10);
	int peopleToKill = sickPeople - Data_CityInfo.numHospitalWorkers;
	if (peopleToKill <= 0) {
		city_message_post(1, Message_102_HealthIllness, 0, 0);
		return;
	}
	if (Data_CityInfo.numHospitalWorkers > 0) {
		city_message_post(1, Message_103_HealthDisease, 0, 0);
	} else {
		city_message_post(1, Message_104_HealthPestilence, 0, 0);
	}
	tutorial_on_disease();
	// kill people who don't have access to a doctor
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (BuildingIsInUse(i) && b->houseSize && b->housePopulation) {
			if (!b->data.house.clinic) {
				peopleToKill -= b->housePopulation;
				Building_collapseOnFire(i, 1);
				if (peopleToKill <= 0) {
					return;
				}
			}
		}
	}
	// kill people in tents
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (BuildingIsInUse(i) && b->houseSize && b->housePopulation) {
			if (b->subtype.houseLevel <= HOUSE_LARGE_TENT) {
				peopleToKill -= b->housePopulation;
				Building_collapseOnFire(i, 1);
				if (peopleToKill <= 0) {
					return;
				}
			}
		}
	}
	// kill anyone
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (BuildingIsInUse(i) && b->houseSize && b->housePopulation) {
			peopleToKill -= b->housePopulation;
			Building_collapseOnFire(i, 1);
			if (peopleToKill <= 0) {
				return;
			}
		}
	}
}

void CityInfo_Population_changeHealthRate(int amount)
{
	Data_CityInfo.healthRate = calc_bound(Data_CityInfo.healthRate + amount, 0, 100);
}

static void recalculatePopulation()
{
	Data_CityInfo.population = 0;
	for (int i = 0; i < 100; i++) {
		Data_CityInfo.population += Data_CityInfo.populationPerAge[i];
	}
	if (Data_CityInfo.population > Data_CityInfo.populationHighestEver) {
		Data_CityInfo.populationHighestEver = Data_CityInfo.population;
	}
}

static int getPeopleInAgeDecennium(int decennium)
{
	int pop = 0;
	for (int i = 0; i < 10; i++) {
		pop += Data_CityInfo.populationPerAge[10 * decennium + i];
	}
	return pop;
}

static void removePeopleFromCensusInDecennium(int decennium, int numPeople)
{
	int emptyBuckets = 0;
	int age = 0;
	while (numPeople > 0 && emptyBuckets < 10) {
		if (Data_CityInfo.populationPerAge[10 * decennium + age] <= 0) {
			emptyBuckets++;
		} else {
			Data_CityInfo.populationPerAge[10 * decennium + age]--;
			numPeople--;
			emptyBuckets = 0;
		}
		age++;
		if (age >= 10) {
			age = 0;
		}
	}
}

static void removePeopleFromCensus(int numPeople)
{
	int index = 0;
	int emptyBuckets = 0;
	// remove people randomly up to age 63
	while (numPeople > 0 && emptyBuckets < 100) {
		int age = random_from_pool(index++) & 0x3f;
		if (Data_CityInfo.populationPerAge[age] <= 0) {
			emptyBuckets++;
		} else {
			Data_CityInfo.populationPerAge[age]--;
			numPeople--;
			emptyBuckets = 0;
		}
	}
	// if random didn't work: remove from age 10 and up
	emptyBuckets = 0;
	int age = 10;
	while (numPeople > 0 && emptyBuckets < 100) {
		if (Data_CityInfo.populationPerAge[age] <= 0) {
			emptyBuckets++;
		} else {
			Data_CityInfo.populationPerAge[age]--;
			numPeople--;
			emptyBuckets = 0;
		}
		age++;
		if (age >= 100) {
			age = 0;
		}
	}
}

static void addPeopleToCensus(int numPeople)
{
	int odd = 0;
	int index = 0;
	for (int i = 0; i < numPeople; i++, odd = 1 - odd) {
		int age = random_from_pool(index++) & 0x3f; // 63
		if (age > 50) {
			age -= 30;
		} else if (age < 10 && odd) {
			age += 20;
		}
		Data_CityInfo.populationPerAge[age]++;
	}
}

void CityInfo_Population_addPeople(int numPeople)
{
	Data_CityInfo.populationLastChange = numPeople;
	if (numPeople > 0) {
		addPeopleToCensus(numPeople);
	} else if (numPeople < 0) {
		removePeopleFromCensus(-numPeople);
	}
	recalculatePopulation();
}

void CityInfo_Population_removePeopleHomeRemoved(int numPeople)
{
	Data_CityInfo.populationLostInRemoval += numPeople;
	removePeopleFromCensus(numPeople);
	recalculatePopulation();
}

void CityInfo_Population_addPeopleHomeless(int numPeople)
{
	Data_CityInfo.populationLostHomeless -= numPeople;
	addPeopleToCensus(numPeople);
	recalculatePopulation();
}

void CityInfo_Population_removePeopleHomeless(int numPeople)
{
	Data_CityInfo.populationLostHomeless += numPeople;
	removePeopleFromCensus(numPeople);
	recalculatePopulation();
}

void CityInfo_Population_removePeopleForTroopRequest(int amount)
{
	int removed = HousePopulation_removePeople(amount);
	removePeopleFromCensus(removed);
	Data_CityInfo.populationLostTroopRequest += amount;
	recalculatePopulation();
}

int CityInfo_Population_getPeopleOfWorkingAge()
{
	return
		getPeopleInAgeDecennium(2) +
		getPeopleInAgeDecennium(3) +
		getPeopleInAgeDecennium(4);
}

int CityInfo_Population_getNumberOfSchoolAgeChildren()
{
	int pop = 0;
	for (int i = 0; i < 14; i++) {
		pop += Data_CityInfo.populationPerAge[i];
	}
	return pop;
}

int CityInfo_Population_getNumberOfAcademyChildren()
{
	int pop = 0;
	for (int i = 14; i < 21; i++) {
		pop += Data_CityInfo.populationPerAge[i];
	}
	return pop;
}

static void yearlyAdvanceAgesAndCalculateDeaths()
{
	int aged100 = Data_CityInfo.populationPerAge[99];
	for (int age = 99; age > 0; age--) {
		Data_CityInfo.populationPerAge[age] = Data_CityInfo.populationPerAge[age-1];
	}
	Data_CityInfo.populationPerAge[0] = 0;
	Data_CityInfo.populationYearlyDeaths = 0;
	for (int decennium = 9; decennium >= 0; decennium--) {
		int people = getPeopleInAgeDecennium(decennium);
		int deaths = calc_adjust_with_percentage(people,
			yearlyDeathsPerHealthPerDecennium[Data_CityInfo.healthRate / 10][decennium]);
		int removed = HousePopulation_removePeople(deaths + aged100);
		removePeopleFromCensusInDecennium(decennium, removed);
		// ^ BUGFIX should be deaths only, now aged100 are removed from census while they weren't *in* the census
		Data_CityInfo.populationYearlyDeaths += removed;
		aged100 = 0;
	}
}

static void yearlyCalculateBirths()
{
	Data_CityInfo.populationYearlyBirths = 0;
	for (int decennium = 9; decennium >= 0; decennium--) {
		int people = getPeopleInAgeDecennium(decennium);
		int births = calc_adjust_with_percentage(people, yearlyBirthsPerDecennium[decennium]);
		int added = HousePopulation_addPeople(births);
		Data_CityInfo.populationPerAge[0] += added;
		Data_CityInfo.populationYearlyBirths += added;
	}
}

static void yearlyUpdateAfterDeathsBirths()
{
	Data_CityInfo.populationYearlyUpdatedNeeded = 0;
	Data_CityInfo.populationLastYear = Data_CityInfo.population;
	recalculatePopulation();

	Data_CityInfo.populationLostInRemoval = 0;
	Data_CityInfo.populationTotalAllYears += Data_CityInfo.population;
	Data_CityInfo.populationTotalYears++;
	Data_CityInfo.populationAveragePerYear = Data_CityInfo.populationTotalAllYears / Data_CityInfo.populationTotalYears;
}

void CityInfo_Population_requestYearlyUpdate()
{
	Data_CityInfo.populationYearlyUpdatedNeeded = 1;
	HousePopulation_calculatePeoplePerType();
}

void CityInfo_Population_yearlyUpdate()
{
	if (Data_CityInfo.populationYearlyUpdatedNeeded) {
		yearlyAdvanceAgesAndCalculateDeaths();
		yearlyCalculateBirths();
		yearlyUpdateAfterDeathsBirths();
	}
}
