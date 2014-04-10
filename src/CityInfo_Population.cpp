#include "CityInfo.h"

#include "HousePopulation.h"
#include "Util.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Random.h"

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
		if (Data_Buildings[i].inUse && Data_Buildings[i].houseSize) {
			Data_Buildings[i].sentiment.houseHappiness += amount;
			BOUND(Data_Buildings[i].sentiment.houseHappiness, 0, 100);
		}
	}
}

void CityInfo_Population_setMaxHappiness(int max)
{
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (Data_Buildings[i].inUse && Data_Buildings[i].houseSize) {
			if (Data_Buildings[i].sentiment.houseHappiness > max) {
				Data_Buildings[i].sentiment.houseHappiness = max;
			}
			BOUND(Data_Buildings[i].sentiment.houseHappiness, 0, 100);
		}
	}
}

void CityInfo_Population_changeHealthRate(int amount)
{
	// TODO
}

void CityInfo_Population_calculateSentiment()
{
	// TODO
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
	int index = Data_Random.poolIndex;
	int emptyBuckets = 0;
	// remove people randomly up to age 63
	while (numPeople > 0 && emptyBuckets < 100) {
		int age = Data_Random.pool[index] & 0x3f;
		if (Data_CityInfo.populationPerAge[age] <= 0) {
			emptyBuckets++;
		} else {
			Data_CityInfo.populationPerAge[age]--;
			numPeople--;
			emptyBuckets = 0;
		}
		if (++index >= 100) {
			index = 0;
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
		if (++age >= 100) {
			index = 0;
		}
	}
}

static void addPeopleToCensus(int numPeople)
{
	int odd = 0;
	int index = Data_Random.poolIndex;
	for (int i = 0; i < numPeople; i++, index++, odd = 1 - odd) {
		if (index >= 100) {
			index = 0;
		}
		int age = Data_Random.pool[index] & 0x3f; // 63
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
		removePeopleFromCensus(numPeople);
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
